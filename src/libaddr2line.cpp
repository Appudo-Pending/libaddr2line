/* addr2line.c -- convert addresses to line number and function name
   Copyright (C) 1997-2014 Free Software Foundation, Inc.
   Contributed by Ulrich Lauther <Ulrich.Lauther@mchp.siemens.de>

   Portions Copyright (C) 2016
        403fd4d072f534ee5bd7da6efc9462f3995bb456bad644cd9bb7bcaad314b02d source@appudo.com
        d3c2e0357cde0e67a0649c55dda800615d5be0c8ea9845aeda270d0fe57c1363 source@appudo.com

   This file was part of GNU Binutils.

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
#include <stdio.h>
#include "bfd.h"
#include "getopt.h"
#include <libiberty/demangle.h>

extern "C" char * strtok ( char * str, const char * delimiters );
extern "C" const char *strrchr (const char *__s, int __c)
     __THROW __asm ("strrchr") __attribute_pure__ __nonnull ((1));

void bfd_fatal (const char * v)
{
    fprintf(stderr, "fatal error: %s\n", v);
    exit(EXIT_FAILURE);
}

void fatal (const char * __format, ...)
{
    va_list args;
    va_start(args, __format);
    vfprintf(stderr, __format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void bfd_nonfatal (const char* v)
{
    fprintf(stderr, "nonfatal error: %s\n", v);
}

void set_default_bfd_target (void)
{

}

void list_matching_formats (char **)
{

}

off_t get_file_size (const char *)
{
    return 1;
}

#define xvec_get_elf_backend_data(xvec) \
  ((const struct elf_backend_data *) (xvec)->backend_data)

#define get_elf_backend_data(abfd) \
   xvec_get_elf_backend_data ((abfd)->xvec)

static struct option long_options[] =
{
  {"addresses", no_argument, NULL, 'a'},
  {"basenames", no_argument, NULL, 's'},
  {"demangle", optional_argument, NULL, 'C'},
  {"exe", required_argument, NULL, 'e'},
  {"functions", no_argument, NULL, 'f'},
  {"inlines", no_argument, NULL, 'i'},
  {"pretty-print", no_argument, NULL, 'p'},
  {"section", required_argument, NULL, 'j'},
  {"target", required_argument, NULL, 'b'},
  {"help", no_argument, NULL, 'H'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, 0, 0}
};

int splitAttr(char* attr, char** args, int max)
{
    int idx = 0;
    char* arg = strtok(attr, " ");
    while(max-- && arg)
    {
        args[idx++] = arg;
        arg = strtok(NULL, " ");
    }

    return idx;
}

class Addr2Line
{
public:
    Addr2Line(char* buffer, int bufferSize)
        : unwind_inlines(false)
        , with_addresses(false)
        , with_functions(false)
        , do_demangle(true)
        , pretty_print(false)
        , base_names(false)
        , naddr(0)
        , addr(NULL)
        , syms(NULL)
        , pc(0)
        , filename(NULL)
        , functionname(NULL)
        , line(0)
        , discriminator(0)
        , found(false)
        , outBuffer(buffer)
        , outLeft(bufferSize)
    {

    }

    ~Addr2Line()
    {
        if(syms != NULL)
            free(syms);
    }

    int
    process_file (const char *file_name, const char *section_name,
              const char *target, int32_t fd = -1);

    int run(char* attr, int32_t fd = -1);
private:
    void writeOut(const char* data, ...);
    void slurp_symtab (bfd *);
    static void find_address_in_section (bfd *, asection *, void *);
    void find_offset_in_section (bfd *, asection *);
    void translate_addresses (bfd *, asection *);
    bfd_boolean unwind_inlines;	/* -i, unwind inlined functions. */
    bfd_boolean with_addresses;	/* -a, show addresses.  */
    bfd_boolean with_functions;	/* -f, show function names.  */
    bfd_boolean do_demangle;		/* -C, demangle names.  */
    bfd_boolean pretty_print;	/* -p, print on one line.  */
    bfd_boolean base_names;		/* -s, strip directory names.  */

    int naddr;		/* Number of addresses to process.  */
    char **addr;		/* Hex addresses to process.  */

    asymbol **syms;		/* Symbol table.  */

    bfd_vma pc;
    const char *filename;
    const char *functionname;
    unsigned int line;
    unsigned int discriminator;
    bfd_boolean found;

    char* outBuffer;
    int   outLeft;
};

/* Print a usage message to STREAM and exit with STATUS.  */


void Addr2Line::writeOut(const char* __format, ...)
{
    int size;
    va_list args;
    va_start(args, __format);
    if((size = vsnprintf(outBuffer, outLeft, __format, args)) >= outLeft)
    {
        outLeft = 0;
    }
    else
    {
        outLeft -= size;
        outBuffer += size;
    }
    va_end(args);
}

