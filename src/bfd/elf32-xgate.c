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
static reloc_howto_type *bfd_elf32_bfd_reloc_name_lookup
(bfd *abfd ATTRIBUTE_UNUSED, const char *r_name);
static void xgate_info_to_howto_rel
(bfd *, arelent *, Elf_Internal_Rela *);
static bfd_boolean xgate_elf_size_one_stub
(struct bfd_hash_entry *gen_entry, void *in_arg);
static bfd_boolean xgate_elf_build_one_stub
(struct bfd_hash_entry *gen_entry, void *in_arg);
static struct bfd_link_hash_table* xgate_elf_bfd_link_hash_table_create
(bfd*);
static bfd_boolean xgate_elf_set_mach_from_flags PARAMS ((bfd *));

static struct elf32_xgate_stub_hash_entry* xgate_add_stub
(const char *stub_name,
    asection *section,
    struct xgate_elf_link_hash_table *htab);

static struct bfd_hash_entry *stub_hash_newfunc
(struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

static void xgate_elf_set_symbol (bfd* abfd, struct bfd_link_info *info,
    const char* name, bfd_vma value,
    asection* sec);

static bfd_boolean xgate_elf_export_one_stub
(struct bfd_hash_entry *gen_entry, void *in_arg);

static void scan_sections_for_abi (bfd*, asection*, PTR);

/* Use REL instead of RELA to save space */
#define USE_REL	1

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
		xgate_elf_special_reloc, /* special_function */
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
		xgate_elf_special_reloc,/* special_function */
		"R_XGATE_LO16", /* name */
		FALSE, /* partial_inplace */
		0xffff, /* src_mask */
		0xffff, /* dst_mask */
		FALSE), /* pcrel_offset */

/* A page relocation */
HOWTO (R_XGATE_GPAGE, /* type */
		0, /* rightshift */
		0, /* size (0 = byte, 1 = short, 2 = long) */
		8, /* bitsize */
		FALSE, /* pc_relative */
		0, /* bitpos */
		complain_overflow_dont, /* complain_on_overflow */
		xgate_elf_special_reloc,/* special_function */
		"R_XGATE_GPAGE", /* name */
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


/* Map BFD reloc types to XGATE ELF reloc types.  */

struct xgate_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct xgate_reloc_map xgate_reloc_map[] = {
    {BFD_RELOC_NONE, R_XGATE_NONE,},
    {BFD_RELOC_8, R_XGATE_8},
    {BFD_RELOC_8_PCREL, R_XGATE_PCREL_8},
    {BFD_RELOC_16_PCREL, R_XGATE_PCREL_16},
    {BFD_RELOC_16, R_XGATE_16},
    {BFD_RELOC_32, R_XGATE_32},

    {BFD_RELOC_VTABLE_INHERIT, R_XGATE_GNU_VTINHERIT},
    {BFD_RELOC_VTABLE_ENTRY, R_XGATE_GNU_VTENTRY},

    {BFD_RELOC_XGATE_LO16, R_XGATE_LO16},
    {BFD_RELOC_XGATE_GPAGE, R_XGATE_GPAGE},
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
      printf("\n FOUND about to compare %d and %d", (int)code, (int) xgate_reloc_map[i].bfd_reloc_val);
      return &elf_xgate_howto_table[xgate_reloc_map[i].elf_reloc_val];
    }
    }
  printf("\n about to return null");
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
 // //printf("\n in info_to_howto_rel");
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_XGATE_max);
  cache_ptr->howto = &elf_xgate_howto_table[r_type];
 // //printf("\n in info_to_howto_rel howto is %s", cache_ptr->howto->name);
}


/* Far trampoline generation.  */

/* Build a 68HC12 trampoline stub.  */
static bfd_boolean
xgate_elf_build_one_stub (struct bfd_hash_entry *gen_entry ATTRIBUTE_UNUSED, void *in_arg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bfd_boolean
xgate_elf_size_one_stub (struct bfd_hash_entry *gen_entry,
    void *in_arg ATTRIBUTE_UNUSED)
{
  //printf("\n in elf_size_one_stub");
  struct elf32_xgate_stub_hash_entry *stub_entry;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_xgate_stub_hash_entry *) gen_entry;

  stub_entry->stub_sec->size += 7;
  return TRUE;
}

