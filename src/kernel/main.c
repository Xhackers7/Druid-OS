#include "system.h"
#include "stdio.h"

void _kernel_panic(char *err)
{
    VGA_init();
    VGA_clear(0x41);
    puts("\n\n\n\n\n\n\n\n\n\n\t\t\t\t  KERNEL PANIC!! (O-O)\n\t\t\t\t");
    puts(err);
    for (;;);
}

void _test()
{
    VGA_init();
    puts("IF this works then yaya\t wohoo\nblu\rbro\bu");
}

void _start() {
    _test();
    _kernel_panic("Error description here");  // uncomment to test kernel panic screen
    for (;;);
}