/* Read in the symbol table.  */

void
Addr2Line::slurp_symtab (bfd *abfd)
{
  long storage;
  long symcount;
  bfd_boolean dynamic = FALSE;

  if ((bfd_get_file_flags (abfd) & HAS_SYMS) == 0)
    return;

  storage = bfd_get_symtab_upper_bound (abfd);
  if (storage == 0)
    {
      storage = bfd_get_dynamic_symtab_upper_bound (abfd);
      dynamic = TRUE;
    }
  if (storage < 0)
    bfd_fatal (bfd_get_filename (abfd));

  syms = (asymbol **) malloc (storage);
  if (dynamic)
    symcount = bfd_canonicalize_dynamic_symtab (abfd, syms);
  else
    symcount = bfd_canonicalize_symtab (abfd, syms);
  if (symcount < 0)
    bfd_fatal (bfd_get_filename (abfd));

  /* If there are no symbols left after canonicalization and
     we have not tried the dynamic symbols then give them a go.  */
  if (symcount == 0
      && ! dynamic
      && (storage = bfd_get_dynamic_symtab_upper_bound (abfd)) > 0)
    {
      free (syms);
      syms = (asymbol**)malloc (storage);
      symcount = bfd_canonicalize_dynamic_symtab (abfd, syms);
    }
}

/* These global variables are used to pass information between
   translate_addresses and find_address_in_section.  */

/* Look for an address in a section.  This is called via
   bfd_map_over_sections.  */

void
Addr2Line::find_address_in_section (bfd *abfd, asection *section,
             void *data)
{
    Addr2Line* v = (Addr2Line*)data;
  bfd_vma vma;
  bfd_size_type size;

  if (v->found)
    return;

  if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
    return;

  vma = bfd_get_section_vma (abfd, section);
  if (v->pc < vma)
    return;

  size = bfd_get_section_size (section);
  if (v->pc >= vma + size)
    return;

  v->found = bfd_find_nearest_line_discriminator (abfd, section, v->syms, v->pc - vma,
                                               &v->filename, &v->functionname,
                                               &v->line, &v->discriminator);
}

/* Look for an offset in a section.  This is directly called.  */

void
Addr2Line::find_offset_in_section (bfd *abfd, asection *section)
{
  bfd_size_type size;

  if (found)
    return;

  if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
    return;

  size = bfd_get_section_size (section);
  if (pc >= size)
    return;

  found = bfd_find_nearest_line_discriminator (abfd, section, syms, pc,
                                               &filename, &functionname,
                                               &line, &discriminator);
}

/* Read hexadecimal addresses from stdin, translate into
   file_name:line_number and optionally function name.  */

void
Addr2Line::translate_addresses (bfd *abfd, asection *section)
{
  int read_stdin = (naddr == 0);

  for (;;)
    {
      if (read_stdin)
    {
      char addr_hex[100];

      if (fgets (addr_hex, sizeof addr_hex, stdin) == NULL)
        break;
      pc = bfd_scan_vma (addr_hex, NULL, 16);
    }
      else
    {
      if (naddr <= 0)
        break;
      --naddr;
      pc = bfd_scan_vma (*addr++, NULL, 16);
    }

      if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
          int arch_size = bfd_get_arch_size(abfd);
      bfd_vma sign = (bfd_vma) 1 << (arch_size - 1);

      pc &= (sign << 1) - 1;
      if (bfd_get_sign_extend_vma (abfd))
        pc = (pc ^ sign) - sign;
    }

      if (with_addresses)
        {
          char tmp[64];
          writeOut ("0x");
          bfd_sprintf_vma (abfd, tmp, pc);
          writeOut(tmp);

          if (pretty_print)
            writeOut (": ");
          else
            writeOut ("\n");
        }

      found = FALSE;
      if (section)
    find_offset_in_section (abfd, section);
      else
    bfd_map_over_sections (abfd, find_address_in_section, this);

      if (! found)
    {
      if (with_functions)
        {
          if (pretty_print)
        writeOut ("?? ");
          else
        writeOut ("??\n");
        }
      writeOut ("??:0\n");
    }
      else
    {
      while (1)
            {
              if (with_functions)
                {
                  const char *name;
                  char *alloc = NULL;

                  name = functionname;
                  if (name == NULL || *name == '\0')
                    name = "??";
                  else if (do_demangle)
                    {
                      alloc = bfd_demangle (abfd, name, DMGL_ANSI | DMGL_PARAMS);
                      if (alloc != NULL)
                        name = alloc;
                    }

                  writeOut ("%s", name);
                  if (pretty_print)
            /* Note for translators:  This writeOut is used to join the
               function name just printed above to the line number/
               file name pair that is about to be printed below.  Eg:

                 foo at 123:bar.c  */
                    writeOut ( " at ");
                  else
                    writeOut ("\n");

                  if (alloc != NULL)
                    free (alloc);
                }

              if (base_names && filename != NULL)
                {
                  char *h;

                  h = (char*)strrchr (filename, '/');
                  if (h != NULL)
                    filename = h + 1;
                }

              writeOut ("%s:", filename ? filename : "??");
          if (line != 0)
                {
                  if (discriminator != 0)
                    writeOut ("%u (discriminator %u)\n", line, discriminator);
                  else
                    writeOut ("%u\n", line);
                }
          else
        writeOut ("?\n");
              if (!unwind_inlines)
                found = FALSE;
              else
                found = bfd_find_inliner_info (abfd, &filename, &functionname,
                           &line);
              if (! found)
                break;
              if (pretty_print)
        /* Note for translators: This writeOut is used to join the
           line number/file name pair that has just been printed with
           the line number/file name pair that is going to be printed
           by the next iteration of the while loop.  Eg:

             123:bar.c (inlined by) 456:main.c  */
                writeOut (" (inlined by) ");
            }
    }
    }
}

