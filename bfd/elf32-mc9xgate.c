/* Motorola 68HC12-specific support for 32-bit ELF
   Copyright 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
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
   MA 02110-1301, USA.  */
/* scratch
 *
 * EM_MC9XGATE
 * EM_MC9S12X
 * mc9s12xgate-dis.lo
 * mc9s12x-dis.lo
 * elf_mc9xgate_reloc_type
 * elf_mc9s12x_reloc_type
 * This target is no longer supported in gas
 *
 */
#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf32-mc9xgate.h"
#include "elf/mc9xgate.h"
#include "opcode/mc9xgate.h"

/* Relocation functions.  */
static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
static void mc9xgate_info_to_howto_rel
  (bfd *, arelent *, Elf_Internal_Rela *);

/* Trampoline generation.  */
static bfd_boolean mc9xgate_elf_size_one_stub
  (struct bfd_hash_entry *gen_entry, void *in_arg);
static bfd_boolean mc9xgate_elf_build_one_stub
  (struct bfd_hash_entry *gen_entry, void *in_arg);
static struct bfd_link_hash_table* mc9xgate_elf_bfd_link_hash_table_create
  (bfd*);

static bfd_boolean mc9xgate_elf_set_mach_from_flags PARAMS ((bfd *));

/* Use REL instead of RELA to save space */
#define USE_REL	1

/* The 68HC12 microcontroler has a memory bank switching system
   with a 16Kb window in the 64Kb address space.  The extended memory
   is mapped in the 16Kb window (at 0x8000).  The page register controls
   which 16Kb bank is mapped.  The call/rtc instructions take care of
   bank switching in function calls/returns.

   For GNU Binutils to work, we consider there is a physical memory
   at 0..0x0ffff and a kind of virtual memory above that.  Symbols
   in virtual memory have their addresses treated in a special way
   when disassembling and when linking.

   For the linker to work properly, we must always relocate the virtual
   memory as if it is mapped at 0x8000.  When a 16-bit relocation is
   made in the virtual memory, we check that it does not cross the
   memory bank where it is used.  This would involve a page change
   which would be wrong.  The 24-bit relocation is for that and it
   treats the address as a physical address + page number.


					Banked
					Address Space
                                        |               |       Page n
					+---------------+ 0x1010000
                                        |               |
                                        | jsr _foo      |
                                        | ..            |       Page 3
                                        | _foo:         |
					+---------------+ 0x100C000
					|	        |
                                        | call _bar     |
					| ..	        |	Page 2
					| _bar:	        |
					+---------------+ 0x1008000
				/------>|	        |
				|	| call _foo     |	Page 1
				|	|       	|
				|	+---------------+ 0x1004000
      Physical			|	|	        |
      Address Space		|	|	        |	Page 0
				|	|	        |
    +-----------+ 0x00FFFF	|	+---------------+ 0x1000000
    |		|		|
    | call _foo	|		|
    |		|		|
    +-----------+ 0x00BFFF -+---/
    |		|           |
    |		|	    |
    |		| 16K	    |
    |		|	    |
    +-----------+ 0x008000 -+
    |		|
    |		|
    =		=
    |		|
    |		|
    +-----------+ 0000


   The 'call _foo' must be relocated with page 3 and 16-bit address
   mapped at 0x8000.

   The 3-bit and 16-bit PC rel relocation is only used by 68HC12.  */
