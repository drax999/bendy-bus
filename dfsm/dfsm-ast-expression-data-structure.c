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

#include <glib.h>

#include "dfsm-ast-expression-data-structure.h"
#include "dfsm-parser-internal.h"

static void dfsm_ast_expression_data_structure_dispose (GObject *object);
static void dfsm_ast_expression_data_structure_sanity_check (DfsmAstNode *node);
static void dfsm_ast_expression_data_structure_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static void dfsm_ast_expression_data_structure_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static GVariantType *dfsm_ast_expression_data_structure_calculate_type (DfsmAstExpression *self, DfsmEnvironment *environment);
static GVariant *dfsm_ast_expression_data_structure_evaluate (DfsmAstExpression *self, DfsmEnvironment *environment, GError **error);
static gdouble dfsm_ast_expression_data_structure_calculate_weight (DfsmAstExpression *self);

struct _DfsmAstExpressionDataStructurePrivate {
	DfsmAstDataStructure *data_structure;
};

G_DEFINE_TYPE (DfsmAstExpressionDataStructure, dfsm_ast_expression_data_structure, DFSM_TYPE_AST_EXPRESSION)

static void
dfsm_ast_expression_data_structure_class_init (DfsmAstExpressionDataStructureClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	DfsmAstNodeClass *node_class = DFSM_AST_NODE_CLASS (klass);
	DfsmAstExpressionClass *expression_class = DFSM_AST_EXPRESSION_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DfsmAstExpressionDataStructurePrivate));

	gobject_class->dispose = dfsm_ast_expression_data_structure_dispose;

	node_class->sanity_check = dfsm_ast_expression_data_structure_sanity_check;
	node_class->pre_check_and_register = dfsm_ast_expression_data_structure_pre_check_and_register;
	node_class->check = dfsm_ast_expression_data_structure_check;

	expression_class->calculate_type = dfsm_ast_expression_data_structure_calculate_type;
	expression_class->evaluate = dfsm_ast_expression_data_structure_evaluate;
	expression_class->calculate_weight = dfsm_ast_expression_data_structure_calculate_weight;
}

static void
dfsm_ast_expression_data_structure_init (DfsmAstExpressionDataStructure *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, DFSM_TYPE_AST_EXPRESSION_DATA_STRUCTURE, DfsmAstExpressionDataStructurePrivate);
}

static void
dfsm_ast_expression_data_structure_dispose (GObject *object)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (object)->priv;

	g_clear_object (&priv->data_structure);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (dfsm_ast_expression_data_structure_parent_class)->dispose (object);
}

static void
dfsm_ast_expression_data_structure_sanity_check (DfsmAstNode *node)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (node)->priv;

	g_assert (priv->data_structure != NULL);
	dfsm_ast_node_sanity_check (DFSM_AST_NODE (priv->data_structure));
}

static void
dfsm_ast_expression_data_structure_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (node)->priv;

	dfsm_ast_node_pre_check_and_register (DFSM_AST_NODE (priv->data_structure), environment, error);

	if (*error != NULL) {
		return;
	}
}

static void
dfsm_ast_expression_data_structure_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (node)->priv;

	dfsm_ast_node_check (DFSM_AST_NODE (priv->data_structure), environment, error);

	if (*error != NULL) {
		return;
	}
}

static GVariantType *
dfsm_ast_expression_data_structure_calculate_type (DfsmAstExpression *expression, DfsmEnvironment *environment)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (expression)->priv;

	/* Type of the expression is the type of the data structure. */
	return dfsm_ast_data_structure_calculate_type (priv->data_structure, environment);
}

static GVariant *
dfsm_ast_expression_data_structure_evaluate (DfsmAstExpression *expression, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionDataStructurePrivate *priv = DFSM_AST_EXPRESSION_DATA_STRUCTURE (expression)->priv;

	return dfsm_ast_data_structure_to_variant (priv->data_structure, environment, error);
}

static gdouble
dfsm_ast_expression_data_structure_calculate_weight (DfsmAstExpression *self)
{
	return dfsm_ast_data_structure_get_weight (DFSM_AST_EXPRESSION_DATA_STRUCTURE (self)->priv->data_structure);
}

/**
 * dfsm_ast_expression_data_structure_new:
 * @data_structure: a #DfsmAstDataStructure to wrap
 * @error: (allow-none): a #GError, or %NULL
 *
 * Create a new #DfsmAstExpression wrapping the given @data_structure.
 *
 * Return value: (transfer full): a new expression
 */
DfsmAstExpression *
dfsm_ast_expression_data_structure_new (DfsmAstDataStructure *data_structure, GError **error)
{
	DfsmAstExpressionDataStructure *expression;
	DfsmAstExpressionDataStructurePrivate *priv;

	g_return_val_if_fail (DFSM_IS_AST_DATA_STRUCTURE (data_structure), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	expression = g_object_new (DFSM_TYPE_AST_EXPRESSION_DATA_STRUCTURE, NULL);
	priv = expression->priv;

	priv->data_structure = g_object_ref (data_structure);

	return DFSM_AST_EXPRESSION (expression);
}

/**
 * dfsm_ast_expression_data_structure_to_variant:
 * @self: a #DfsmAstExpressionDataStructure
 * @environment: TODO
 * @error: (allow-none): a #GError, or %NULL
 *
 * TODO
 *
 * Return value: (transfer full): TODO
 */
GVariant *
dfsm_ast_expression_data_structure_to_variant (DfsmAstExpressionDataStructure *self, DfsmEnvironment *environment, GError **error)
{
	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION_DATA_STRUCTURE (self), NULL);
	g_return_val_if_fail (DFSM_IS_ENVIRONMENT (environment), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	return dfsm_ast_data_structure_to_variant (self->priv->data_structure, environment, error);
}

/**
 * dfsm_ast_expression_data_structure_set_from_variant:
 * @self: a #DfsmAstExpressionDataStructure
 * @environment: TODO
 * @new_value: TODO
 * @error: (allow-none): a #GError, or %NULL
 *
 * TODO
 */
void
dfsm_ast_expression_data_structure_set_from_variant (DfsmAstExpressionDataStructure *self, DfsmEnvironment *environment, GVariant *new_value,
                                                     GError **error)
{

}

/**
 * dfsm_ast_expression_data_structure_get_data_structure:
 * @self: a #DfsmAstExpressionDataStructure
 *
 * Gets the #DfsmAstDataStructure which forms this expression.
 *
 * Return value: (transfer none): the data structure forming the expression
 */
DfsmAstDataStructure *
dfsm_ast_expression_data_structure_get_data_structure (DfsmAstExpressionDataStructure *self)
{
	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION_DATA_STRUCTURE (self), NULL);

	return self->priv->data_structure;
}
