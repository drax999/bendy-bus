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

#include "dfsm-environment.h"
#include "dfsm-utils.h"

#ifndef DFSM_AST_H
#define DFSM_AST_H

G_BEGIN_DECLS

typedef enum {
	DFSM_AST_NODE_OBJECT,
	DFSM_AST_NODE_EXPRESSION,
	DFSM_AST_NODE_DATA_STRUCTURE,
	DFSM_AST_NODE_TRANSITION,
	DFSM_AST_NODE_PRECONDITION,
	DFSM_AST_NODE_STATEMENT,
	DFSM_AST_NODE_VARIABLE,
} DfsmAstNodeType;

typedef struct _DfsmAstNode DfsmAstNode;

typedef void (*DfsmAstNodeCheckFunc) (DfsmAstNode *node, GError **error);
typedef void (*DfsmAstNodeFreeFunc) (DfsmAstNode *node);

struct _DfsmAstNode {
	DfsmAstNodeType node_type;
	DfsmAstNodeCheckFunc check_func;
	DfsmAstNodeFreeFunc free_func;
	gint ref_count;
};

void dfsm_ast_node_check (DfsmAstNode *node, GError **error);
gpointer dfsm_ast_node_ref (gpointer/*<DfsmAstNode>*/ node);
void dfsm_ast_node_unref (gpointer/*<DfsmAstNode>*/ node);

typedef struct {
	DfsmAstNode parent;
	gchar *object_path;
	GPtrArray *interface_names; /* array of strings */
	DfsmEnvironment *environment;
	GPtrArray *states; /* array of strings (indexed by DfsmAstStateNumber) */
	GPtrArray *transitions; /* array of DfsmAstTransitions */
} DfsmAstObject;

DfsmAstObject *dfsm_ast_object_new (GDBusNodeInfo *dbus_node_info, const gchar *object_path, GPtrArray/*<string>*/ *interface_names,
                                    GPtrArray/*<GHashTable>*/ *data_blocks, GPtrArray/*<GPtrArray>*/ *state_blocks,
                                    GPtrArray/*<DfsmAstTransition>*/ *transition_blocks, GError **error) DFSM_CONSTRUCTOR;

typedef enum {
	/* Function calls */
	DFSM_AST_EXPRESSION_FUNCTION_CALL,

	/* Data structures */
	DFSM_AST_EXPRESSION_DATA_STRUCTURE,

	/* Unary expressions */
	DFSM_AST_EXPRESSION_NOT,

	/* Binary expressions */
	DFSM_AST_EXPRESSION_TIMES,
	DFSM_AST_EXPRESSION_DIVIDE,
	DFSM_AST_EXPRESSION_MODULUS,
	DFSM_AST_EXPRESSION_PLUS,
	DFSM_AST_EXPRESSION_MINUS,
	DFSM_AST_EXPRESSION_LT,
	DFSM_AST_EXPRESSION_LTE,
	DFSM_AST_EXPRESSION_GT,
	DFSM_AST_EXPRESSION_GTE,
	DFSM_AST_EXPRESSION_EQ,
	DFSM_AST_EXPRESSION_NEQ,
	DFSM_AST_EXPRESSION_AND,
	DFSM_AST_EXPRESSION_OR,
} DfsmAstExpressionType;

typedef struct _DfsmAstExpression DfsmAstExpression;

typedef GVariantType *(*DfsmAstExpressionCalculateTypeFunc) (DfsmAstExpression *expression);
typedef GVariant *(*DfsmAstExpressionEvaluateFunc) (DfsmAstExpression *expression, DfsmEnvironment *environment, GError **error);

/* abstract */
struct _DfsmAstExpression {
	DfsmAstNode parent;
	DfsmAstExpressionType expression_type;
	DfsmAstExpressionCalculateTypeFunc calculate_type_func;
	DfsmAstExpressionEvaluateFunc evaluate_func;
};

GVariantType *dfsm_ast_expression_calculate_type (DfsmAstExpression *expression) DFSM_CONSTRUCTOR;
GVariant *dfsm_ast_expression_evaluate (DfsmAstExpression *expression, DfsmEnvironment *environment, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstExpression parent;
	gchar *function_name;
	DfsmAstExpression *parameters;
} DfsmAstExpressionFunctionCall;

DfsmAstExpression *dfsm_ast_expression_function_call_new (const gchar *function_name, DfsmAstExpression *parameters, GError **error) DFSM_CONSTRUCTOR;