static reloc_howto_type elf_mc9xgate_howto_table[] = {
  /* This reloc does nothing.  */
  HOWTO (R_MC9XGATE_NONE,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_NONE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute relocation */
  HOWTO (R_MC9XGATE_8,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_8",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute relocation (upper address) */
  HOWTO (R_MC9XGATE_HI8,		/* type */
	 8,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATEi_HI8",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute relocation (upper address) */
  HOWTO (R_MC9XGATE_LO8,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_LO8",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit PC-rel relocation */
  HOWTO (R_MC9XGATE_PCREL_8,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_PCREL_8",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 TRUE),                 /* pcrel_offset */

  /* A 16 bit absolute relocation */
  HOWTO (R_MC9XGATE_16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont /*bitfield */ ,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_16",	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 32 bit absolute relocation.  This one is never used for the
     code relocation.  It's used by gas for -gstabs generation.  */
  HOWTO (R_MC9XGATE_32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_32",	/* name */
	 FALSE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 3 bit absolute relocation */
  HOWTO (R_MC9XGATE_3B,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 3,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_4B",	/* name */
	 FALSE,			/* partial_inplace */
	 0x003,			/* src_mask */
	 0x003,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit PC-rel relocation */
  HOWTO (R_MC9XGATE_PCREL_16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MC9XGATE_PCREL_16",	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),                 /* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_MC9XGATE_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_MC9XGATE_GNU_VTINHERIT",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_MC9XGATE_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_MC9XGATE_GNU_VTENTRY",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 24 bit relocation */
  HOWTO (R_MC9XGATE_24,	        /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 24,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 mc9xgate_elf_special_reloc,	/* special_function */
	 "R_MC9XGATE_24",	/* name */
	 FALSE,			/* partial_inplace */
	 0xffffff,		/* src_mask */
	 0xffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16-bit low relocation */
  HOWTO (R_MC9XGATE_LO16,        /* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 mc9xgate_elf_special_reloc,/* special_function */
	 "R_MC9XGATE_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A page relocation */
  HOWTO (R_MC9XGATE_PAGE,        /* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 mc9xgate_elf_special_reloc,/* special_function */
	 "R_MC9XGATE_PAGE",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),
  EMPTY_HOWTO (16),
  EMPTY_HOWTO (17),
  EMPTY_HOWTO (18),
  EMPTY_HOWTO (19),

  /* Mark beginning of a jump instruction (any form).  */
  HOWTO (R_MC9XGATE_RL_JUMP,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 mc9xgate_elf_ignore_reloc,	/* special_function */
	 "R_MC9XGATE_RL_JUMP",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 TRUE),                 /* pcrel_offset */

  /* Mark beginning of Gcc relaxation group instruction.  */
  HOWTO (R_MC9XGATE_RL_GROUP,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 mc9xgate_elf_ignore_reloc,	/* special_function */
	 "R_MC9XGATE_RL_GROUP",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 TRUE),                 /* pcrel_offset */
};

/* Map BFD reloc types to M68HC11 ELF reloc types.  */

struct mc9xgate_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct mc9xgate_reloc_map mc9xgate_reloc_map[] = {
  {BFD_RELOC_NONE, R_MC9XGATE_NONE,},
  {BFD_RELOC_8, R_MC9XGATE_8},
  {BFD_RELOC_MC9XGATE_HI8, R_MC9XGATE_HI8},
  {BFD_RELOC_MC9XGATE_LO8, R_MC9XGATE_LO8},
  {BFD_RELOC_8_PCREL, R_MC9XGATE_PCREL_8},
  {BFD_RELOC_16_PCREL, R_MC9XGATE_PCREL_16},
  {BFD_RELOC_16, R_MC9XGATE_16},
  {BFD_RELOC_32, R_MC9XGATE_32},
  {BFD_RELOC_MC9XGATE_3B, R_MC9XGATE_3B},

  {BFD_RELOC_VTABLE_INHERIT, R_MC9XGATE_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY, R_MC9XGATE_GNU_VTENTRY},

  {BFD_RELOC_MC9XGATE_LO16, R_MC9XGATE_LO16},
  {BFD_RELOC_MC9XGATE_PAGE, R_MC9XGATE_PAGE},
  {BFD_RELOC_MC9XGATE_24, R_MC9XGATE_24},

  {BFD_RELOC_MC9XGATE_RL_JUMP, R_MC9XGATE_RL_JUMP},
  {BFD_RELOC_MC9XGATE_RL_GROUP, R_MC9XGATE_RL_GROUP},
};

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                 bfd_reloc_code_real_type code)
{
  //printf("\n in bfd_elf32_bfd_reloc_type_lookup--line 398\n");
  unsigned int i;

  for (i = 0;
       i < sizeof (mc9xgate_reloc_map) / sizeof (struct mc9xgate_reloc_map);
       i++)
    {
      if (mc9xgate_reloc_map[i].bfd_reloc_val == code)
	return &elf_mc9xgate_howto_table[mc9xgate_reloc_map[i].elf_reloc_val];
    }

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  //printf("\n in bfd_elf32_bfd_reloc_name_lookup--line 416\n");
  unsigned int i;

  for (i = 0;
       i < (sizeof (elf_mc9xgate_howto_table)
	    / sizeof (elf_mc9xgate_howto_table[0]));
       i++)
    if (elf_mc9xgate_howto_table[i].name != NULL
	&& strcasecmp (elf_mc9xgate_howto_table[i].name, r_name) == 0)
      return &elf_mc9xgate_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an M68HC11 ELF reloc.  */

static void
mc9xgate_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED,
                           arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_MC9XGATE_max);
  cache_ptr->howto = &elf_mc9xgate_howto_table[r_type];
}


/* Far trampoline generation.  */

/* Build a 68HC12 trampoline stub.  */
static bfd_boolean
mc9xgate_elf_build_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct elf32_mc9xgate_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;
  struct mc9xgate_elf_link_hash_table *htab;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_byte *loc;
  bfd_vma sym_value, phys_page, phys_addr;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_mc9xgate_stub_hash_entry *) gen_entry;
  info = (struct bfd_link_info *) in_arg;

  htab = mc9xgate_elf_hash_table (info);

  stub_sec = stub_entry->stub_sec;

  /* Make a note of the offset within the stubs for this entry.  */
  stub_entry->stub_offset = stub_sec->size;
  stub_sec->size += 7;
  loc = stub_sec->contents + stub_entry->stub_offset;

  stub_bfd = stub_sec->owner;

  /* Create the trampoline call stub:

     ldy #%addr(symbol)
     call %page(symbol), __trampoline

  */
  sym_value = (stub_entry->target_value
               + stub_entry->target_section->output_offset
               + stub_entry->target_section->output_section->vma);
  phys_addr = mc9xgate_phys_addr (&htab->pinfo, sym_value);
  phys_page = mc9xgate_phys_page (&htab->pinfo, sym_value);

  /* ldy #%page(sym) */
  bfd_put_8 (stub_bfd, 0xCD, loc);
  bfd_put_16 (stub_bfd, phys_addr, loc + 1);
  loc += 3;

  /* call %page(sym), __trampoline  */
  bfd_put_8 (stub_bfd, 0x4a, loc);
  bfd_put_16 (stub_bfd, htab->pinfo.trampoline_addr, loc + 1);
  bfd_put_8 (stub_bfd, phys_page, loc + 3);

  return TRUE;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bfd_boolean
mc9xgate_elf_size_one_stub (struct bfd_hash_entry *gen_entry,
                           void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf32_mc9xgate_stub_hash_entry *stub_entry;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_mc9xgate_stub_hash_entry *) gen_entry;

  stub_entry->stub_sec->size += 7;
  return TRUE;
}

/* Create a 68HC12 ELF linker hash table.  */

static struct bfd_link_hash_table *
mc9xgate_elf_bfd_link_hash_table_create (bfd *abfd)
{
  struct mc9xgate_elf_link_hash_table *ret;

  ret = mc9xgate_elf_hash_table_create (abfd);
  if (ret == (struct mc9xgate_elf_link_hash_table *) NULL)
    return NULL;

  ret->size_one_stub = mc9xgate_elf_size_one_stub;
  ret->build_one_stub = mc9xgate_elf_build_one_stub;

  return &ret->root.root;
}

static bfd_boolean
mc9xgate_elf_set_mach_from_flags (bfd *abfd)
{
  flagword flags = elf_elfheader (abfd)->e_flags;

  switch (flags & EF_MC9XGATE_MACH_MASK)
    {
    case EF_MC9XGATE_MACH:
      bfd_default_set_arch_mach (abfd, bfd_arch_mc9xgate, bfd_mach_mc9xgate);
      /* TODO bfd_mach_mc9s12x does not work as a parameter(0 instead) re-make headers maybe....
       * UPDATE SEEMS TO WORK NOW */


   // 	 bfd_default_set_arch_mach (abfd, bfd_arch_m68hc12, bfd_mach_m6812s);  /* for testing */
      break;
 //   case EF_MC9XGATE_MACH:
 //     bfd_default_set_arch_mach (abfd, bfd_arch_mc9s12x, bfd_mach_mc9s12x);
 //     break;
 //   case EF_MC9XGATE_GENERIC:
 //    bfd_default_set_arch_mach (abfd, bfd_arch_mc9s12x,
 //                                bfd_mach_mc9s12x_default);
 //     break;
    default:
      return FALSE;
    }
  return TRUE;
}

/* Specific sections:
   - The .page0 is a data section that is mapped in [0x0000..0x00FF].
     Page0 accesses are faster on the M68HC12.
   - The .vectors is the section that represents the interrupt
     vectors.  */
static const struct bfd_elf_special_section elf32_mc9xgate_special_sections[] =
{
  { STRING_COMMA_LEN (".eeprom"),   0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".page0"),    0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".softregs"), 0, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".vectors"),  0, SHT_PROGBITS, SHF_ALLOC },
  { NULL,                       0,  0, 0,            0 }
};

#define ELF_ARCH		bfd_arch_mc9xgate
#define ELF_MACHINE_CODE	EM_MC9XGATE

//#define ELF_MACHINE_CODE	EM_68HC12 /* testing value */
#define ELF_MAXPAGESIZE		0x1000

#define TARGET_BIG_SYM          bfd_elf32_mc9xgate_vec
#define TARGET_BIG_NAME		"elf32-mc9xgate"

#define elf_info_to_howto	0
#define elf_info_to_howto_rel	     mc9xgate_info_to_howto_rel  /* TODO figure out why this has to be here */
#define elf_backend_check_relocs     elf32_mc9xgate_check_relocs
#define elf_backend_relocate_section elf32_mc9xgate_relocate_section
#define elf_backend_object_p		mc9xgate_elf_set_mach_from_flags
#define elf_backend_final_write_processing	0
#define elf_backend_can_gc_sections		1
#define elf_backend_special_sections elf32_mc9xgate_special_sections
#define elf_backend_post_process_headers     elf32_mc9xgate_post_process_headers
#define elf_backend_add_symbol_hook  elf32_mc9xgate_add_symbol_hook

#define bfd_elf32_bfd_link_hash_table_create \
		mc9xgate_elf_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_free \
		mc9xgate_elf_bfd_link_hash_table_free
#define bfd_elf32_bfd_merge_private_bfd_data \
					_bfd_mc9xgate_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags	_bfd_mc9xgate_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
					_bfd_mc9xgate_elf_print_private_bfd_data

#include "elf32-target.h"
/* TODO
 * I dont think you can.
 *  make sure commenting out the above is OK  the elf32-target file ends up in the build directory*/

/* ############################################################################################################################################
 * included from hc1x.c ??????

 Motorola 68HC11/HC12-specific support for 32-bit ELF
   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009 Free Software Foundation, Inc.
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

#define mc9xgate_stub_hash_lookup(table, string, create, copy) \
  ((struct elf32_mc9xgate_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

static struct elf32_mc9xgate_stub_hash_entry* mc9xgate_add_stub
  (const char *stub_name,
   asection *section,
   struct mc9xgate_elf_link_hash_table *htab);

static struct bfd_hash_entry *stub_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

static void mc9xgate_elf_set_symbol (bfd* abfd, struct bfd_link_info *info,
                                    const char* name, bfd_vma value,
                                    asection* sec);

static bfd_boolean mc9xgate_elf_export_one_stub
  (struct bfd_hash_entry *gen_entry, void *in_arg);

static void scan_sections_for_abi (bfd*, asection*, PTR);

struct mc9xgate_scan_param
{
   struct mc9xgate_page_info* pinfo;
   bfd_boolean use_memory_banks;
};


/* Create a 68HC11/68HC12 ELF linker hash table.  */

struct mc9xgate_elf_link_hash_table*
mc9xgate_elf_hash_table_create (bfd *abfd)
{
  struct mc9xgate_elf_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct mc9xgate_elf_link_hash_table);

  ret = (struct mc9xgate_elf_link_hash_table *) bfd_malloc (amt);
  if (ret == (struct mc9xgate_elf_link_hash_table *) NULL)
    return NULL;

  memset (ret, 0, amt);
  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      _bfd_elf_link_hash_newfunc,
				      sizeof (struct elf_link_hash_entry)))
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
			    sizeof (struct elf32_mc9xgate_stub_hash_entry)))
    return NULL;

  ret->stub_bfd = NULL;
  ret->stub_section = 0;
  ret->add_stub_section = NULL;
  ret->sym_cache.abfd = NULL;

  return ret;
}

