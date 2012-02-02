/* Freescale XGATE-specific support for 32-bit ELF
   Copyright 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
   Contributed by Sean Keys(skeys@ipdatasys.com)
   
   Contributed by Stephane Carrez (stcarrez@nerim.fr)
   (Heavily copied from the D10V port by Martin Hunt (hunt@cygnus.com))

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
   MA 02110-1301, USA.
   Can't disassemble for architecture
   in elf_set_mach_from_flags
   */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf32-xgate.h"
#include "elf/xgate.h"
#include "opcode/xgate.h"

/* Relocation functions.  */
static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
(bfd *, bfd_reloc_code_real_type);
static void xgate_info_to_howto_rel
(bfd *, arelent *, Elf_Internal_Rela *);

/* Trampoline generation.  */
static bfd_boolean xgate_elf_size_one_stub
(struct bfd_hash_entry *gen_entry, void *in_arg);
//static bfd_boolean xgate_elf_build_one_stub
//(struct bfd_hash_entry *gen_entry, void *in_arg);
static struct bfd_link_hash_table* xgate_elf_bfd_link_hash_table_create
(bfd*);

static bfd_boolean xgate_elf_set_mach_from_flags PARAMS ((bfd *));

/* Try to minimize the amount of space occupied by relocation tables
   on the ROM (not that the ROM won't be swamped by other ELF overhead).  */
#define USE_REL 1

/* todo add brief  */

