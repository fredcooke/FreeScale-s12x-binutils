#objdump: -p
#as:	  -mlong -mshort-double
#name:	  Elf flags XGATE 32-bit int, 32-bit double
#source:  abi.s

*.*: +file format elf32\-xgate
private flags = 1:\[abi=32-bit int, 32-bit double, cpu=XGATE\]
