/* addr2line.c -- convert addresses to line number and function name
   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2009  Free Software Foundation, Inc.
   Contributed by Ulrich Lauther <Ulrich.Lauther@mchp.siemens.de>

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* Derived from objdump.c and nm.c by Ulrich.Lauther@mchp.siemens.de

   Usage:
   addr2line [options] addr addr ...
   or
   addr2line [options]

   both forms write results to stdout, the second form reads addresses
   to be converted from stdin.  */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#include "bfd.h"

static asymbol **syms;		/* Symbol table.  */
static char *_output;

static int slurp_symtab(bfd *);
static void find_address_in_section(bfd *, asection *, void *);
static void translate_addresses(bfd *);

int print_output(const char* format, ...)
{
	int retVal;

	va_list args;
	va_start(args, format);
	retVal = vsprintf(_output, format, args);
	va_end(args);

	return retVal;
}


/* Read in the symbol table.  */

static int
slurp_symtab(bfd *abfd)
{
	long storage;
	long symcount;
	bfd_boolean dynamic = FALSE;

	if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
	{
		return 1;
	}

	storage = bfd_get_symtab_upper_bound(abfd);

	if (storage == 0)
	{
		storage = bfd_get_dynamic_symtab_upper_bound(abfd);
		dynamic = TRUE;
	}

	if (storage < 0)
	{
		return 1;
	}

	syms = (asymbol **) malloc(storage);

	if (dynamic)
	{
		symcount = bfd_canonicalize_dynamic_symtab(abfd, syms);
	}
	else
	{
		symcount = bfd_canonicalize_symtab(abfd, syms);
	}

	if (symcount < 0)
	{
		return 1;
	}
}

/* These global variables are used to pass information between
   translate_addresses and find_address_in_section.  */

static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static bfd_boolean found;

/* Look for an address in a section.  This is called via
   bfd_map_over_sections.  */

static void
find_address_in_section(bfd *abfd, asection *section,
						void *data ATTRIBUTE_UNUSED)
{
	bfd_vma vma;
	bfd_size_type size;

	if (found)
	{
		return;
	}

	if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
	{
		return;
	}

	vma = bfd_get_section_vma(abfd, section);

	if (pc < vma)
	{
		return;
	}

	size = bfd_get_section_size(section);

	if (pc >= vma + size)
	{
		return;
	}

	found = bfd_find_nearest_line(abfd, section, syms, pc - vma,
								  &filename, &functionname, &line);
}

/* Read hexadecimal addresses from stdin, translate into
   file_name:line_number and optionally function name.  */

static void
translate_addresses(bfd *abfd)
{
	found = FALSE;
	bfd_map_over_sections(abfd, find_address_in_section, NULL);

	if (! found)
	{
		print_output("??:0\n");
	}
	else
	{
		print_output("%s:%u\n", filename ? filename : "??", line);
		found = FALSE;
	}
}

/* Process a file.  Returns an exit value for main().  */

static int
process_file(const char *file_name)
{
	bfd *abfd;
	char **matching;

	abfd = bfd_openr(file_name, NULL);

	if (abfd == NULL)
	{
		return 1;
	}

	/* Decompress sections.  */
	abfd->flags |= BFD_DECOMPRESS;

	if (bfd_check_format(abfd, bfd_archive))
	{
		print_output("%s: cannot get addresses from archive", file_name);
		return 1;
	}

	if (! bfd_check_format_matches(abfd, bfd_object, &matching))
	{
		return 1;
	}

	if (slurp_symtab(abfd) == 1)
	{
		return 1;
	}

	translate_addresses(abfd);

	if (syms != NULL)
	{
		free(syms);
		syms = NULL;
	}

	bfd_close(abfd);

	return 0;
}

int
addr2line(const char *file_name, intptr_t address, char *output)
{
	bfd_init();

	pc = address;
	_output = output;

	return process_file(file_name);
}