/* Create a 68HC12 ELF linker hash table.  */

static struct bfd_link_hash_table *
xgate_elf_bfd_link_hash_table_create (bfd *abfd ATTRIBUTE_UNUSED)
{
return NULL;
}

static bfd_boolean
xgate_elf_set_mach_from_flags (bfd *abfd ATTRIBUTE_UNUSED)
{

  return TRUE;
}

/* Specific sections:
   - The .page0 is a data section that is mapped in [0x0000..0x00FF].
     Page0 accesses are faster on the M68HC12.
   - The .vectors is the section that represents the interrupt
     vectors.
   - The .xgate section is starts in 0xE08800 or as xgate sees it 0x0800
 */
static const struct bfd_elf_special_section elf32_xgate_special_sections[] =
    {
        { STRING_COMMA_LEN (".eeprom"),   0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
        { STRING_COMMA_LEN (".page0"),    0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
        { STRING_COMMA_LEN (".softregs"), 0, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
        { STRING_COMMA_LEN (".vectors"),  0, SHT_PROGBITS, SHF_ALLOC },
        //  { STRING_COMMA_LEN (".xgate"),    0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
        { NULL,                       0,  0, 0,            0 }
    };


struct xgate_scan_param
{
  struct xgate_page_info* pinfo;
  bfd_boolean use_memory_banks;
};


/* Create a 68HC11/68HC12 ELF linker hash table.  M68HC11_ELF_DATA */

struct xgate_elf_link_hash_table*
xgate_elf_hash_table_create (bfd *abfd ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Free the derived linker hash table.  */

void
xgate_elf_bfd_link_hash_table_free (struct bfd_link_hash_table *hash ATTRIBUTE_UNUSED)
{

}

/* Assorted hash table functions.  */

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry, struct bfd_hash_table *table ATTRIBUTE_UNUSED,
    const char *string ATTRIBUTE_UNUSED)
    {
  return entry;
    }

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf32_xgate_stub_hash_entry *
xgate_add_stub (const char *stub_name ATTRIBUTE_UNUSED, asection *section ATTRIBUTE_UNUSED,
    struct xgate_elf_link_hash_table *htab ATTRIBUTE_UNUSED)
    {

    }

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it for identify far symbols and force a loading of
   the trampoline handler.  */

bfd_boolean
elf32_xgate_add_symbol_hook (bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED,
    Elf_Internal_Sym *sym ATTRIBUTE_UNUSED,
    const char **namep ATTRIBUTE_UNUSED,
    flagword *flagsp ATTRIBUTE_UNUSED,
    asection **secp ATTRIBUTE_UNUSED,
    bfd_vma *valp ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_xgate_setup_section_lists (bfd *output_bfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{

  return 1;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bfd_boolean
elf32_xgate_size_stubs (bfd *output_bfd, bfd *stub_bfd ATTRIBUTE_UNUSED,
    struct bfd_link_info *info ATTRIBUTE_UNUSED,
    asection * (*add_stub_section) (const char*, asection*) ATTRIBUTE_UNUSED)
{
  return FALSE;
}

/* Export the trampoline addresses in the symbol table.  */
static bfd_boolean
xgate_elf_export_one_stub (struct bfd_hash_entry *gen_entry ATTRIBUTE_UNUSED, void *in_arg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Export a symbol or set its value and section.  */
static void
xgate_elf_set_symbol (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED,
    const char *name ATTRIBUTE_UNUSED, bfd_vma value ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED)
{

}


/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  This function is called via m68hc12elf_finish in the
   linker.  */

bfd_boolean
elf32_xgate_build_stubs (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

void
xgate_elf_get_bank_parameters (struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
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

bfd_reloc_status_type
xgate_elf_special_reloc (bfd *abfd ATTRIBUTE_UNUSED,
    arelent *reloc_entry ATTRIBUTE_UNUSED,
    asymbol *symbol ATTRIBUTE_UNUSED,
    void *data ATTRIBUTE_UNUSED,
    asection *input_section ATTRIBUTE_UNUSED,
    bfd *output_bfd ATTRIBUTE_UNUSED,
    char **error_message ATTRIBUTE_UNUSED)
{

  abort();
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

bfd_boolean
elf32_xgate_check_relocs (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED,
    asection *sec ATTRIBUTE_UNUSED, const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Relocate a 68hc11/68hc12 ELF section.  */
bfd_boolean
elf32_xgate_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
    struct bfd_link_info *info ATTRIBUTE_UNUSED,
    bfd *input_bfd ATTRIBUTE_UNUSED, asection *input_section ATTRIBUTE_UNUSED,
    bfd_byte *contents ATTRIBUTE_UNUSED, Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED,
    Elf_Internal_Sym *local_syms ATTRIBUTE_UNUSED,
    asection **local_sections ATTRIBUTE_UNUSED)
{
  return TRUE;
}



/* Set and control ELF flags in ELF header.  */

bfd_boolean
_bfd_xgate_elf_set_private_flags (bfd *abfd ATTRIBUTE_UNUSED, flagword flags ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bfd_boolean
_bfd_xgate_elf_merge_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED, bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_xgate_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  //printf("\n in elf_print_private_bfd_data");
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
  /* TODO reduce to 1 target */
  if (strcmp (bfd_get_target (abfd), "elf32-mc9s12X") == 0)
    fprintf (file, _("cpu=XGATE]"));
  else if (elf_elfheader (abfd)->e_flags & EF_XGATE_MACH)
    fprintf (file, _("cpu=XGATE]"));
  else
    fprintf (file, _("cpu=XGATE]"));

//  if (elf_elfheader (abfd)->e_flags & E_XGATE_BANKS)
//    fprintf (file, _(" [memory=bank-model]"));
//  else
//    fprintf (file, _(" [memory=flat]"));

  fputc ('\n', file);

  return TRUE;
}

static void scan_sections_for_abi (bfd *abfd ATTRIBUTE_UNUSED,
    asection *asect ATTRIBUTE_UNUSED, void *arg ATTRIBUTE_UNUSED)
{

}

/* Tweak the OSABI field of the elf header.  */

void
elf32_xgate_post_process_headers (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{

}

#define ELF_ARCH                bfd_arch_xgate
//#define ELF_ARCH                bfd_arch_mc9s12x
#define ELF_MACHINE_CODE        EM_XGATE
#define ELF_TARGET_ID           XGATE_ELF_DATA

//#define ELF_MACHINE_CODE      EM_68HC12 /* testing value */
#define ELF_MAXPAGESIZE         0x1000

#define TARGET_BIG_SYM          bfd_elf32_xgate_vec
#define TARGET_BIG_NAME         "elf32-xgate"

#define elf_info_to_howto       0
#define elf_info_to_howto_rel        xgate_info_to_howto_rel
#define elf_backend_check_relocs     elf32_xgate_check_relocs
#define elf_backend_relocate_section elf32_xgate_relocate_section
#define elf_backend_object_p            xgate_elf_set_mach_from_flags
#define elf_backend_final_write_processing      0
#define elf_backend_can_gc_sections             1
#define elf_backend_special_sections elf32_xgate_special_sections
#define elf_backend_post_process_headers     elf32_xgate_post_process_headers
#define elf_backend_add_symbol_hook  elf32_xgate_add_symbol_hook

#define bfd_elf32_bfd_link_hash_table_create \
    xgate_elf_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_free \
    xgate_elf_bfd_link_hash_table_free
#define bfd_elf32_bfd_merge_private_bfd_data \
    _bfd_xgate_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags _bfd_xgate_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
    _bfd_xgate_elf_print_private_bfd_data
#define xgate_stub_hash_lookup(table, string, create, copy) \
    ((struct elf32_xgate_stub_hash_entry *) \
        bfd_hash_lookup ((table), (string), (create), (copy)))

#include "elf32-target.h"
