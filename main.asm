; CISC-16-A OS Main Kernel
; Modular architecture entry point

%include "config.inc"
%include "utils.inc" 
%include "vga.inc"
%include "ps2.inc"
%include "shell.inc"

ORG KERNEL_ENTRY

KERNEL_START:
    ; Setup stack
    MOV SP, RAM_TOP
    
    ; Inline screen clear (VGA_ClearScreen optimization)
    MOV DI, VRAM_START
    MOV CX, SCREEN_WIDTH * SCREEN_HEIGHT
    MOV AL, ' '
    MOV AH, DEFAULT_COLOR
.clear_loop:
    MOV [DI], AL        ; Space char
    INC DI
    MOV [DI], AH        ; Color
    INC DI
    DEC CX
    JNZ .clear_loop
    
    ; Reset cursor
    MOV BYTE [cursor_x], 0
    MOV BYTE [cursor_y], 0
    
    ; Initialize keyboard
    CALL KBD_Init
    
    ; Start shell
    CALL SHELL_MainLoop
    
    HALT

END KERNEL_START