/* Free the derived linker hash table.  */

void
mc9xgate_elf_bfd_link_hash_table_free (struct bfd_link_hash_table *hash)
{
  struct mc9xgate_elf_link_hash_table *ret
    = (struct mc9xgate_elf_link_hash_table *) hash;

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
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_mc9xgate_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_mc9xgate_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf32_mc9xgate_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
    }

  return entry;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf32_mc9xgate_stub_hash_entry *
mc9xgate_add_stub (const char *stub_name, asection *section,
                  struct mc9xgate_elf_link_hash_table *htab)
{
  struct elf32_mc9xgate_stub_hash_entry *stub_entry;

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = mc9xgate_stub_hash_lookup (htab->stub_hash_table, stub_name,
                                         TRUE, FALSE);
  if (stub_entry == NULL)
    {
      (*_bfd_error_handler) (_("%B: cannot create stub entry %s"),
			     section->owner, stub_name);
      return NULL;
    }

  if (htab->stub_section == 0)
    {
      htab->stub_section = (*htab->add_stub_section) (".tramp",
                                                      htab->tramp_section);
    }

  stub_entry->stub_sec = htab->stub_section;
  stub_entry->stub_offset = 0;
  return stub_entry;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it for identify far symbols and force a loading of
   the trampoline handler.  */

bfd_boolean
elf32_mc9xgate_add_symbol_hook (bfd *abfd, struct bfd_link_info *info,
                               Elf_Internal_Sym *sym,
                               const char **namep ATTRIBUTE_UNUSED,
                               flagword *flagsp ATTRIBUTE_UNUSED,
                               asection **secp ATTRIBUTE_UNUSED,
                               bfd_vma *valp ATTRIBUTE_UNUSED)
{
  if (sym->st_other & STO_MC9XGATE_FAR)
    {
      struct elf_link_hash_entry *h;

      h = (struct elf_link_hash_entry *)
	bfd_link_hash_lookup (info->hash, "__far_trampoline",
                              FALSE, FALSE, FALSE);
      if (h == NULL)
        {
          struct bfd_link_hash_entry* entry = NULL;

          _bfd_generic_link_add_one_symbol (info, abfd,
                                            "__far_trampoline",
                                            BSF_GLOBAL,
                                            bfd_und_section_ptr,
                                            (bfd_vma) 0, (const char*) NULL,
                                            FALSE, FALSE, &entry);
        }

    }
  return TRUE;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_mc9xgate_setup_section_lists (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  bfd_size_type amt;
  asection *text_section;
  struct mc9xgate_elf_link_hash_table *htab;

  htab = mc9xgate_elf_hash_table (info);

  if (bfd_get_flavour (info->output_bfd) != bfd_target_elf_flavour)
    return 0;

  /* Count the number of input BFDs and find the top input section id.
     Also search for an existing ".tramp" section so that we know
     where generated trampolines must go.  Default to ".text" if we
     can't find it.  */
  htab->tramp_section = 0;
  text_section = 0;
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link_next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	{
          const char* name = bfd_get_section_name (input_bfd, section);

          if (!strcmp (name, ".tramp"))
            htab->tramp_section = section;

          if (!strcmp (name, ".text"))
            text_section = section;

	  if (top_id < section->id)
	    top_id = section->id;
	}
    }
  htab->bfd_count = bfd_count;
  if (htab->tramp_section == 0)
    htab->tramp_section = text_section;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     strip_excluded_output_sections doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = (asection **) bfd_malloc (amt);
  htab->input_list = input_list;
  if (input_list == NULL)
    return -1;

  /* For sections we aren't interested in, mark their entries with a
     value we can check later.  */
  list = input_list + top_index;
  do
    *list = bfd_abs_section_ptr;
  while (list-- != input_list);

  for (section = output_bfd->sections;
       section != NULL;
       section = section->next)
    {
      if ((section->flags & SEC_CODE) != 0)
	input_list[section->index] = NULL;
    }

  return 1;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bfd_boolean
elf32_mc9xgate_size_stubs (bfd *output_bfd, bfd *stub_bfd,
                          struct bfd_link_info *info,
                          asection * (*add_stub_section) (const char*, asection*))
{
  bfd *input_bfd;
  asection *section;
  Elf_Internal_Sym *local_syms, **all_local_syms;
  unsigned int bfd_indx, bfd_count;
  bfd_size_type amt;
  asection *stub_sec;

  struct mc9xgate_elf_link_hash_table *htab = mc9xgate_elf_hash_table (info);

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link_next)
    {
      bfd_count += 1;
    }

  /* We want to read in symbol extension records only once.  To do this
     we need to read in the local symbols in parallel and save them for
     later use; so hold pointers to the local symbols in an array.  */
  amt = sizeof (Elf_Internal_Sym *) * bfd_count;
  all_local_syms = (Elf_Internal_Sym **) bfd_zmalloc (amt);
  if (all_local_syms == NULL)
    return FALSE;

  /* Walk over all the input BFDs, swapping in local symbols.  */
  for (input_bfd = info->input_bfds, bfd_indx = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link_next, bfd_indx++)
    {
      Elf_Internal_Shdr *symtab_hdr;

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
	continue;

      /* We need an array of the local symbols attached to the input bfd.  */
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL)
	{
	  local_syms = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					     symtab_hdr->sh_info, 0,
					     NULL, NULL, NULL);
	  /* Cache them for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) local_syms;
	}
      if (local_syms == NULL)
        {
          free (all_local_syms);
	  return FALSE;
        }

      all_local_syms[bfd_indx] = local_syms;
    }

  for (input_bfd = info->input_bfds, bfd_indx = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link_next, bfd_indx++)
    {
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;
      struct elf_link_hash_entry ** sym_hashes;

      sym_hashes = elf_sym_hashes (input_bfd);

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
        continue;

      local_syms = all_local_syms[bfd_indx];

      /* Walk over each section attached to the input bfd.  */
      for (section = input_bfd->sections;
           section != NULL;
           section = section->next)
        {
          Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

          /* If there aren't any relocs, then there's nothing more
             to do.  */
          if ((section->flags & SEC_RELOC) == 0
              || section->reloc_count == 0)
            continue;

          /* If this section is a link-once section that will be
             discarded, then don't create any stubs.  */
          if (section->output_section == NULL
              || section->output_section->owner != output_bfd)
            continue;

          /* Get the relocs.  */
          internal_relocs
            = _bfd_elf_link_read_relocs (input_bfd, section, NULL,
					 (Elf_Internal_Rela *) NULL,
					 info->keep_memory);
          if (internal_relocs == NULL)
            goto error_ret_free_local;

          /* Now examine each relocation.  */
          irela = internal_relocs;
          irelaend = irela + section->reloc_count;
          for (; irela < irelaend; irela++)
            {
              unsigned int r_type, r_indx;
              struct elf32_mc9xgate_stub_hash_entry *stub_entry;
              asection *sym_sec;
              bfd_vma sym_value;
              struct elf_link_hash_entry *hash;
              const char *stub_name;
              Elf_Internal_Sym *sym;

              r_type = ELF32_R_TYPE (irela->r_info);

              /* Only look at 16-bit relocs.  */
              if (r_type != (unsigned int) R_MC9XGATE_16)
                continue;

              /* Now determine the call target, its name, value,
                 section.  */
              r_indx = ELF32_R_SYM (irela->r_info);
              if (r_indx < symtab_hdr->sh_info)
                {
                  /* It's a local symbol.  */
                  Elf_Internal_Shdr *hdr;
                  bfd_boolean is_far;

                  sym = local_syms + r_indx;
                  is_far = (sym && (sym->st_other & STO_MC9XGATE_FAR));
                  if (!is_far)
                    continue;

		  if (sym->st_shndx >= elf_numsections (input_bfd))
		    sym_sec = NULL;
		  else
		    {
		      hdr = elf_elfsections (input_bfd)[sym->st_shndx];
		      sym_sec = hdr->bfd_section;
		    }
                  stub_name = (bfd_elf_string_from_elf_section
                               (input_bfd, symtab_hdr->sh_link,
                                sym->st_name));
                  sym_value = sym->st_value;
                  hash = NULL;
                }
              else
                {
                  /* It's an external symbol.  */
                  int e_indx;

                  e_indx = r_indx - symtab_hdr->sh_info;
                  hash = (struct elf_link_hash_entry *)
                    (sym_hashes[e_indx]);

                  while (hash->root.type == bfd_link_hash_indirect
                         || hash->root.type == bfd_link_hash_warning)
                    hash = ((struct elf_link_hash_entry *)
                            hash->root.u.i.link);

                  if (hash->root.type == bfd_link_hash_defined
                      || hash->root.type == bfd_link_hash_defweak
                      || hash->root.type == bfd_link_hash_new)
                    {
                      if (!(hash->other & STO_MC9XGATE_FAR))
                        continue;
                    }
                  else if (hash->root.type == bfd_link_hash_undefweak)
                    {
                      continue;
                    }
                  else if (hash->root.type == bfd_link_hash_undefined)
                    {
                      continue;
                    }
                  else
                    {
                      bfd_set_error (bfd_error_bad_value);
                      goto error_ret_free_internal;
                    }
                  sym_sec = hash->root.u.def.section;
                  sym_value = hash->root.u.def.value;
                  stub_name = hash->root.root.string;
                }

              if (!stub_name)
                goto error_ret_free_internal;

              stub_entry = mc9xgate_stub_hash_lookup
                (htab->stub_hash_table,
                 stub_name,
                 FALSE, FALSE);
              if (stub_entry == NULL)
                {
                  if (add_stub_section == 0)
                    continue;

                  stub_entry = mc9xgate_add_stub (stub_name, section, htab);
                  if (stub_entry == NULL)
                    {
                    error_ret_free_internal:
                      if (elf_section_data (section)->relocs == NULL)
                        free (internal_relocs);
                      goto error_ret_free_local;
                    }
                }

              stub_entry->target_value = sym_value;
              stub_entry->target_section = sym_sec;
            }

          /* We're done with the internal relocs, free them.  */
          if (elf_section_data (section)->relocs == NULL)
            free (internal_relocs);
        }
    }

