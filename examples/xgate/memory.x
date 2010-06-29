/* Memory definition for a 68HC11 program in EEPROM (1024 bytes) */
MEMORY
{
  page0  (rwx) : ORIGIN = 0x0000, LENGTH = 0x03ff
  eeprom (rx)  : ORIGIN = 0xfe00, LENGTH = 0x01ff
  text   (rx)  : ORIGIN = 0xfe00, LENGTH = 0x01ff
  data         : ORIGIN = 0x0000, LENGTH = 0x03ff
}

/* Setup the stack on the top of the data internal ram (not used).  */
PROVIDE (_stack = 0x0400-1);
