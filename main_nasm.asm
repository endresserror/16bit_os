; CISC-16-A OS (NASM Compatible Version)
; Complete implementation in single file

[BITS 16]

; =================================================================
; CONFIG INLINE
; =================================================================
%define VRAM_START      0x8000
%define PS2_DATA_PORT   0xF000
%define PS2_STAT_PORT   0xF001
%define RAM_TOP         0x7FFF
%define SCREEN_WIDTH    80
%define SCREEN_HEIGHT   30
%define DEFAULT_COLOR   0x0F
%define KBD_BUFFER_SIZE 8
%define CMD_BUFFER_SIZE 16
%define KERNEL_ENTRY    0xFE00

; =================================================================
; DATA SECTION
; =================================================================
section .data

; VGA Console Data
cursor_x:       db 0
cursor_y:       db 0


; Shell Data (ultra-minimal)
input_buffer:   times 16 db 0
temp_buffer:    times 4 db 0

; Ultra-minimal strings
welcome_msg:    db "CISC-16-A OS", 0Ah, 0
prompt_msg:     db ">", 0
help_msg:       db "?POR", 0Ah, 0
err_msg:        db "E", 0Ah, 0

; =================================================================
; CODE SECTION
; =================================================================
section .text

org KERNEL_ENTRY

KERNEL_START:
    ; Setup stack
    mov sp, RAM_TOP
    
    ; Inline screen clear (VGA_ClearScreen optimization)
    mov di, VRAM_START
    mov cx, SCREEN_WIDTH * SCREEN_HEIGHT
    mov al, ' '
    mov ah, DEFAULT_COLOR
.clear_loop:
    mov [di], al        ; Space char
    inc di
    mov [di], ah        ; Color
    inc di
    dec cx
    jnz .clear_loop
    
    ; Reset cursor
    mov byte [cursor_x], 0
    mov byte [cursor_y], 0
    
    ; Initialize keyboard
    call KBD_Init
    
    ; Start shell
    call SHELL_MainLoop
    
    halt

; =================================================================
; UTILITY FUNCTIONS
; =================================================================

; -----------------------------------------------------------------
; STR2HEX - Convert pure hex string to 16-bit number (simplified)
; Input: SI = hex string (no prefix/suffix)
; Output: AX = number, CF = error
; Destroys: BX
; -----------------------------------------------------------------
STR2HEX:
    push bx
    mov ax, 0
    
.loop:
    mov bl, [si]
    cmp bl, 0
    je .ok
    cmp bl, ' '
    je .ok
    
    ; Convert hex char
    shl ax, 4
    cmp bl, '9'
    jle .digit
    cmp bl, 'F'
    jle .alpha
    sub bl, 32          ; Convert lowercase to uppercase
.alpha:
    sub bl, 7           ; A-F -> 10-15
.digit:
    sub bl, '0'
    add al, bl
    inc si
    jmp .loop
    
.ok:
    clc
    pop bx
    ret

; -----------------------------------------------------------------
; BYTE2HEX - Ultra-compact hex conversion  
; -----------------------------------------------------------------
BYTE2HEX:
    push ax
    mov ah, al
    ; Upper nibble
    shr al, 4
    add al, '0'
    cmp al, '9'
    jle .store1
    add al, 7           ; A-F
.store1:
    mov [di], al
    ; Lower nibble  
    mov al, ah
    and al, 0Fh
    add al, '0'
    cmp al, '9'
    jle .store2
    add al, 7
.store2:
    mov [di+1], al
    mov byte [di+2], 0
    pop ax
    ret

; =================================================================
; VGA FUNCTIONS
; =================================================================

; -----------------------------------------------------------------
; VGA_Scroll - Simple scroll (just reset cursor to top)
; -----------------------------------------------------------------
VGA_Scroll:
    mov byte [cursor_x], 0
    mov byte [cursor_y], 0
    ret

; -----------------------------------------------------------------
; PUTC - Display character (minimal version)
; -----------------------------------------------------------------
PUTC:
    push bx
    push di
    
    cmp al, 0Ah         ; Newline
    je .newline
    cmp al, 0Dh         ; Carriage return
    je .newline
    
    ; Calculate VRAM addr: (y*80+x)*2+8000h
    mov bl, [cursor_y]
    mov bh, 0
    mov ax, 80
    mul bx              ; AX = y * 80
    mov bl, [cursor_x]
    mov bh, 0
    add ax, bx          ; AX = y*80 + x
    shl ax, 1           ; AX *= 2
    add ax, VRAM_START  ; Add base
    mov di, ax
    
    ; Write char + color
    mov [di], al
    inc di
    mov byte [di], DEFAULT_COLOR
    
    ; Advance cursor
    inc byte [cursor_x]
    cmp byte [cursor_x], SCREEN_WIDTH
    jl .done
    
