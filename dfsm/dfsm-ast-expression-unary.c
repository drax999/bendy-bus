/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * D-Bus Simulator
 * Copyright (C) Philip Withnall 2011 <philip@tecnocode.co.uk>
 * 
 * D-Bus Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * D-Bus Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with D-Bus Simulator.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:dfsm-ast-expression-unary
 * @short_description: AST unary expression node
 * @stability: Unstable
 * @include: dfsm/dfsm-ast-expression-unary.h
 *
 * AST unary expression node implementing support for unary operations such as logical negation.
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "dfsm-ast-expression-unary.h"
#include "dfsm-parser.h"
#include "dfsm-parser-internal.h"

static void dfsm_ast_expression_unary_dispose (GObject *object);
static void dfsm_ast_expression_unary_sanity_check (DfsmAstNode *node);
static void dfsm_ast_expression_unary_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static void dfsm_ast_expression_unary_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static GVariantType *dfsm_ast_expression_unary_calculate_type (DfsmAstExpression *self, DfsmEnvironment *environment);
static GVariant *dfsm_ast_expression_unary_evaluate (DfsmAstExpression *self, DfsmEnvironment *environment);
static gdouble dfsm_ast_expression_unary_calculate_weight (DfsmAstExpression *self);

struct _DfsmAstExpressionUnaryPrivate {
	DfsmAstExpressionUnaryType expression_type;
	DfsmAstExpression *child_node;
};

G_DEFINE_TYPE (DfsmAstExpressionUnary, dfsm_ast_expression_unary, DFSM_TYPE_AST_EXPRESSION)

static void
dfsm_ast_expression_unary_class_init (DfsmAstExpressionUnaryClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	DfsmAstNodeClass *node_class = DFSM_AST_NODE_CLASS (klass);
	DfsmAstExpressionClass *expression_class = DFSM_AST_EXPRESSION_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DfsmAstExpressionUnaryPrivate));

	gobject_class->dispose = dfsm_ast_expression_unary_dispose;

	node_class->sanity_check = dfsm_ast_expression_unary_sanity_check;
	node_class->pre_check_and_register = dfsm_ast_expression_unary_pre_check_and_register;
	node_class->check = dfsm_ast_expression_unary_check;

	expression_class->calculate_type = dfsm_ast_expression_unary_calculate_type;
	expression_class->evaluate = dfsm_ast_expression_unary_evaluate;
	expression_class->calculate_weight = dfsm_ast_expression_unary_calculate_weight;
}

static void
dfsm_ast_expression_unary_init (DfsmAstExpressionUnary *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, DFSM_TYPE_AST_EXPRESSION_UNARY, DfsmAstExpressionUnaryPrivate);
}

static void
dfsm_ast_expression_unary_dispose (GObject *object)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (object)->priv;

	g_clear_object (&priv->child_node);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (dfsm_ast_expression_unary_parent_class)->dispose (object);
}

static void
dfsm_ast_expression_unary_sanity_check (DfsmAstNode *node)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (node)->priv;

	switch (priv->expression_type) {
		case DFSM_AST_EXPRESSION_UNARY_NOT:
			/* Valid */
			break;
		default:
			g_assert_not_reached ();
	}

	g_assert (priv->child_node != NULL);
	dfsm_ast_node_sanity_check (DFSM_AST_NODE (priv->child_node));
}

static void
dfsm_ast_expression_unary_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (node)->priv;

	dfsm_ast_node_pre_check_and_register (DFSM_AST_NODE (priv->child_node), environment, error);

	if (*error != NULL) {
		return;
	}
}

static void
dfsm_ast_expression_unary_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (node)->priv;
	GVariantType *child_type;
	const GVariantType *desired_supertype;

	dfsm_ast_node_check (DFSM_AST_NODE (priv->child_node), environment, error);

	if (*error != NULL) {
		return;
	}

	child_type = dfsm_ast_expression_calculate_type (priv->child_node, environment);

	switch (priv->expression_type) {
		case DFSM_AST_EXPRESSION_UNARY_NOT:
			desired_supertype = G_VARIANT_TYPE_BOOLEAN;
			break;
		default:
			g_assert_not_reached ();
	}

	if (g_variant_type_is_subtype_of (child_type, desired_supertype) == FALSE) {
		gchar *formal, *actual;

		formal = g_variant_type_dup_string (desired_supertype);
		actual = g_variant_type_dup_string (child_type);

		g_variant_type_free (child_type);

		g_set_error (error, DFSM_PARSE_ERROR, DFSM_PARSE_ERROR_AST_INVALID,
		             _("Type mismatch between the formal and actual parameter to unary operator %u: expects type %s but received type %s."),
		             priv->expression_type, formal, actual);
		return;
	}

	g_variant_type_free (child_type);
}

static GVariantType *
dfsm_ast_expression_unary_calculate_type (DfsmAstExpression *expression, DfsmEnvironment *environment)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (expression)->priv;

	switch (priv->expression_type) {
		case DFSM_AST_EXPRESSION_UNARY_NOT:
			return g_variant_type_copy (G_VARIANT_TYPE_BOOLEAN);
		default:
			g_assert_not_reached ();
	}
}

static GVariant *
dfsm_ast_expression_unary_evaluate (DfsmAstExpression *expression, DfsmEnvironment *environment)
{
	DfsmAstExpressionUnaryPrivate *priv = DFSM_AST_EXPRESSION_UNARY (expression)->priv;
	GVariant *child_value, *unary_value;

	/* Evaluate our sub-expression first. */
	child_value = dfsm_ast_expression_evaluate (priv->child_node, environment);

	/* Do the actual evaluation. */
	switch (priv->expression_type) {
		case DFSM_AST_EXPRESSION_UNARY_NOT:
			unary_value = g_variant_new_boolean (!g_variant_get_boolean (child_value));
			break;
		default:
			g_assert_not_reached ();
	}

	/* Tidy up and return */
	g_variant_unref (child_value);

	g_assert (g_variant_is_floating (unary_value) == TRUE);
	g_variant_ref_sink (unary_value); /* sink reference */

	return unary_value;
}

static gdouble
dfsm_ast_expression_unary_calculate_weight (DfsmAstExpression *self)
{
	return dfsm_ast_expression_calculate_weight (DFSM_AST_EXPRESSION_UNARY (self)->priv->child_node);
}

/**
 * dfsm_ast_expression_unary_new:
 * @expression_type: the type of expression
 * @child_node: the expression's child node, or %NULL
 *
 * Create a new #DfsmAstExpression of type @expression_type with the given child node.
 *
 * Return value: (transfer full): a new AST node
 */
DfsmAstExpression *
dfsm_ast_expression_unary_new (DfsmAstExpressionUnaryType expression_type, DfsmAstExpression *child_node)
{
	DfsmAstExpressionUnary *expression;
	DfsmAstExpressionUnaryPrivate *priv;

	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION (child_node), NULL);

	switch (expression_type) {
		case DFSM_AST_EXPRESSION_UNARY_NOT:
			/* Valid */
			break;
		default:
			g_assert_not_reached ();
	}

	expression = g_object_new (DFSM_TYPE_AST_EXPRESSION_UNARY, NULL);
	priv = expression->priv;

	priv->expression_type = expression_type;
	priv->child_node = g_object_ref (child_node);

	return DFSM_AST_EXPRESSION (expression);
}
