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

#ifndef DFSM_AST_STATEMENT_ASSIGNMENT_H
#define DFSM_AST_STATEMENT_ASSIGNMENT_H

#include <glib.h>
#include <glib-object.h>

#include <dfsm/dfsm-ast-data-structure.h>
#include <dfsm/dfsm-ast-expression.h>
#include <dfsm/dfsm-ast-statement.h>

G_BEGIN_DECLS

#define DFSM_TYPE_AST_STATEMENT_ASSIGNMENT		(dfsm_ast_statement_assignment_get_type ())
#define DFSM_AST_STATEMENT_ASSIGNMENT(o) \
	(G_TYPE_CHECK_INSTANCE_CAST ((o), DFSM_TYPE_AST_STATEMENT_ASSIGNMENT, DfsmAstStatementAssignment))
#define DFSM_AST_STATEMENT_ASSIGNMENT_CLASS(k) \
	(G_TYPE_CHECK_CLASS_CAST((k), DFSM_TYPE_AST_STATEMENT_ASSIGNMENT, DfsmAstStatementAssignmentClass))
#define DFSM_IS_AST_STATEMENT_ASSIGNMENT(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), DFSM_TYPE_AST_STATEMENT_ASSIGNMENT))
#define DFSM_IS_AST_STATEMENT_ASSIGNMENT_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DFSM_TYPE_AST_STATEMENT_ASSIGNMENT))
#define DFSM_AST_STATEMENT_ASSIGNMENT_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS ((o), DFSM_TYPE_AST_STATEMENT_ASSIGNMENT, DfsmAstStatementAssignmentClass))

typedef struct _DfsmAstStatementAssignmentPrivate	DfsmAstStatementAssignmentPrivate;

/**
 * DfsmAstStatementAssignment:
 *
 * All the fields in the #DfsmAstStatementAssignment structure are private and should never be accessed directly.
 */
typedef struct {
	DfsmAstStatement parent;
	DfsmAstStatementAssignmentPrivate *priv;
} DfsmAstStatementAssignment;

/**
 * DfsmAstStatementAssignmentClass:
 *
 * All the fields in the #DfsmAstStatementAssignmentClass structure are private and should never be accessed directly.
 */
typedef struct {
	/*< private >*/
	DfsmAstStatementClass parent;
} DfsmAstStatementAssignmentClass;

GType dfsm_ast_statement_assignment_get_type (void) G_GNUC_CONST;

DfsmAstDataStructure *dfsm_ast_statement_assignment_get_variable (DfsmAstStatementAssignment *self) G_GNUC_PURE;
DfsmAstExpression *dfsm_ast_statement_assignment_get_expression (DfsmAstStatementAssignment *self) G_GNUC_PURE;

G_END_DECLS

#endif /* !DFSM_AST_STATEMENT_ASSIGNMENT_H */