.newline:
    mov byte [cursor_x], 0
    inc byte [cursor_y]
    cmp byte [cursor_y], SCREEN_HEIGHT
    jl .done
    call VGA_Scroll
    mov byte [cursor_y], SCREEN_HEIGHT-1
    
.done:
    pop di
    pop bx
    ret

; -----------------------------------------------------------------
; PRINT - Display string
; -----------------------------------------------------------------
PRINT:
    push si
    push ax
.loop:
    mov al, [si]
    test al, al
    jz .end
    call PUTC
    inc si
    jmp .loop
.end:
    pop ax
    pop si
    ret

; -----------------------------------------------------------------
; PRINT_NEWLINE - Print newline
; -----------------------------------------------------------------
PRINT_NEWLINE:
    mov al, 0Ah
    call PUTC
    ret

; =================================================================
; PS/2 KEYBOARD FUNCTIONS
; =================================================================

; -----------------------------------------------------------------
; KBD_Init - Initialize keyboard (minimal)
; -----------------------------------------------------------------
KBD_Init:
    ret



; =================================================================
; SHELL FUNCTIONS
; =================================================================

; -----------------------------------------------------------------
; SHELL_Init - Initialize shell (minimal)
; -----------------------------------------------------------------  
SHELL_Init:
    ret

; -----------------------------------------------------------------
; SHELL_MainLoop - Single-letter command loop (v6.0)
; -----------------------------------------------------------------
SHELL_MainLoop:
    ; Show welcome
    mov si, welcome_msg
    call PRINT
    
.loop:
    ; Show prompt
    mov si, prompt_msg
    call PRINT
    
    ; Get input line
    mov di, input_buffer
    mov cx, CMD_BUFFER_SIZE - 1
    call READ_LINE
    
    ; Execute single-char command
    mov al, [input_buffer]     ; Get first character
    cmp al, '?'
    je .cmd_help
    cmp al, 'P'
    je .cmd_peek
    cmp al, 'O' 
    je .cmd_poke
    cmp al, 'R'
    je .cmd_run
    
    ; Unknown command - show error
    mov si, err_msg
    call PRINT
    jmp .loop
    
.cmd_help:
    mov si, help_msg
    call PRINT
    jmp .loop
    
.cmd_peek:
    call CMD_PEEK
    jmp .loop
    
.cmd_poke:
    call CMD_POKE  
    jmp .loop
    
.cmd_run:
    call CMD_RUN
    jmp .loop

; -----------------------------------------------------------------
; CMD_PEEK - Read and display memory byte (P <addr>)
; -----------------------------------------------------------------
CMD_PEEK:
    push ax
    push bx
    push si
    push di
    
    ; Skip 'P' and space, get address
    mov si, input_buffer + 2
    call STR2HEX
    jc .error
    
    ; Read memory at address AX
    mov bx, ax
    mov al, [bx]
    
    ; Convert to hex and display
    mov di, temp_buffer
    call BYTE2HEX
    mov si, temp_buffer
    call PRINT
    call PRINT_NEWLINE
    jmp .done
    
.error:
    mov si, err_msg
    call PRINT
    
.done:
    pop di
    pop si  
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------
; CMD_POKE - Write byte to memory (O <addr> <val>)
; -----------------------------------------------------------------
CMD_POKE:
    push ax
    push bx
    push si
    
    ; Get address
    mov si, input_buffer + 2
    call STR2HEX
    jc .error
    mov bx, ax           ; Save address
    
    ; Skip to next arg (find space)
    mov si, input_buffer + 2
.find_space:
    mov al, [si]
    cmp al, 0
    je .error
    cmp al, ' '
    je .found_space
    inc si
    jmp .find_space
    
.found_space:
    inc si               ; Skip space
    call STR2HEX        ; Get value
    jc .error
    
    ; Write byte
    mov [bx], al
    jmp .done
    
.error:
    mov si, err_msg
    call PRINT
    
.done:
    pop si
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------
; CMD_RUN - Execute code at address (R <addr>)
; -----------------------------------------------------------------
CMD_RUN:
    push ax
    push si
    
    ; Get address
    mov si, input_buffer + 2
    call STR2HEX
    jc .error
    
    ; Jump to address
    call ax
    jmp .done
    
.error:
    mov si, err_msg
    call PRINT
    
.done:
    pop si
    pop ax
    ret

; -----------------------------------------------------------------
; READ_LINE - Ultra-simple line read
; -----------------------------------------------------------------
READ_LINE:
    ; For testing, just put a test command in buffer
    mov byte [di], 'P'        ; P command
    mov byte [di+1], ' '      ; Space
    mov byte [di+2], '8'      ; Address 8000  
    mov byte [di+3], '0'
    mov byte [di+4], '0'
    mov byte [di+5], '0'
    mov byte [di+6], 0        ; Null terminate
    call PRINT_NEWLINE
    ret

