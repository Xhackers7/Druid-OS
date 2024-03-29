[bits 16]
[org 0x7c00]

%define ENDL 0x0D, 0x0A, 0

start: jmp boot

KERNEL_OFFSET equ 0x7f00

%include "src/boot/bios_functions/out.asm"
%include "src/boot/bios_functions/disk.asm"
%include "src/boot/gdt.asm"
%include "src/boot/switch-to-32-bit.asm"

load_kernel:
  mov dl, [BOOT_DRIVE]
  mov bx, KERNEL_OFFSET   ; store to es:bx(0x0:0x7f00)
  mov dh, 30   ; read 30 segments(15.36 KB) just to be safe when our kerne; grows

  ; loads and jumps to the kernel
  call read_kernel
  ret


halt:
  hlt
  jmp halt


boot:
  mov [BOOT_DRIVE], dl
  
  mov bp, 0x7c00
  mov sp, bp


  mov si, wlcm_msg
  call print

  mov ah, 00h
  int 16h

  call load_kernel
  call switch_to_32_bit

  jmp halt


[bits 32]
START_KENREL:
  call KERNEL_OFFSET
  jmp halt


wlcm_msg: db "Druid v0.20.23.3. Press any key to boot", ENDL
BOOT_DRIVE: db 0

times 510-($-$$) db 0
dw 0xaa55