  if (add_stub_section)
    {
      /* OK, we've added some stubs.  Find out the new size of the
         stub sections.  */
      for (stub_sec = htab->stub_bfd->sections;
           stub_sec != NULL;
           stub_sec = stub_sec->next)
        {
          stub_sec->size = 0;
        }

      bfd_hash_traverse (htab->stub_hash_table, htab->size_one_stub, htab);
    }
  free (all_local_syms);
  return TRUE;

 error_ret_free_local:
  free (all_local_syms);
  return FALSE;
}

/* Export the trampoline addresses in the symbol table.  */
static bfd_boolean
mc9xgate_elf_export_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct bfd_link_info *info;
  struct mc9xgate_elf_link_hash_table *htab;
  struct elf32_mc9xgate_stub_hash_entry *stub_entry;
  char* name;
  bfd_boolean result;

  info = (struct bfd_link_info *) in_arg;
  htab = mc9xgate_elf_hash_table (info);

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_mc9xgate_stub_hash_entry *) gen_entry;

  /* Generate the trampoline according to HC11 or HC12.  */
  result = (* htab->build_one_stub) (gen_entry, in_arg);

  /* Make a printable name that does not conflict with the real function.  */
  name = alloca (strlen (stub_entry->root.string) + 16);
  sprintf (name, "tramp.%s", stub_entry->root.string);

  /* Export the symbol for debugging/disassembling.  */
  mc9xgate_elf_set_symbol (htab->stub_bfd, info, name,
                          stub_entry->stub_offset,
                          stub_entry->stub_sec);
  return result;
}

