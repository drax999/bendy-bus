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

#include "dfsm-ast-expression.h"
#include "dfsm-ast-object.h"

G_DEFINE_ABSTRACT_TYPE (DfsmAstExpression, dfsm_ast_expression, DFSM_TYPE_AST_NODE)

static void
dfsm_ast_expression_class_init (DfsmAstExpressionClass *klass)
{
	/* Nothing to see here. */
}

static void
dfsm_ast_expression_init (DfsmAstExpression *self)
{
	/* Nothing to see here. */
}

/**
 * dfsm_ast_expression_calculate_type:
 * @self: a #DfsmAstExpression
 * @environment: a #DfsmEnvironment containing all defined variables
 *
 * Calculate the type of the given @expression. In some cases this may not be a definite type, for example if the expression is an empty data
 * structure. In most cases, however, the type will be definite.
 *
 * This assumes that the expression has already been checked, and so this does not perform any type checking of its own.
 *
 * Return value: (transfer full): the type of the expression
 */
GVariantType *
dfsm_ast_expression_calculate_type (DfsmAstExpression *self, DfsmEnvironment *environment)
{
	DfsmAstExpressionClass *klass;

	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION (self), NULL);
	g_return_val_if_fail (DFSM_IS_ENVIRONMENT (environment), NULL);

	klass = DFSM_AST_EXPRESSION_GET_CLASS (self);

	g_assert (klass->calculate_type != NULL);
	return klass->calculate_type (self, environment);
}

/**
 * dfsm_ast_expression_evaluate:
 * @self: a #DfsmAstExpression
 * @environment: a #DfsmEnvironment containing all defined variables
 * @error: (allow-none): a #GError, or %NULL
 *
 * Evaluate the given @expression in the given @environment. This will not modify the environment.
 *
 * This assumes that the expression has already been checked, and so this does not perform any type checking of its own.
 *
 * Return value: (transfer full): non-floating value of the expression
 */
GVariant *
dfsm_ast_expression_evaluate (DfsmAstExpression *self, DfsmEnvironment *environment, GError **error)
{
	DfsmAstExpressionClass *klass;
	GVariant *return_value;
	GError *child_error = NULL;

	g_return_val_if_fail (DFSM_IS_AST_EXPRESSION (self), NULL);
	g_return_val_if_fail (DFSM_IS_ENVIRONMENT (environment), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	klass = DFSM_AST_EXPRESSION_GET_CLASS (self);

	g_assert (klass->evaluate != NULL);
	return_value = klass->evaluate (self, environment, &child_error);

	g_assert ((return_value == NULL) != (child_error == NULL));
	g_assert (return_value == NULL || g_variant_is_floating (return_value) == FALSE);

	/* We use our own child_error so that we can guarantee to virtual method implementations that error will be non-NULL. */
	if (child_error != NULL) {
		g_propagate_error (error, child_error);
	}

	return return_value;
}
