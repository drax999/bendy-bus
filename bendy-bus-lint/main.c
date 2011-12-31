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

#include "config.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <dfsm/dfsm.h>

enum StatusCodes {
	STATUS_SUCCESS = 0,
	STATUS_INVALID_OPTIONS = 1,
	STATUS_UNREADABLE_FILE = 2,
	STATUS_INVALID_CODE = 3,
};

static void
print_help_text (GOptionContext *context)
{
	gchar *help_text;

	help_text = g_option_context_get_help (context, TRUE, NULL);
	puts (help_text);
	g_free (help_text);
}

int
main (int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;
	const gchar *simulation_filename, *introspection_filename;
	gchar *simulation_code, *introspection_xml;
	GPtrArray/*<DfsmObject>*/ *simulated_objects;

	/* Set up localisation. */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	g_type_init ();
	g_set_application_name (_("D-Bus Simulator Lint"));

	/* Parse command line options */
	context = g_option_context_new (_("[simulation code file] [introspection XML file]"));
	g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
	g_option_context_set_summary (context, _("Checks the FSM simulation code for a D-Bus client–server conversation simulation."));

	if (g_option_context_parse (context, &argc, &argv, &error) == FALSE) {
		g_printerr (_("Error parsing command line options: %s"), error->message);
		g_printerr ("\n");

		print_help_text (context);

		g_error_free (error);
		g_option_context_free (context);

		exit (STATUS_INVALID_OPTIONS);
	}

	/* Extract the simulation and the introspection filenames. */
	if (argc < 3) {
		g_printerr (_("Error parsing command line options: %s"), _("Simulation and introspection filenames must be provided"));
		g_printerr ("\n");

		print_help_text (context);

		g_option_context_free (context);

		exit (STATUS_INVALID_OPTIONS);
	}

	simulation_filename = argv[1];
	introspection_filename = argv[2];

	g_option_context_free (context);

	/* Load the files. */
	g_file_get_contents (simulation_filename, &simulation_code, NULL, &error);

	if (error != NULL) {
		g_printerr (_("Error loading simulation code from file ‘%s’: %s"), simulation_filename, error->message);
		g_printerr ("\n");

		g_error_free (error);

		exit (STATUS_UNREADABLE_FILE);
	}

	g_file_get_contents (introspection_filename, &introspection_xml, NULL, &error);

	if (error != NULL) {
		g_printerr (_("Error loading introspection XML from file ‘%s’: %s"), introspection_filename, error->message);
		g_printerr ("\n");

		g_error_free (error);
		g_free (simulation_code);

		exit (STATUS_UNREADABLE_FILE);
	}

	/* Build the DfsmObjects and thus check the simulation code. */
	simulated_objects = dfsm_object_factory_from_files (simulation_code, introspection_xml, &error);

	g_free (introspection_xml);
	g_free (simulation_code);

	if (error != NULL) {
		g_printerr (_("Error creating simulated DFSMs: %s"), error->message);
		g_printerr ("\n");

		g_error_free (error);

		exit (STATUS_INVALID_CODE);
	}

	g_ptr_array_unref (simulated_objects);

	return STATUS_SUCCESS;
}