/* Export a symbol or set its value and section.  */
static void
mc9xgate_elf_set_symbol (bfd *abfd, struct bfd_link_info *info,
                        const char *name, bfd_vma value, asection *sec)
{
  struct elf_link_hash_entry *h;

  h = (struct elf_link_hash_entry *)
    bfd_link_hash_lookup (info->hash, name, FALSE, FALSE, FALSE);
  if (h == NULL)
    {
      _bfd_generic_link_add_one_symbol (info, abfd,
                                        name,
                                        BSF_GLOBAL,
                                        sec,
                                        value,
                                        (const char*) NULL,
                                        TRUE, FALSE, NULL);
    }
  else
    {
      h->root.type = bfd_link_hash_defined;
      h->root.u.def.value = value;
      h->root.u.def.section = sec;
    }
}


/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  This function is called via m68hc12elf_finish in the
   linker.  */

bfd_boolean
elf32_mc9xgate_build_stubs (bfd *abfd, struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct mc9xgate_elf_link_hash_table *htab;
  struct mc9xgate_scan_param param;

  mc9xgate_elf_get_bank_parameters (info);
  htab = mc9xgate_elf_hash_table (info);

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Allocate memory to hold the linker stubs.  */
      size = stub_sec->size;
      stub_sec->contents = (unsigned char *) bfd_zalloc (htab->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return FALSE;
      stub_sec->size = 0;
    }

  /* Build the stubs as directed by the stub hash table.  */
  table = htab->stub_hash_table;
  bfd_hash_traverse (table, mc9xgate_elf_export_one_stub, info);

  /* Scan the output sections to see if we use the memory banks.
     If so, export the symbols that define how the memory banks
     are mapped.  This is used by gdb and the simulator to obtain
     the information.  It can be used by programs to burn the eprom
     at the good addresses.  */
  param.use_memory_banks = FALSE;
  param.pinfo = &htab->pinfo;
  bfd_map_over_sections (abfd, scan_sections_for_abi, &param);
  if (param.use_memory_banks)
    {
      mc9xgate_elf_set_symbol (abfd, info, BFD_MC9XGATE_BANK_START_NAME,
                              htab->pinfo.bank_physical,
                              bfd_abs_section_ptr);
      mc9xgate_elf_set_symbol (abfd, info, BFD_MC9XGATE_BANK_VIRTUAL_NAME,
                              htab->pinfo.bank_virtual,
                              bfd_abs_section_ptr);
      mc9xgate_elf_set_symbol (abfd, info, BFD_MC9XGATE_BANK_SIZE_NAME,
                              htab->pinfo.bank_size,
                              bfd_abs_section_ptr);
    }

  return TRUE;
}