static reloc_howto_type elf_xgate_howto_table[] = {
/* This reloc does nothing.  */
HOWTO (R_XGATE_NONE, /* type */
		0, /* rightshift */
		2, /* size (0 = byte, 1 = short, 2 = long) */
		32, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont,/* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_NONE", /* name */
		FALSE, /* partial_inplace */
		0, /* src_mask */
		0, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 8 bit absolute relocation */
HOWTO (R_XGATE_8, /* type */
		0, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		8, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_bitfield, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_8", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */

///* A 8 bit absolute relocation (upper address) */
//HOWTO (R_XGATE_HI8, /* type */
//		8, /* rightshift */
//		0, /* size (0 = byte, 1 = short, 2 = long) */
//		8, /* bitsize */
//		FALSE, /* pc_relative */
//		0, /* bitpos */
//		complain_overflow_bitfield, /* complain_on_overflow */
//		bfd_elf_generic_reloc, /* special_function */
//		"R_XGATE_HI8", /* name */
//		FALSE, /* partial_inplace */
//		0x00ff, /* src_mask */
//		0x00ff, /* dst_mask */
//		FALSE), /* pcrel_offset */

///* A 8 bit absolute relocation (upper address) */
//HOWTO (R_XGATE_LO8, /* type */
//		0, /* rightshift */
//		0, /* size (0 = byte, 1 = short, 2 = long) */
//		8, /* bitsize */
//		FALSE, /* pc_relative */
//		0, /* bitpos */
//		complain_overflow_dont, /* complain_on_overflow */
//		bfd_elf_generic_reloc, /* special_function */
//		"R_XGATE_LO8", /* name */
//		FALSE, /* partial_inplace */
//		0x00ff, /* src_mask */
//		0x00ff, /* dst_mask */
//		FALSE), /* pcrel_offset */

/* A 8 bit PC-rel relocation */
HOWTO (R_XGATE_PCREL_8, /* type */
		0, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		8, /* bitsize */
		TRUE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_bitfield, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_PCREL_8", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		TRUE), /* pcrel_offset */

/* A 16 bit absolute relocation */
HOWTO (R_XGATE_16, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont /*bitfield */, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_16", /* name */
		FALSE, /* partial_inplace */
		0xffff, /* src_mask */
		0xffff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 32 bit absolute relocation.  This one is never used for the
 code relocation.  It's used by gas for -gstabs generation.  */
HOWTO (R_XGATE_32, /* type */
		0, /* rightshift */
		2, /* size (0 = byte, 1 = short, 2 = long) */
		32, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_bitfield, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_32", /* name */
		FALSE, /* partial_inplace */
		0xffffffff, /* src_mask */
		0xffffffff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 3 bit absolute relocation */
HOWTO (R_XGATE_3B, /* type */
		0, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		3, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_bitfield, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_4B", /* name */
		FALSE, /* partial_inplace */
		0x003, /* src_mask */
		0x003, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 16 bit PC-rel relocation */
HOWTO (R_XGATE_PCREL_16, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		TRUE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_PCREL_16", /* name */
		FALSE, /* partial_inplace */
		0xffff, /* src_mask */
		0xffff, /* dst_mask */
		TRUE), /* pcrel_offset */

/* GNU extension to record C++ vtable hierarchy */
HOWTO (R_XGATE_GNU_VTINHERIT, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		0, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		NULL, /* special_function */
		"R_XGATE_GNU_VTINHERIT", /* name */
		FALSE, /* partial_inplace */
		0, /* src_mask */
		0, /* dst_mask */
		FALSE), /* pcrel_offset */

/* GNU extension to record C++ vtable member usage */
HOWTO (R_XGATE_GNU_VTENTRY, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		0, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		_bfd_elf_rel_vtable_reloc_fn, /* special_function */
		"R_XGATE_GNU_VTENTRY", /* name */
		FALSE, /* partial_inplace */
		0, /* src_mask */
		0, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 24 bit relocation */
HOWTO (R_XGATE_24, /* type */
		0, /* rightshift */
		2, /* size (0 = byte, 1 = short, 2 = long) */
		24, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_24", /* name */
		FALSE, /* partial_inplace */
		0xffffff, /* src_mask */
		0xffffff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 16-bit low relocation */
HOWTO (R_XGATE_LO16, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc,/* special_function */
		"R_XGATE_LO16", /* name */
		FALSE, /* partial_inplace */
		0xffff, /* src_mask */
		0xffff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A page relocation */
HOWTO (R_XGATE_PAGE, /* type */
		0, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		8, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc,/* special_function */
		"R_XGATE_PAGE", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */
/* A 9 bit absolute relocation  */
HOWTO (R_XGATE_PCREL_9, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		9, /* bitsize */
		TRUE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_bitfield, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_PCREL_9", /* name */
		FALSE, /* partial_inplace */
		0xffff, /* src_mask */
		0xffff, /* dst_mask */
		TRUE), /* pcrel_offset */

/* A 8 bit absolute relocation (upper address) */
HOWTO (R_XGATE_PCREL_10, /* type */
		8, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		10, /* bitsize */
		TRUE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_PCREL_10", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		TRUE), /* pcrel_offset */

/* A 8 bit absolute relocation  */
HOWTO (R_XGATE_IMM8_LO, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_IMM8_LO", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A 16 bit absolute relocation (upper address) */
HOWTO (R_XGATE_IMM8_HI, /* type */
		8, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_IMM8_HI", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */
/* A 3 bit absolute relocation */
HOWTO (R_XGATE_IMM3, /* type */
		8, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_IMM3", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */
/* A 4 bit absolute relocation  */
HOWTO (R_XGATE_IMM4, /* type */
		8, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_IMM4", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */
/* A 5 bit absolute relocation  */
HOWTO (R_XGATE_IMM5, /* type */
		8, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		16, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		bfd_elf_generic_reloc, /* special_function */
		"R_XGATE_IMM5", /* name */
		FALSE, /* partial_inplace */
		0x00ff, /* src_mask */
		0x00ff, /* dst_mask */
		FALSE), /* pcrel_offset */
        //EMPTY_HOWTO (14),
		//EMPTY_HOWTO (15),
		//EMPTY_HOWTO (16),
		//EMPTY_HOWTO (17),
		//EMPTY_HOWTO (18),
		//EMPTY_HOWTO (19),
/* Mark beginning of a jump instruction (any form).  */
HOWTO (R_XGATE_RL_JUMP, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		0, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		xgate_elf_ignore_reloc, /* special_function */
		"R_XGATE_RL_JUMP", /* name */
		TRUE, /* partial_inplace */
		0, /* src_mask */
		0, /* dst_mask */
		TRUE), /* pcrel_offset */

/* Mark beginning of Gcc relaxation group instruction.  */
HOWTO (R_XGATE_RL_GROUP, /* type */
		0, /* rightshift */
		1, /* size (0 = byte, 1 = short, 2 = long) */
		0, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		xgate_elf_ignore_reloc, /* special_function */
		"R_XGATE_RL_GROUP", /* name */
		TRUE, /* partial_inplace */
		0, /* src_mask */
		0, /* dst_mask */
		TRUE), /* pcrel_offset */
};


/* Map BFD reloc types to M68HC11 ELF reloc types.  */

struct xgate_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct xgate_reloc_map xgate_reloc_map[] = {
    {BFD_RELOC_NONE, R_XGATE_NONE,},
    {BFD_RELOC_8, R_XGATE_8},
//    {BFD_RELOC_XGATE_HI8, R_XGATE_HI8},
//    {BFD_RELOC_XGATE_LO8, R_XGATE_LO8},
    {BFD_RELOC_8_PCREL, R_XGATE_PCREL_8},
    {BFD_RELOC_16_PCREL, R_XGATE_PCREL_16},
    {BFD_RELOC_16, R_XGATE_16},
    {BFD_RELOC_32, R_XGATE_32},
    {BFD_RELOC_XGATE_3B, R_XGATE_3B},

    {BFD_RELOC_VTABLE_INHERIT, R_XGATE_GNU_VTINHERIT},
    {BFD_RELOC_VTABLE_ENTRY, R_XGATE_GNU_VTENTRY},

    {BFD_RELOC_XGATE_LO16, R_XGATE_LO16},
    {BFD_RELOC_XGATE_PAGE, R_XGATE_PAGE},
    {BFD_RELOC_XGATE_24, R_XGATE_24},
    {BFD_RELOC_XGATE_PCREL_9, R_XGATE_PCREL_9},
    {BFD_RELOC_XGATE_PCREL_10,  R_XGATE_PCREL_10},
    {BFD_RELOC_XGATE_IMM8_LO, R_XGATE_IMM8_LO},
    {BFD_RELOC_XGATE_IMM8_HI, R_XGATE_IMM8_HI},
    {BFD_RELOC_XGATE_IMM3, R_XGATE_IMM3},
    {BFD_RELOC_XGATE_IMM4, R_XGATE_IMM4},
    {BFD_RELOC_XGATE_IMM5, R_XGATE_IMM5},

    {BFD_RELOC_XGATE_RL_JUMP, R_XGATE_RL_JUMP},
    {BFD_RELOC_XGATE_RL_GROUP, R_XGATE_RL_GROUP},
};

static struct bfd_hash_entry *stub_hash_newfunc
(struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

static bfd_boolean xgate_elf_export_one_stub
(struct bfd_hash_entry *gen_entry, void *in_arg);

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
    bfd_reloc_code_real_type code)
    {
  printf("\n in bfd_elf32_bfd_reloc_type_lookup");
  unsigned int i;

  for (i = 0;
      i < sizeof (xgate_reloc_map) / sizeof (struct xgate_reloc_map);
      i++)
    {
    if (xgate_reloc_map[i].bfd_reloc_val == code){
      return &elf_xgate_howto_table[xgate_reloc_map[i].elf_reloc_val];
      printf("\n returning howto %s", elf_xgate_howto_table[i].name);
    }
    }
  return NULL;
    }

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
    const char *r_name)
    {
  printf("\n in bfd_elf32_bfd_reloc_name_lookup");
  unsigned int i;

  for (i = 0;
      i < (sizeof (elf_xgate_howto_table)
          / sizeof (elf_xgate_howto_table[0]));
      i++)
    if (elf_xgate_howto_table[i].name != NULL
        && strcasecmp (elf_xgate_howto_table[i].name, r_name) == 0){
      printf("\n returning howto %s", elf_xgate_howto_table[i].name);
      return &elf_xgate_howto_table[i];
    }

  return NULL;
    }

/* Set the howto pointer for an M68HC11/XGATE ELF reloc.  */
static void
xgate_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED,
    arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  printf("\n in xgate_info_to_howto_rel");
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_XGATE_max);
  cache_ptr->howto = &elf_xgate_howto_table[r_type];
  printf("\n in info_to_howto_rel howto is %s", cache_ptr->howto->name);
}

/* Create a 68HC11/68HC12 ELF linker hash table.  M68HC11_ELF_DATA */
struct xgate_elf_link_hash_table*
xgate_elf_hash_table_create (bfd *abfd)
{
  //printf("\n in elf_hash_table_create");
  struct xgate_elf_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct xgate_elf_link_hash_table);

  ret = (struct xgate_elf_link_hash_table *) bfd_malloc (amt);
  if (ret == (struct xgate_elf_link_hash_table *) NULL)
    return NULL;

  memset (ret, 0, amt);
  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
      _bfd_elf_link_hash_newfunc,
      sizeof (struct elf_link_hash_entry), XGATE_ELF_DATA))
    {
    free (ret);
    return NULL;
    }

  /* Init the stub hash table too.  */
  amt = sizeof (struct bfd_hash_table);
  ret->stub_hash_table = (struct bfd_hash_table*) bfd_malloc (amt);
  if (ret->stub_hash_table == NULL)
    {
    free (ret);
    return NULL;
    }
  if (!bfd_hash_table_init (ret->stub_hash_table, stub_hash_newfunc,
      sizeof (struct elf32_xgate_stub_hash_entry)))
    return NULL;

  ret->stub_bfd = NULL;
  ret->stub_section = 0;
  ret->add_stub_section = NULL;
  ret->sym_cache.abfd = NULL;

  return ret;
}

/* Free the derived linker hash table.  */

void
xgate_elf_bfd_link_hash_table_free (struct bfd_link_hash_table *hash)
{
  //printf("\n in elf_bfd_link_hash_table_free");
  struct xgate_elf_link_hash_table *ret
  = (struct xgate_elf_link_hash_table *) hash;

  bfd_hash_table_free (ret->stub_hash_table);
  free (ret->stub_hash_table);
  _bfd_generic_link_hash_table_free (hash);
}

/* Assorted hash table functions.  */

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry, struct bfd_hash_table *table,
    const char *string)
    {
  //printf("\n in stub_has_newfunc");
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
    entry = bfd_hash_allocate (table,
        sizeof (struct elf32_xgate_stub_hash_entry));
    if (entry == NULL)
      return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
    struct elf32_xgate_stub_hash_entry *eh;

    /* Initialize the local fields.  */
    eh = (struct elf32_xgate_stub_hash_entry *) entry;
    eh->stub_sec = NULL;
    eh->stub_offset = 0;
    eh->target_value = 0;
    eh->target_section = NULL;
    }

  return entry;
    }

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */
bfd_reloc_status_type
xgate_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
    arelent *reloc_entry,
    asymbol *symbol ATTRIBUTE_UNUSED,
    void *data ATTRIBUTE_UNUSED,
    asection *input_section,
    bfd *output_bfd,
    char **error_message ATTRIBUTE_UNUSED)
{
  //printf("\n in elf ignore reloc");
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

bfd_boolean
elf32_xgate_check_relocs (bfd *abfd, struct bfd_link_info *info,
    asection *sec, const Elf_Internal_Rela *relocs)
{

  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *     rel;
  const Elf_Internal_Rela *     rel_end;

  if (info->relocatable){
    return TRUE;
  }

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  rel_end = relocs + sec->reloc_count;

  for (rel = relocs; rel < rel_end; rel++)
    {
    struct elf_link_hash_entry * h;
    unsigned long r_symndx;

    r_symndx = ELF32_R_SYM (rel->r_info);

    if (r_symndx < symtab_hdr->sh_info)
      h = NULL;
    else
      {
      h = sym_hashes [r_symndx - symtab_hdr->sh_info];
      while (h->root.type == bfd_link_hash_indirect
          || h->root.type == bfd_link_hash_warning)
        h = (struct elf_link_hash_entry *) h->root.u.i.link;
      }

    switch (ELF32_R_TYPE (rel->r_info))
    {
    /* This relocation describes the C++ object vtable hierarchy.
           Reconstruct it for later use during GC.  */
    case R_XGATE_GNU_VTINHERIT:
      if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
        return FALSE;
      break;

      /* This relocation describes which C++ vtable entries are actually
           used.  Record for later use during GC.  */
    case R_XGATE_GNU_VTENTRY:
      BFD_ASSERT (h != NULL);
      if (h != NULL
          && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
        return FALSE;
      break;
    }
    }

  return TRUE;
}

/* Set and control ELF flags in ELF header.  */

bfd_boolean
_bfd_xgate_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return TRUE;
}

bfd_boolean
_bfd_xgate_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & E_XGATE_I32)
    fprintf (file, _("[abi=32-bit int, "));
  else
    fprintf (file, _("[abi=16-bit int, "));

  if (elf_elfheader (abfd)->e_flags & E_XGATE_F64)
    fprintf (file, _("64-bit double, "));
  else
    fprintf (file, _("32-bit double, "));
  if (strcmp (bfd_get_target (abfd), "elf32-xgate") == 0)
    fprintf (file, _("cpu=XGATE]"));
  if (elf_elfheader (abfd)->e_flags & E_XGATE_GLOBAL)
    fprintf (file, _(" [memory=global-model]"));
  else
    fprintf (file, _(" [memory=flat]"));

  fputc ('\n', file);

  return TRUE;
}


#define ELF_ARCH                bfd_arch_xgate
#define ELF_MACHINE_CODE        EM_XGATE

#define ELF_MAXPAGESIZE         0x1000

#define TARGET_BIG_SYM          bfd_elf32_xgate_vec
#define TARGET_BIG_NAME         "elf32-xgate"

#define ELF_TARGET_ID           XGATE_ELF_DATA

#define elf_info_to_howto       0

#define elf_info_to_howto_rel        xgate_info_to_howto_rel

#define elf_backend_can_gc_sections             1

#define bfd_elf32_bfd_link_hash_table_free \
    xgate_elf_bfd_link_hash_table_free

#define bfd_elf32_bfd_print_private_bfd_data \
    _bfd_xgate_elf_print_private_bfd_data

#include "elf32-target.h"