typedef struct _DfsmAstDataStructure DfsmAstDataStructure;

typedef struct {
	DfsmAstExpression parent;
	DfsmAstDataStructure *data_structure;
} DfsmAstExpressionDataStructure;

DfsmAstExpression *dfsm_ast_expression_data_structure_new (DfsmAstDataStructure *data_structure, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstExpression parent;
	DfsmAstExpression *child_node;
} DfsmAstExpressionUnary;

DfsmAstExpression *dfsm_ast_expression_unary_new (DfsmAstExpressionType expression_type, DfsmAstExpression *child_node, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstExpression parent;
	DfsmAstExpression *left_node;
	DfsmAstExpression *right_node;
} DfsmAstExpressionBinary;

DfsmAstExpression *dfsm_ast_expression_binary_new (DfsmAstExpressionType expression_type, DfsmAstExpression *left_node,
                                                   DfsmAstExpression *right_node, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstExpression *key;
	DfsmAstExpression *value;
} DfsmAstDictionaryEntry;

DfsmAstDictionaryEntry *dfsm_ast_dictionary_entry_new (DfsmAstExpression *key, DfsmAstExpression *value) DFSM_CONSTRUCTOR;
void dfsm_ast_dictionary_entry_free (DfsmAstDictionaryEntry *entry);

typedef enum {
	DFSM_AST_DATA_BYTE,
	DFSM_AST_DATA_BOOLEAN,
	DFSM_AST_DATA_INT16,
	DFSM_AST_DATA_UINT16,
	DFSM_AST_DATA_INT32,
	DFSM_AST_DATA_UINT32,
	DFSM_AST_DATA_INT64,
	DFSM_AST_DATA_UINT64,
	DFSM_AST_DATA_DOUBLE,
	DFSM_AST_DATA_STRING,
	DFSM_AST_DATA_OBJECT_PATH,
	DFSM_AST_DATA_SIGNATURE,
	DFSM_AST_DATA_ARRAY,
	DFSM_AST_DATA_STRUCT,
	DFSM_AST_DATA_VARIANT,
	DFSM_AST_DATA_DICT,
	DFSM_AST_DATA_UNIX_FD,
	DFSM_AST_DATA_REGEXP,
	DFSM_AST_DATA_VARIABLE,
} DfsmAstDataStructureType;

typedef struct _DfsmAstVariable DfsmAstVariable;

struct _DfsmAstDataStructure {
	DfsmAstNode parent;
	DfsmAstDataStructureType data_structure_type;
	union {
		guchar byte_val;
		gboolean boolean_val;
		gint16 int16_val;
		guint16 uint16_val;
		gint32 int32_val;
		guint32 uint32_val;
		gint64 int64_val;
		guint64 uint64_val;
		gdouble double_val;
		gchar *string_val;
		gchar *object_path_val;
		gchar *signature_val;
		GPtrArray/*<DfsmAstExpression>*/ *array_val;
		GPtrArray/*<DfsmAstExpression>*/ *struct_val;
		GVariant *variant_val;
		GPtrArray/*<DfsmAstDictionaryEntry>*/ *dict_val;
		gint unix_fd_val;
		gchar *regexp_val;
		DfsmAstVariable *variable_val;
	};
};

DfsmAstDataStructure *dfsm_ast_data_structure_new (DfsmAstDataStructureType data_structure_type, gpointer value, GError **error) DFSM_CONSTRUCTOR;
GVariantType *dfsm_ast_data_structure_calculate_type (DfsmAstDataStructure *data_structure, DfsmEnvironment *environment) DFSM_CONSTRUCTOR;

GVariant *dfsm_ast_data_structure_to_variant (DfsmAstDataStructure *data_structure, DfsmEnvironment *environment, GError **error) DFSM_CONSTRUCTOR;
void dfsm_ast_data_structure_set_from_variant (DfsmAstDataStructure *data_structure, DfsmEnvironment *environment, GVariant *new_value, GError **error);

typedef struct {
	DfsmAstDataStructure parent;
	gdouble weight;
	gboolean is_fuzzy;
} DfsmAstFuzzyDataStructure;

DfsmAstDataStructure *dfsm_ast_fuzzy_data_structure_new (DfsmAstDataStructure *data_structure, gdouble weight, GError **error) DFSM_CONSTRUCTOR;

