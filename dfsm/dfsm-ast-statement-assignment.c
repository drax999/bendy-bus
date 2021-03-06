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
 * SECTION:dfsm-ast-statement-assignment
 * @short_description: AST assignment statement node
 * @stability: Unstable
 * @include: dfsm/dfsm-ast-statement-assignment.h
 *
 * AST assignment statement implementing support for assigning new values to variables and structures of variables.
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "dfsm-ast-statement-assignment.h"
#include "dfsm-parser.h"
#include "dfsm-parser-internal.h"

static void dfsm_ast_statement_assignment_dispose (GObject *object);
static void dfsm_ast_statement_assignment_sanity_check (DfsmAstNode *node);
static void dfsm_ast_statement_assignment_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static void dfsm_ast_statement_assignment_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error);
static void dfsm_ast_statement_assignment_execute (DfsmAstStatement *statement, DfsmEnvironment *environment, DfsmOutputSequence *output_sequence);

struct _DfsmAstStatementAssignmentPrivate {
	DfsmAstDataStructure *data_structure; /* lvalue */
	DfsmAstExpression *expression; /* rvalue */
};

G_DEFINE_TYPE (DfsmAstStatementAssignment, dfsm_ast_statement_assignment, DFSM_TYPE_AST_STATEMENT)

static void
dfsm_ast_statement_assignment_class_init (DfsmAstStatementAssignmentClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	DfsmAstNodeClass *node_class = DFSM_AST_NODE_CLASS (klass);
	DfsmAstStatementClass *statement_class = DFSM_AST_STATEMENT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DfsmAstStatementAssignmentPrivate));

	gobject_class->dispose = dfsm_ast_statement_assignment_dispose;

	node_class->sanity_check = dfsm_ast_statement_assignment_sanity_check;
	node_class->pre_check_and_register = dfsm_ast_statement_assignment_pre_check_and_register;
	node_class->check = dfsm_ast_statement_assignment_check;

	statement_class->execute = dfsm_ast_statement_assignment_execute;
}

static void
dfsm_ast_statement_assignment_init (DfsmAstStatementAssignment *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, DFSM_TYPE_AST_STATEMENT_ASSIGNMENT, DfsmAstStatementAssignmentPrivate);
}

static void
dfsm_ast_statement_assignment_dispose (GObject *object)
{
	DfsmAstStatementAssignmentPrivate *priv = DFSM_AST_STATEMENT_ASSIGNMENT (object)->priv;

	g_clear_object (&priv->expression);
	g_clear_object (&priv->data_structure);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (dfsm_ast_statement_assignment_parent_class)->dispose (object);
}

static void
dfsm_ast_statement_assignment_sanity_check (DfsmAstNode *node)
{
	DfsmAstStatementAssignmentPrivate *priv = DFSM_AST_STATEMENT_ASSIGNMENT (node)->priv;

	g_assert (priv->data_structure != NULL);
	dfsm_ast_node_sanity_check (DFSM_AST_NODE (priv->data_structure));

	g_assert (priv->expression != NULL);
	dfsm_ast_node_sanity_check (DFSM_AST_NODE (priv->expression));
}

static void
dfsm_ast_statement_assignment_pre_check_and_register (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstStatementAssignmentPrivate *priv = DFSM_AST_STATEMENT_ASSIGNMENT (node)->priv;

	dfsm_ast_node_pre_check_and_register (DFSM_AST_NODE (priv->data_structure), environment, error);

	if (*error != NULL) {
		return;
	}

	dfsm_ast_node_pre_check_and_register (DFSM_AST_NODE (priv->expression), environment, error);

	if (*error != NULL) {
		return;
	}
}

