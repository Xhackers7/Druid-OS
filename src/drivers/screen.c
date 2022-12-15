#include "screen.h"

void VGA_set_cursor(uint8_t csrX, uint8_t csrY)
{
  currentContext.csrX = csrX;
  currentContext.csrY = csrY;

  if (currentContext.csrX >= currentContext.maxCols)
  {
    currentContext.csrX = 0;
    currentContext.csrY++;
  }
  if (currentContext.csrY >= currentContext.maxRows)
    VGA_scroll(1);

  uint32_t offset = csr_to_offset(currentContext.csrX, currentContext.csrY);
  offset /= 2;
  x86_write_port_byte(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
  x86_write_port_byte(VGA_DATA_REGISTER, (uint8_t)(offset >> 8));
  x86_write_port_byte(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
  x86_write_port_byte(VGA_DATA_REGISTER, (uint8_t)(offset & 0xff));
}

uint32_t VGA_get_cursor_offset()
{
  x86_write_port_byte(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
  uint32_t offset = x86_read_port_byte(VGA_DATA_REGISTER) << 8;
  x86_write_port_byte(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
  offset += x86_read_port_byte(VGA_DATA_REGISTER);
  return offset * 2;
}

void VGA_clear(uint8_t color)
{
  if (!color)
    color = currentContext.color;

  currentContext.color = color;

  uint16_t blank = 0x20 | (color << 8);

  memsetw((uint16_t *)currentContext.videoAddress, blank, currentContext.maxCols * currentContext.maxRows);
  VGA_set_cursor(0, 0);
}

void VGA_print_char_at(char character, int col, int row, uint8_t attribute)
{
  VGA_set_cursor(col, row);
  VGA_print_char(character, attribute);
}

void VGA_print_char(char character, uint8_t attribute)
{
  if (!attribute)
  {
    attribute = currentContext.color;
  }

  if (character == '\n')
  {
    currentContext.csrX = 0;
    currentContext.csrY++;
  }

  else if (character == '\r')
  {
    currentContext.csrX = 0;
  }
  else if (character == '\t')
  {
    currentContext.csrX = (currentContext.csrX + 8) & ~(8 - 1);
  }
  else if (character == '\b')
  {
   if (currentContext.csrX != 0) currentContext.csrX--;
   uint32_t offset = csr_to_offset(currentContext.csrX, currentContext.csrY);
   memset(currentContext.videoAddress + offset, ' ', 1);
   memset(currentContext.videoAddress + offset + 1, attribute, 1);
  }

  /* Any character greater than or equal to space is printable */
  else if (character >= ' ')
  {
    uint32_t offset = csr_to_offset(currentContext.csrX, currentContext.csrY);
    memset(currentContext.videoAddress + offset, character, 1);
    memset(currentContext.videoAddress + offset + 1, attribute, 1);
    currentContext.csrX++;
  }
  VGA_set_cursor(currentContext.csrX, currentContext.csrY);
}

uint32_t VGA_scroll(uint8_t amt)
{
  uint16_t blank = 0x20 | (currentContext.color << 8);
  uint32_t offset = (amt * currentContext.maxCols) * 2; // the memory offset where x=0, y=amt
  uint32_t count = (currentContext.maxRows - amt) * 80; // number of words(2 bytes) till x=79, y=0 from x=0, y=amt
  memcpy(currentContext.videoAddress, currentContext.videoAddress + offset, count * 2);
  VGA_set_cursor(currentContext.csrX, currentContext.csrY - amt);                  //  Move csrY back amt times so that its relatively in the same position it originally was
  memsetw((uint16_t *)(currentContext.videoAddress + count * 2), blank, amt * 80); // sets all rows from x=79, y=24-amt to the end of vga array as the blank character
  return VGA_get_cursor_offset();
}

void VGA_init()
{
  currentContext.color = VGA_DEFAULT_COLOR;
  VGA_set_cursor(0, 0);
  VGA_clear(VGA_DEFAULT_COLOR);
}

// Helper functions
uint32_t csr_to_offset(uint8_t col, uint8_t row)
{
  return 2 * (row * currentContext.maxCols + col);
}