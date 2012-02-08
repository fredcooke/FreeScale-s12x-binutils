#objdump: -p
#as:	  -mshort-double
#name:	  Elf flags XGATE 16-bit int, 32-bit double
#source:  abi.s

.*: +file format elf32\-xgate
private flags = 0:\[abi=16-bit int, 32-bit double, cpu=XGATE\] \[memory=flat\]