void
mc9xgate_elf_get_bank_parameters (struct bfd_link_info *info)
{
  unsigned i;
  struct mc9xgate_page_info *pinfo;
  struct bfd_link_hash_entry *h;

  pinfo = &mc9xgate_elf_hash_table (info)->pinfo;
  if (pinfo->bank_param_initialized)
    return;

  pinfo->bank_virtual = MC9XGATE_BANK_VIRT;
  pinfo->bank_mask = MC9XGATE_BANK_MASK;
  pinfo->bank_physical = MC9XGATE_BANK_BASE;
  pinfo->bank_shift = MC9XGATE_BANK_SHIFT;
  pinfo->bank_size = 1 << MC9XGATE_BANK_SHIFT;

  h = bfd_link_hash_lookup (info->hash, BFD_MC9XGATE_BANK_START_NAME,
                            FALSE, FALSE, TRUE);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_physical = (h->u.def.value
                            + h->u.def.section->output_section->vma
                            + h->u.def.section->output_offset);

  h = bfd_link_hash_lookup (info->hash, BFD_MC9XGATE_BANK_VIRTUAL_NAME,
                            FALSE, FALSE, TRUE);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_virtual = (h->u.def.value
                           + h->u.def.section->output_section->vma
                           + h->u.def.section->output_offset);

  h = bfd_link_hash_lookup (info->hash, BFD_MC9XGATE_BANK_SIZE_NAME,
                            FALSE, FALSE, TRUE);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_size = (h->u.def.value
                        + h->u.def.section->output_section->vma
                        + h->u.def.section->output_offset);

  pinfo->bank_shift = 0;
  for (i = pinfo->bank_size; i != 0; i >>= 1)
    pinfo->bank_shift++;
  pinfo->bank_shift--;
  pinfo->bank_mask = (1 << pinfo->bank_shift) - 1;
  pinfo->bank_physical_end = pinfo->bank_physical + pinfo->bank_size;
  pinfo->bank_param_initialized = 1;

  h = bfd_link_hash_lookup (info->hash, "__far_trampoline", FALSE,
                            FALSE, TRUE);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->trampoline_addr = (h->u.def.value
                              + h->u.def.section->output_section->vma
                              + h->u.def.section->output_offset);
}

/* Return 1 if the address is in banked memory.
   This can be applied to a virtual address and to a physical address.  */
int
mc9xgate_addr_is_banked (struct mc9xgate_page_info *pinfo, bfd_vma addr)
{
  if (addr >= pinfo->bank_virtual)
    return 1;

  if (addr >= pinfo->bank_physical && addr <= pinfo->bank_physical_end)
    return 1;

  return 0;
}

/* Return the physical address seen by the processor, taking
   into account banked memory.  */
bfd_vma
mc9xgate_phys_addr (struct mc9xgate_page_info *pinfo, bfd_vma addr)
{
  if (addr < pinfo->bank_virtual)
    return addr;

  /* Map the address to the memory bank.  */
  addr -= pinfo->bank_virtual;
  addr &= pinfo->bank_mask;
  addr += pinfo->bank_physical;
  return addr;
}

/* Return the page number corresponding to an address in banked memory.  */
bfd_vma
mc9xgate_phys_page (struct mc9xgate_page_info *pinfo, bfd_vma addr)
{
  if (addr < pinfo->bank_virtual)
    return 0;

  /* Map the address to the memory bank.  */
  addr -= pinfo->bank_virtual;
  addr >>= pinfo->bank_shift;
  addr &= 0x0ff;
  return addr;
}

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */

bfd_reloc_status_type
mc9xgate_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
                          arelent *reloc_entry,
                          asymbol *symbol ATTRIBUTE_UNUSED,
                          void *data ATTRIBUTE_UNUSED,
                          asection *input_section,
                          bfd *output_bfd,
                          char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

bfd_reloc_status_type
mc9xgate_elf_special_reloc (bfd *abfd ATTRIBUTE_UNUSED,
                           arelent *reloc_entry,
                           asymbol *symbol,
                           void *data ATTRIBUTE_UNUSED,
                           asection *input_section,
                           bfd *output_bfd,
                           char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    return bfd_reloc_continue;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  abort();
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

bfd_boolean
elf32_mc9xgate_check_relocs (bfd *abfd, struct bfd_link_info *info,
                            asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *     rel;
  const Elf_Internal_Rela *     rel_end;

  if (info->relocatable)
    return TRUE;

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
        case R_MC9XGATE_GNU_VTINHERIT:
          if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
            return FALSE;
          break;

        /* This relocation describes which C++ vtable entries are actually
           used.  Record for later use during GC.  */
        case R_MC9XGATE_GNU_VTENTRY:
          BFD_ASSERT (h != NULL);
          if (h != NULL
              && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
            return FALSE;
          break;
        }
    }

  return TRUE;
}