typedef enum {
	DFSM_AST_TRANSITION_METHOD_CALL,
	DFSM_AST_TRANSITION_ARBITRARY,
} DfsmAstTransitionTrigger;

typedef struct {
	DfsmAstNode parent;
	gchar *from_state_name;
	gchar *to_state_name;
	DfsmAstTransitionTrigger trigger;
	union {
		gchar *method_name; /* for DFSM_AST_TRANSITION_METHOD_CALL, otherwise null */
	} trigger_params;
	GPtrArray *preconditions; /* array of DfsmAstPreconditions */
	GPtrArray *statements; /* array of DfsmAstStatements */
} DfsmAstTransition;

DfsmAstTransition *dfsm_ast_transition_new (const gchar *from_state_name, const gchar *to_state_name, const gchar *transition_type,
                                            GPtrArray/*<DfsmAstPrecondition>*/ *preconditions,
                                            GPtrArray/*<DfsmAstStatement>*/ *statements, GError **error) DFSM_CONSTRUCTOR;
gboolean dfsm_ast_transition_check_preconditions (DfsmAstTransition *transition, DfsmEnvironment *environment, GError **error);
GVariant *dfsm_ast_transition_execute (DfsmAstTransition *transition, DfsmEnvironment *environment, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstNode parent;
	gchar *error_name; /* nullable */
	DfsmAstExpression *condition;
} DfsmAstPrecondition;

DfsmAstPrecondition *dfsm_ast_precondition_new (const gchar *error_name /* nullable */, DfsmAstExpression *condition, GError **error) DFSM_CONSTRUCTOR;
gboolean dfsm_ast_precondition_check (DfsmAstPrecondition *precondition, DfsmEnvironment *environment, GError **error);

typedef enum {
	DFSM_AST_STATEMENT_ASSIGNMENT,
	DFSM_AST_STATEMENT_THROW,
	DFSM_AST_STATEMENT_EMIT,
	DFSM_AST_STATEMENT_REPLY,
} DfsmAstStatementType;

typedef struct _DfsmAstStatement DfsmAstStatement;

typedef GVariant *(*DfsmAstStatementExecuteFunc) (DfsmAstStatement *statement, DfsmEnvironment *environment, GError **error);

/* abstract */
struct _DfsmAstStatement {
	DfsmAstNode parent;
	DfsmAstStatementType statement_type;
	DfsmAstStatementExecuteFunc execute_func;
};

GVariant *dfsm_ast_statement_execute (DfsmAstStatement *statement, DfsmEnvironment *environment, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstStatement parent;
	DfsmAstDataStructure *data_structure; /* lvalue */
	DfsmAstExpression *expression; /* rvalue */
} DfsmAstStatementAssignment;

DfsmAstStatement *dfsm_ast_statement_assignment_new (DfsmAstDataStructure *data_structure,
                                                     DfsmAstExpression *expression, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstStatement parent;
	gchar *error_name;
} DfsmAstStatementThrow;

DfsmAstStatement *dfsm_ast_statement_throw_new (const gchar *error_name, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstStatement parent;
	gchar *signal_name;
	DfsmAstExpression *expression;
} DfsmAstStatementEmit;

DfsmAstStatement *dfsm_ast_statement_emit_new (const gchar *signal_name, DfsmAstExpression *expression, GError **error) DFSM_CONSTRUCTOR;

typedef struct {
	DfsmAstStatement parent;
	DfsmAstExpression *expression;
} DfsmAstStatementReply;

DfsmAstStatement *dfsm_ast_statement_reply_new (DfsmAstExpression *expression, GError **error) DFSM_CONSTRUCTOR;

struct _DfsmAstVariable {
	DfsmAstNode parent;
	DfsmVariableScope scope;
	gchar *variable_name;
};

DfsmAstVariable *dfsm_ast_variable_new (DfsmVariableScope scope, const gchar *variable_name, GError **error) DFSM_CONSTRUCTOR;
GVariantType *dfsm_ast_variable_calculate_type (DfsmAstVariable *variable, DfsmEnvironment *environment) DFSM_CONSTRUCTOR;

GVariant *dfsm_ast_variable_to_variant (DfsmAstVariable *variable, DfsmEnvironment *environment, GError **error) DFSM_CONSTRUCTOR;
void dfsm_ast_variable_set_from_variant (DfsmAstVariable *variable, DfsmEnvironment *environment, GVariant *new_value, GError **error);

G_END_DECLS

#endif /* !DFSM_AST_H */
