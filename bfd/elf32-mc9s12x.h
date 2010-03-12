/* Motorola 68HC11/68HC12-specific support for 32-bit ELF
   Copyright 2003, 2004, 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
   Contributed by Stephane Carrez (stcarrez@nerim.fr)

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _ELF32_MC9S12X_H
#define _ELF32_MC9S12X_H

#include "elf-bfd.h"
#include "bfdlink.h"
#include "elf/mc9s12x.h"

/* Name of symbols exported by MC9S12X linker when there is a memory
   bank window.  */
#define BFD_MC9S12X_BANK_START_NAME   "__bank_start"
#define BFD_MC9S12X_BANK_SIZE_NAME    "__bank_size"
#define BFD_MC9S12X_BANK_VIRTUAL_NAME "__bank_virtual"

/* Set and control ELF flags in ELF header.  */
extern bfd_boolean _bfd_mc9s12x_elf_merge_private_bfd_data (bfd*,bfd*);
extern bfd_boolean _bfd_mc9s12x_elf_set_private_flags (bfd*,flagword);
extern bfd_boolean _bfd_mc9s12x_elf_print_private_bfd_data (bfd*, void*);

/* This hash entry is used to record a trampoline that must be generated
   to call a far function using a normal calling convention ('jsr').
   The trampoline is used when a pointer to a far function is used.
   It takes care of installing the proper memory bank as well as creating
   the 'call/rtc' calling convention.  */
struct elf32_mc9s12x_stub_hash_entry {

  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump.  */
  bfd_vma target_value;
  asection *target_section;
};

/* Placeholder for the parameters to compute memory page and physical address.
   The following formulas are used:

   sym > bank_virtual =>
     %addr(sym) = (((sym - bank_virtual) & bank_mask) + bank_physical
     %page(sym) = (((sym - bank_virtual) >> bank_shift) % 256

   sym < bank_virtual =>
     %addr(sym) = sym
     %page(sym) = 0


   These parameters are obtained from the symbol table by looking
   at the following:

   __bank_start         Symbol marking the start of memory bank window
                        (bank_physical)
   __bank_virtual       Logical address of symbols for which the transformation
                        must be computed
   __bank_page_size     Size in bytes of page size (this is *NOT* the memory
                        bank window size and the window size is always
                        less or equal to the page size)

   For 68HC12, the window is at 0x8000 and the page size is 16K (full window).
   For 68HC11 this is board specific (implemented by external hardware).

*/
struct mc9s12x_page_info
{
  bfd_vma bank_virtual;
  bfd_vma bank_physical;
  bfd_vma bank_physical_end;
  bfd_vma bank_mask;
  bfd_vma bank_size;
  int bank_shift;
  int bank_param_initialized;
  bfd_vma trampoline_addr;
};

struct mc9s12x_elf_link_hash_table
{
  struct elf_link_hash_table root;
  struct mc9s12x_page_info pinfo;

  /* The stub hash table.  */
  struct bfd_hash_table* stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  asection* stub_section;
  asection* tramp_section;

  /* Linker call-backs.  */
  asection * (*add_stub_section) PARAMS ((const char *, asection *));

  /* Assorted information used by elf32_hppa_size_stubs.  */
  unsigned int bfd_count;
  int top_index;
  asection **input_list;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  bfd_boolean (* size_one_stub) PARAMS((struct bfd_hash_entry*, void*));
  bfd_boolean (* build_one_stub) PARAMS((struct bfd_hash_entry*, void*));
};

/* Get the Sparc64 ELF linker hash table from a link_info structure.  */

#define mc9s12x_elf_hash_table(p) \
  ((struct mc9s12x_elf_link_hash_table *) ((p)->hash))

/* Create a 68HC11/68HC12 ELF linker hash table.  */

extern struct mc9s12x_elf_link_hash_table* mc9s12x_elf_hash_table_create
  (bfd*);
extern void mc9s12x_elf_bfd_link_hash_table_free (struct bfd_link_hash_table*);

extern void mc9s12x_elf_get_bank_parameters (struct bfd_link_info*);

/* Return 1 if the address is in banked memory.
   This can be applied to a virtual address and to a physical address.  */
extern int mc9s12x_addr_is_banked (struct mc9s12x_page_info*, bfd_vma);

/* Return the physical address seen by the processor, taking
   into account banked memory.  */
extern bfd_vma mc9s12x_phys_addr (struct mc9s12x_page_info*, bfd_vma);

/* Return the page number corresponding to an address in banked memory.  */
extern bfd_vma mc9s12x_phys_page (struct mc9s12x_page_info*, bfd_vma);

bfd_reloc_status_type mc9s12x_elf_ignore_reloc
  (bfd *abfd, arelent *reloc_entry,
   asymbol *symbol, void *data, asection *input_section,
   bfd *output_bfd, char **error_message);
bfd_reloc_status_type mc9s12x_elf_special_reloc
  (bfd *abfd, arelent *reloc_entry,
    asymbol *symbol, void *data, asection *input_section,
    bfd *output_bfd, char **error_message);

bfd_boolean elf32_mc9s12x_check_relocs
  (bfd * abfd, struct bfd_link_info * info,
   asection * sec, const Elf_Internal_Rela * relocs);
bfd_boolean elf32_mc9s12x_relocate_section
  (bfd *output_bfd, struct bfd_link_info *info,
   bfd *input_bfd, asection *input_section,
   bfd_byte *contents, Elf_Internal_Rela *relocs,
   Elf_Internal_Sym *local_syms, asection **local_sections);

bfd_boolean elf32_mc9s12x_add_symbol_hook
  (bfd *abfd, struct bfd_link_info *info,
   Elf_Internal_Sym *sym, const char **namep,
   flagword *flagsp, asection **secp,
   bfd_vma *valp);

/* Tweak the OSABI field of the elf header.  */

extern void elf32_mc9s12x_post_process_headers (bfd*, struct bfd_link_info*);

int elf32_mc9s12x_setup_section_lists (bfd *, struct bfd_link_info *);

bfd_boolean elf32_mc9s12x_size_stubs
  (bfd *, bfd *, struct bfd_link_info *,
   asection * (*) (const char *, asection *));

bfd_boolean elf32_mc9s12x_build_stubs (bfd* abfd, struct bfd_link_info *);
#endif