/* Relocate a 68hc11/68hc12 ELF section.  */
bfd_boolean
elf32_mc9xgate_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
                                struct bfd_link_info *info,
                                bfd *input_bfd, asection *input_section,
                                bfd_byte *contents, Elf_Internal_Rela *relocs,
                                Elf_Internal_Sym *local_syms,
                                asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  const char *name = NULL;
  struct mc9xgate_page_info *pinfo;
  const struct elf_backend_data * const ebd = get_elf_backend_data (input_bfd);

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  /* Get memory bank parameters.  */
  mc9xgate_elf_get_bank_parameters (info);
  pinfo = &mc9xgate_elf_hash_table (info)->pinfo;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      arelent arel;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma relocation = 0;
      bfd_reloc_status_type r = bfd_reloc_undefined;
      bfd_vma phys_page;
      bfd_vma phys_addr;
      bfd_vma insn_addr;
      bfd_vma insn_page;
      bfd_boolean is_far = FALSE;
      struct elf_link_hash_entry *h;
      const char* stub_name = 0;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type == R_MC9XGATE_GNU_VTENTRY
          || r_type == R_MC9XGATE_GNU_VTINHERIT )
        continue;

      #undef elf_info_to_howto_rel  /* TODO fix so this is not necessary */
      (*ebd->elf_info_to_howto_rel) (input_bfd, &arel, rel);
      howto = arel.howto;

      h = NULL;
      sym = NULL;
      sec = NULL;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);
	  is_far = (sym && (sym->st_other & STO_MC9XGATE_FAR));
	  if (is_far)
	    stub_name = (bfd_elf_string_from_elf_section
			 (input_bfd, symtab_hdr->sh_link,
			  sym->st_name));
	}
      else
	{
	  bfd_boolean unresolved_reloc, warned;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation, unresolved_reloc,
				   warned);

	  is_far = (h && (h->other & STO_MC9XGATE_FAR));
	  stub_name = h->root.root.string;
	}

      if (sec != NULL && elf_discarded_section (sec))
	{
	  /* For relocs against symbols from removed linkonce sections,
	     or sections discarded by a linker script, we just want the
	     section contents zeroed.  Avoid any special processing.  */
	  _bfd_clear_contents (howto, input_bfd, contents + rel->r_offset);
	  rel->r_info = 0;
	  rel->r_addend = 0;
	  continue;
	}

      if (info->relocatable)
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (sym != NULL && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    rel->r_addend += sec->output_offset;
	  continue;
	}

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (input_bfd, sec);
	}

      if (is_far && ELF32_R_TYPE (rel->r_info) == R_MC9XGATE_16)
	{
	  struct elf32_mc9xgate_stub_hash_entry* stub;
	  struct mc9xgate_elf_link_hash_table *htab;

	  htab = mc9xgate_elf_hash_table (info);
	  stub = mc9xgate_stub_hash_lookup (htab->stub_hash_table,
					   name, FALSE, FALSE);
	  if (stub)
	    {
	      relocation = stub->stub_offset
		+ stub->stub_sec->output_section->vma
		+ stub->stub_sec->output_offset;
	      is_far = FALSE;
	    }
	}

      /* Do the memory bank mapping.  */
      phys_addr = mc9xgate_phys_addr (pinfo, relocation + rel->r_addend);
      phys_page = mc9xgate_phys_page (pinfo, relocation + rel->r_addend);
      switch (r_type)
        {
        case R_MC9XGATE_24:
          /* Reloc used by 68HC12 call instruction.  */
          bfd_put_16 (input_bfd, phys_addr,
                      (bfd_byte*) contents + rel->r_offset);
          bfd_put_8 (input_bfd, phys_page,
                     (bfd_byte*) contents + rel->r_offset + 2);
          r = bfd_reloc_ok;
          r_type = R_MC9XGATE_NONE;
          break;

        case R_MC9XGATE_NONE:
          r = bfd_reloc_ok;
          break;

        case R_MC9XGATE_LO16:
          /* Reloc generated by %addr(expr) gas to obtain the
             address as mapped in the memory bank window.  */
          relocation = phys_addr;
          break;

        case R_MC9XGATE_PAGE:
          /* Reloc generated by %page(expr) gas to obtain the
             page number associated with the address.  */
          relocation = phys_page;
          break;

        case R_MC9XGATE_16:
          /* Get virtual address of instruction having the relocation.  */
          if (is_far)
            {
              const char* msg;
              char* buf;
              msg = _("Reference to the far symbol `%s' using a wrong "
                      "relocation may result in incorrect execution");
              buf = alloca (strlen (msg) + strlen (name) + 10);
              sprintf (buf, msg, name);

              (* info->callbacks->warning)
                (info, buf, name, input_bfd, NULL, rel->r_offset);
            }

          /* Get virtual address of instruction having the relocation.  */
          insn_addr = input_section->output_section->vma
            + input_section->output_offset
            + rel->r_offset;

          insn_page = mc9xgate_phys_page (pinfo, insn_addr);

          if (mc9xgate_addr_is_banked (pinfo, relocation + rel->r_addend)
              && mc9xgate_addr_is_banked (pinfo, insn_addr)
              && phys_page != insn_page)
            {
              const char* msg;
              char* buf;

              msg = _("banked address [%lx:%04lx] (%lx) is not in the same bank "
                      "as current banked address [%lx:%04lx] (%lx)");

              buf = alloca (strlen (msg) + 128);
              sprintf (buf, msg, phys_page, phys_addr,
                       (long) (relocation + rel->r_addend),
                       insn_page, mc9xgate_phys_addr (pinfo, insn_addr),
                       (long) (insn_addr));
              if (!((*info->callbacks->warning)
                    (info, buf, name, input_bfd, input_section,
                     rel->r_offset)))
                return FALSE;
              break;
            }
          if (phys_page != 0 && insn_page == 0)
            {
              const char* msg;
              char* buf;

              msg = _("reference to a banked address [%lx:%04lx] in the "
                      "normal address space at %04lx");

              buf = alloca (strlen (msg) + 128);
              sprintf (buf, msg, phys_page, phys_addr, insn_addr);
              if (!((*info->callbacks->warning)
                    (info, buf, name, input_bfd, input_section,
                     insn_addr)))
                return FALSE;

              relocation = phys_addr;
              break;
            }

          /* If this is a banked address use the phys_addr so that
             we stay in the banked window.  */
          if (mc9xgate_addr_is_banked (pinfo, relocation + rel->r_addend))
            relocation = phys_addr;
          break;
        }
      if (r_type != R_MC9XGATE_NONE)
        r = _bfd_final_link_relocate (howto, input_bfd, input_section,
                                      contents, rel->r_offset,
                                      relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) 0;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      if (!((*info->callbacks->reloc_overflow)
		    (info, NULL, name, howto->name, (bfd_vma) 0,
		     input_bfd, input_section, rel->r_offset)))
		return FALSE;
	      break;

	    case bfd_reloc_undefined:
	      if (!((*info->callbacks->undefined_symbol)
		    (info, name, input_bfd, input_section,
		     rel->r_offset, TRUE)))
		return FALSE;
	      break;

	    case bfd_reloc_outofrange:
	      msg = _ ("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      msg = _ ("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      msg = _ ("internal error: dangerous error");
	      goto common_error;

	    default:
	      msg = _ ("internal error: unknown error");
	      /* fall through */

	    common_error:
	      if (!((*info->callbacks->warning)
		    (info, msg, name, input_bfd, input_section,
		     rel->r_offset)))
		return FALSE;
	      break;
	    }
	}
    }

  return TRUE;
}



/* Set and control ELF flags in ELF header.  */

