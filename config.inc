; CISC-16-A OS System Configuration

; Memory Map
VRAM_START      EQU 8000h       ; VGA Video RAM
PS2_DATA_PORT   EQU 0F000h      ; PS/2 data port
PS2_STAT_PORT   EQU 0F001h      ; PS/2 status port
RAM_TOP         EQU 7FFFh       ; Stack top

; VGA Settings
SCREEN_WIDTH    EQU 80          ; Screen width
SCREEN_HEIGHT   EQU 30          ; Screen height
DEFAULT_COLOR   EQU 0Fh         ; Default color (white on black)

; Buffer sizes
KBD_BUFFER_SIZE EQU 8           ; Keyboard buffer size
CMD_BUFFER_SIZE EQU 32          ; Command buffer size

; Boot ROM Settings
KERNEL_ENTRY    EQU 0FE00h      ; Kernel entry point
BOOT_ROM_SIZE   EQU 512         ; 512 byte limit
