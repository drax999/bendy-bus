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

#include "dfsm-utils.h"

static gboolean
_is_member_name (const gchar *member_name)
{
	const gchar *i;

	g_return_val_if_fail (member_name != NULL, FALSE);

	/* Characters outside [A-Za-z0-9_]? */
	for (i = member_name; *i != '\0'; i++) {
		if (*i != '_' && g_ascii_isalnum (*i) == FALSE) {
			return FALSE;
		}
	}

	/* Zero-length string? */
	if (i == member_name) {
		return FALSE;
	}

	return TRUE;
}

/**
 * dfsm_is_variable_name:
 * @variable_name: variable name to check
 *
 * Checks whether @variable_name is a valid variable name in the FSM language. Valid variable names conform to the restrictions on D-Bus member names,
 * as specified in the <ulink url="http://dbus.freedesktop.org/doc/dbus-specification.html#sect3" type="http">D-Bus Specification</ulink>,
 * § Member names.
 *
 * Return value: %TRUE if @variable_name is a valid variable name; %FALSE otherwise
 */
gboolean
dfsm_is_variable_name (const gchar *variable_name)
{
	return _is_member_name (variable_name);
}

/**
 * dfsm_is_state_name:
 * @state_name: state name to check
 *
 * Checks whether @state_name is a valid state name in the FSM language. Valid state names conform to the restrictions on D-Bus member names,
 * as specified in the <ulink url="http://dbus.freedesktop.org/doc/dbus-specification.html#sect3" type="http">D-Bus Specification</ulink>,
 * § Member names.
 *
 * Return value: %TRUE if @state_name is a valid state name; %FALSE otherwise
 */
gboolean
dfsm_is_state_name (const gchar *state_name)
{
	return _is_member_name (state_name);
}

/**
 * dfsm_is_function_name:
 * @function_name: function name to check
 *
 * Checks whether @function_name is a valid function name in the FSM language. Valid function names conform to the restrictions on D-Bus member names,
 * as specified in the <ulink url="http://dbus.freedesktop.org/doc/dbus-specification.html#sect3" type="http">D-Bus Specification</ulink>,
 * § Member names.
 *
 * Return value: %TRUE if @function_name is a valid function name; %FALSE otherwise
 */
gboolean
dfsm_is_function_name (const gchar *function_name)
{
	return _is_member_name (function_name);
}