static void
dfsm_ast_statement_assignment_check (DfsmAstNode *node, DfsmEnvironment *environment, GError **error)
{
	DfsmAstStatementAssignmentPrivate *priv = DFSM_AST_STATEMENT_ASSIGNMENT (node)->priv;
	GVariantType *lvalue_type, *rvalue_type;

	/* Check the sub-nodes first .*/
	dfsm_ast_node_check (DFSM_AST_NODE (priv->data_structure), environment, error);

	if (*error != NULL) {
		return;
	}

	dfsm_ast_node_check (DFSM_AST_NODE (priv->expression), environment, error);

	if (*error != NULL) {
		return;
	}

	/* Check that the l-value can actually be assigned to: it has to be a data structure with leaf nodes which are only variables (apart from
	 * dictionary keys, which can have any type). */
	if (dfsm_ast_data_structure_is_variable (priv->data_structure) == FALSE) {
		g_set_error (error, DFSM_PARSE_ERROR, DFSM_PARSE_ERROR_AST_INVALID,
		             _("Non-variable data structure as the l-value of an assignment: "
		               "only data structures consisting of only variables may be assigned to."));

		return;
	}

	/* Check the types of the l- and r-values are compatible. */
	lvalue_type = dfsm_ast_data_structure_calculate_type (priv->data_structure, environment);
	rvalue_type = dfsm_ast_expression_calculate_type (priv->expression, environment);

	if (g_variant_type_is_subtype_of (rvalue_type, lvalue_type) == FALSE) {
		gchar *expected, *received;

		expected = g_variant_type_dup_string (lvalue_type);
		received = g_variant_type_dup_string (rvalue_type);

		g_variant_type_free (lvalue_type);
		g_variant_type_free (rvalue_type);

		g_set_error (error, DFSM_PARSE_ERROR, DFSM_PARSE_ERROR_AST_INVALID,
		             _("Type mismatch for assignment: expected l-value type %s but received r-value type %s."), expected, received);

		return;
	}

	g_variant_type_free (lvalue_type);
	g_variant_type_free (rvalue_type);
}

static void
dfsm_ast_statement_assignment_execute (DfsmAstStatement *statement, DfsmEnvironment *environment, DfsmOutputSequence *output_sequence)
{
	DfsmAstStatementAssignmentPrivate *priv = DFSM_AST_STATEMENT_ASSIGNMENT (statement)->priv;
	GVariant *rvalue;

	/* Evaluate the rvalue */
	rvalue = dfsm_ast_expression_evaluate (priv->expression, environment);
	g_assert (rvalue != NULL);

	/* Perform the assignment */
	dfsm_ast_data_structure_set_from_variant (priv->data_structure, environment, rvalue);

	g_variant_unref (rvalue);
}

/**
 * dfsm_ast_statement_assignment_new:
 * @data_structure: data structure being assigned to (lvalue)
 * @expression: expression to be evaluated for assignment (rvalue)
 *
 * Create a new #DfsmAstStatement for an assignment of @expression to @data_structure.
 *
 * Return value: (transfer full): a new AST node
 */
DfsmAstStatement *
dfsm_ast_statement_assignment_new (DfsmAstDataStructure *data_structure, DfsmAstExpression *expression)
{
	DfsmAstStatementAssignment *statement;
	DfsmAstStatementAssignmentPrivate *priv;

	g_return_val_if_fail (DFSM_IS_AST_DATA_STRUCTURE (data_structure), NULL);
	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION (expression), NULL);

	statement = g_object_new (DFSM_TYPE_AST_STATEMENT_ASSIGNMENT, NULL);
	priv = statement->priv;

	priv->data_structure = g_object_ref (data_structure);
	priv->expression = g_object_ref (expression);

	return DFSM_AST_STATEMENT (statement);
}

/**
 * dfsm_ast_statement_assignment_get_variable:
 * @self: a #DfsmAstStatementAssignment
 *
 * Gets the variable being assigned to in the assignment (i.e. the l-value of the assignment).
 *
 * Return value: (transfer none): variable being assigned to
 */
DfsmAstDataStructure *
dfsm_ast_statement_assignment_get_variable (DfsmAstStatementAssignment *self)
{
	g_return_val_if_fail (DFSM_IS_AST_STATEMENT_ASSIGNMENT (self), NULL);

	return self->priv->data_structure;
}

/**
 * dfsm_ast_statement_assignment_get_expression:
 * @self: a #DfsmAstStatementAssignment
 *
 * Gets the expression being evaluated and assigned in the assignment (i.e. the r-value of the assignment).
 *
 * Return value: (transfer none): unevaluated form of the expression being evaluated and assigned
 */
DfsmAstExpression *
dfsm_ast_statement_assignment_get_expression (DfsmAstStatementAssignment *self)
{
	g_return_val_if_fail (DFSM_IS_AST_STATEMENT_ASSIGNMENT (self), NULL);

	return self->priv->expression;
}