/* Process a file.  Returns an exit value for main().  */

int
Addr2Line::process_file (const char *file_name, const char *section_name,
          const char *target, int32_t fd)
{
  bfd *abfd;
  asection *section;
  char **matching;

  if (get_file_size (file_name) < 1)
    return 1;

  if(fd != -1)
      abfd = bfd_fdopenr (file_name, target, fd);
  else
    abfd = bfd_openr (file_name, target);
  if (abfd == NULL)
    bfd_fatal (file_name);

  /* Decompress sections.  */
  abfd->flags |= BFD_DECOMPRESS;

  if (bfd_check_format (abfd, bfd_archive))
    fatal ("%s: cannot get addresses from archive", file_name);

  if (! bfd_check_format_matches (abfd, bfd_object, &matching))
    {
      bfd_nonfatal (bfd_get_filename (abfd));
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
    {
      list_matching_formats (matching);
      free (matching);
    }
      fatal("error");
    }

  if (section_name != NULL)
    {
      section = bfd_get_section_by_name (abfd, section_name);
      if (section == NULL)
    fatal ("%s: cannot find section %s", file_name, section_name);
    }
  else
    section = NULL;

  slurp_symtab (abfd);

  translate_addresses (abfd, section);

  if (syms != NULL)
    {
      free (syms);
      syms = NULL;
    }

  bfd_close (abfd);

  return 0;
}

__attribute__((constructor))
void addr2line_init()
{
    bfd_init ();
    set_default_bfd_target ();
}

int Addr2Line::run(char* attr, int32_t fd)
{
    char* argv[128];
    int argc = splitAttr(attr, argv, 128-1);
    const char *file_name = NULL;
    const char *section_name = NULL;
    char *target = NULL;
    int c;

    argv[argc] = NULL;
    optind = 0;

    while ((c = getopt_long (argc, argv, "ab:Ce:sfHhij:pVv", long_options, (int *) 0))
        != EOF)
       {
         switch (c)
       {
       case 0:
         break;		/* We've been given a long option.  */
       case 'a':
         with_addresses = TRUE;
         break;
       case 'b':
         target = optarg;
         break;
       case 'C':
         do_demangle = TRUE;
         if (optarg != NULL)
           {
             enum demangling_styles style;

             style = cplus_demangle_name_to_style (optarg);
             if (style == unknown_demangling)
           fatal ("unknown demangling style `%s'",
                  optarg);

             cplus_demangle_set_style (style);
           }
         break;
       case 'e':
         file_name = optarg;
         break;
       case 's':
         base_names = TRUE;
         break;
       case 'f':
         with_functions = TRUE;
         break;
           case 'p':
             pretty_print = TRUE;
             break;
       case 'v':
       case 'V':
         break;
       case 'h':
       case 'H':
         break;
       case 'i':
         unwind_inlines = TRUE;
         break;
       case 'j':
         section_name = optarg;
         break;
       default:
         break;
       }
       }

     if (file_name == NULL)
       file_name = "a.out";

     addr = argv + optind;
     naddr = argc - optind;

    return process_file(file_name, section_name, target, fd);
}

extern "C"
 __attribute__((visibility("default"))) int addr2line(char* attr, char* outfile, int outSize, int fd)
{
    Addr2Line v(outfile, outSize);

    return v.run(attr, fd);
}

/*
int
main (int argc, char **argv)
{
    char tmp[4096];
    if(!addr2line(argv[1], argv[2], tmp, sizeof(tmp)))
        fprintf(stderr, "%s\n", tmp);
}
*/