bfd_boolean
_bfd_mc9xgate_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bfd_boolean
_bfd_mc9xgate_elf_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword old_flags;
  flagword new_flags;
  bfd_boolean ok = TRUE;

  /* Check if we have the same endianess */
  if (!_bfd_generic_verify_endian_match (ibfd, obfd))
    return FALSE;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  new_flags = elf_elfheader (ibfd)->e_flags;
  elf_elfheader (obfd)->e_flags |= new_flags & EF_MC9XGATE_ABI;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (! elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = new_flags;
      elf_elfheader (obfd)->e_ident[EI_CLASS]
	= elf_elfheader (ibfd)->e_ident[EI_CLASS];

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	{
	  if (! bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				   bfd_get_mach (ibfd)))
	    return FALSE;
	}

      return TRUE;
    }

  /* Check ABI compatibility.  */
  if ((new_flags & E_MC9XGATE_I32) != (old_flags & E_MC9XGATE_I32))
    {
      (*_bfd_error_handler)
	(_("%B: linking files compiled for 16-bit integers (-mshort) "
           "and others for 32-bit integers"), ibfd);
      ok = FALSE;
    }
  if ((new_flags & E_MC9XGATE_F64) != (old_flags & E_MC9XGATE_F64))
    {
      (*_bfd_error_handler)
	(_("%B: linking files compiled for 32-bit double (-fshort-double) "
           "and others for 64-bit double"), ibfd);
      ok = FALSE;
    }
/*
 *
 *
   Check ABI compatibility.
  if ((new_flags & E_M68HC11_I32) != (old_flags & E_M68HC11_I32))
    {
      (*_bfd_error_handler)
	(_("%B: linking files compiled for 16-bit integers (-mshort) "
           "and others for 32-bit integers"), ibfd);
      ok = FALSE;
    }
  if ((new_flags & E_M68HC11_F64) != (old_flags & E_M68HC11_F64))
    {
      (*_bfd_error_handler)
	(_("%B: linking files compiled for 32-bit double (-fshort-double) "
           "and others for 64-bit double"), ibfd);
      ok = FALSE;
    }

  Processor compatibility.
  if (!EF_M68HC11_CAN_MERGE_MACH (new_flags, old_flags))
    {
      (*_bfd_error_handler)
	(_("%B: linking files compiled for HCS12 with "
           "others compiled for HC12"), ibfd);
      ok = FALSE;
    }
  new_flags = ((new_flags & ~EF_M68HC11_MACH_MASK)
               | (EF_M68HC11_MERGE_MACH (new_flags, old_flags)));

  elf_elfheader (obfd)->e_flags = new_flags;

  new_flags &= ~(EF_M68HC11_ABI | EF_M68HC11_MACH_MASK);
  old_flags &= ~(EF_M68HC11_ABI | EF_M68HC11_MACH_MASK);
 */


  /* Processor compatibility.  */
  /* TODO eliminate compat mode mabye have a mode MUST BE REFACTORED */
//  if (!EF_MC9XGATE_CAN_MERGE_MACH (new_flags, old_flags))
//    {
//      (*_bfd_error_handler)
//	(_("%B: linking files compiled for HCS12 with "
//           "others compiled for HC12"), ibfd);
      ok = FALSE;
//    }
//  new_flags = ((new_flags & ~EF_MC9XGATE_MACH_MASK)
//               | (EF_MC9XGATE_MERGE_MACH (new_flags, old_flags)));

  elf_elfheader (obfd)->e_flags = new_flags;

  new_flags &= ~(EF_MC9XGATE_ABI | EF_MC9XGATE_MACH_MASK);
  old_flags &= ~(EF_MC9XGATE_ABI | EF_MC9XGATE_MACH_MASK);

  /* Warn about any other mismatches */
  if (new_flags != old_flags)
    {
      (*_bfd_error_handler)
	(_("%B: uses different e_flags (0x%lx) fields than previous modules (0x%lx)"),
	 ibfd, (unsigned long) new_flags, (unsigned long) old_flags);
      ok = FALSE;
    }

  if (! ok)
    {
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  return TRUE;
}

bfd_boolean
_bfd_mc9xgate_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & E_MC9XGATE_I32)
    fprintf (file, _("[abi=32-bit int, "));
  else
    fprintf (file, _("[abi=16-bit int, "));

  if (elf_elfheader (abfd)->e_flags & E_MC9XGATE_F64)
    fprintf (file, _("64-bit double, "));
  else
    fprintf (file, _("32-bit double, "));
/* TODO reduce to 1 target */
  if (strcmp (bfd_get_target (abfd), "elf32-mc9s12X") == 0)
    fprintf (file, _("cpu=MC9XGATE]"));
  else if (elf_elfheader (abfd)->e_flags & EF_MC9XGATE_MACH)
    fprintf (file, _("cpu=MC9XGATE]"));
  else
    fprintf (file, _("cpu=MC9XGATE]"));

  if (elf_elfheader (abfd)->e_flags & E_MC9XGATE_BANKS)
    fprintf (file, _(" [memory=bank-model]"));
  else
    fprintf (file, _(" [memory=flat]"));

  fputc ('\n', file);

  return TRUE;
}

static void scan_sections_for_abi (bfd *abfd ATTRIBUTE_UNUSED,
                                   asection *asect, void *arg)
{
  struct mc9xgate_scan_param* p = (struct mc9xgate_scan_param*) arg;

  if (asect->vma >= p->pinfo->bank_virtual)
    p->use_memory_banks = TRUE;
}

/* Tweak the OSABI field of the elf header.  */

void
elf32_mc9xgate_post_process_headers (bfd *abfd, struct bfd_link_info *link_info)
{
  struct mc9xgate_scan_param param;

  if (link_info == 0)
    return;

  mc9xgate_elf_get_bank_parameters (link_info);

  param.use_memory_banks = FALSE;
  param.pinfo = &mc9xgate_elf_hash_table (link_info)->pinfo;
  bfd_map_over_sections (abfd, scan_sections_for_abi, &param);
  if (param.use_memory_banks)
    {
      Elf_Internal_Ehdr * i_ehdrp;

      i_ehdrp = elf_elfheader (abfd);
      i_ehdrp->e_flags |= E_MC9XGATE_BANKS;
    }
}

