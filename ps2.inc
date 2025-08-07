; CISC-16-A OS PS/2 Driver

section .data
kbd_buffer:     times KBD_BUFFER_SIZE db 0
kbd_head:       db 0
kbd_tail:       db 0

; Minimal hex-only scancode map (hex digits + Enter/Backspace only)
hex_scancodes:  db 02h, 03h, 04h, 05h, 06h, 07h, 08h, 09h, 0Ah, 0Bh  ; 0-9
                db 1Eh, 30h, 2Eh, 21h, 12h, 23h                      ; A-F  
hex_chars:      db '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'    ; 0-9
                db 'A', 'B', 'C', 'D', 'E', 'F'                      ; A-F

; -----------------------------------------------------------------
; KBD_Init - Initialize keyboard (minimal)
; -----------------------------------------------------------------
KBD_Init:
    mov byte [kbd_head], 0
    mov byte [kbd_tail], 0
    ret

; -----------------------------------------------------------------
; KBD_CheckInput - Check if keyboard input is available (non-blocking)
; Input: None
; Output: CF = 1 if input available, CF = 0 if no input
; Destroys: AL
; -----------------------------------------------------------------
KBD_CheckInput:
    ; Check hardware status port
    mov al, [PS2_STAT_PORT]
    test al, 01h            ; Check data ready bit
    jz .no_input
    
    stc                     ; Set carry flag - input available
    ret
    
.no_input:
    clc                     ; Clear carry flag - no input
    ret

; -----------------------------------------------------------------
; KBD_ReadScancode - Read raw scancode from hardware (blocking)
; Input: None
; Output: AL = scancode
; Destroys: None
; -----------------------------------------------------------------
KBD_ReadScancode:
    push bx
    
.wait_loop:
    call KBD_CheckInput
    jnc .wait_loop          ; Wait until input available
    
    ; Read scancode from data port
    mov al, [PS2_DATA_PORT]
    
    pop bx
    ret

; -----------------------------------------------------------------
; KBD_ScancodeToASCII - Convert hex scancode to ASCII (minimal)
; Input: AL = scancode
; Output: AL = ASCII character (0 if invalid)
; Destroys: BX, CX
; -----------------------------------------------------------------
KBD_ScancodeToASCII:
    push bx
    push cx
    
    ; Special cases
    cmp al, 0Eh         ; Backspace
    je .backspace
    cmp al, 1Ch         ; Enter
    je .enter
    cmp al, 39h         ; Space 
    je .space
    
    ; Search hex scancode table
    mov cx, 16          ; 16 hex chars (0-9, A-F)
    mov bx, 0
.search:
    cmp al, [hex_scancodes + bx]
    je .found
    inc bx
    dec cx
    jnz .search
    
    ; Not found
    mov al, 0
    jmp .end
    
.found:
    mov al, [hex_chars + bx]
    jmp .end
    
.backspace:
    mov al, 08h
    jmp .end
    
.enter:
    mov al, 0Dh
    jmp .end
    
.space:
    mov al, ' '
    
.end:
    pop cx
    pop bx
    ret

; -----------------------------------------------------------------
; KBD_BufferPut - Add character to keyboard buffer
; Input: AL = character to add
; Output: CF = 1 if buffer full, CF = 0 if successful
; Destroys: BX
; -----------------------------------------------------------------
KBD_BufferPut:
    push bx
    
    ; Calculate next tail position
    mov bl, [kbd_tail]
    inc bl
    and bl, KBD_BUFFER_SIZE - 1    ; Wrap around using mask (assumes power of 2)
    
    ; Check if buffer would be full
    cmp bl, [kbd_head]
    je .buffer_full
    
    ; Store character in buffer
    mov bh, 0
    mov bl, [kbd_tail]
    and bl, KBD_BUFFER_SIZE - 1
    mov [kbd_buffer + bx], al
    
    ; Update tail pointer
    mov bl, [kbd_tail]
    inc bl
    and bl, KBD_BUFFER_SIZE - 1
    mov [kbd_tail], bl
    
    clc                     ; Success
    pop bx
    ret
    
.buffer_full:
    stc                     ; Buffer full error
    pop bx
    ret

; -----------------------------------------------------------------
; KBD_BufferGet - Get character from keyboard buffer
; Input: None
; Output: AL = character, CF = 1 if buffer empty
; Destroys: BX
; -----------------------------------------------------------------
KBD_BufferGet:
    push bx
    
    ; Check if buffer is empty
    mov al, [kbd_head]
    cmp al, [kbd_tail]
    je .buffer_empty
    
    ; Get character from buffer
    mov bl, [kbd_head]
    mov bh, 0
    and bl, KBD_BUFFER_SIZE - 1
    mov al, [kbd_buffer + bx]
    
    ; Update head pointer
    inc byte [kbd_head]
    mov bl, [kbd_head]
    and bl, KBD_BUFFER_SIZE - 1
    mov [kbd_head], bl
    
    clc                     ; Success
    pop bx
    ret
    
.buffer_empty:
    stc                     ; Buffer empty
    pop bx
    ret

; -----------------------------------------------------------------
; GETC - Get single character from keyboard (blocking)
; Input: None
; Output: AL = ASCII character
; Destroys: None
; -----------------------------------------------------------------
GETC:
    push bx
    
.input_loop:
    ; Try to get character from buffer first
    call KBD_BufferGet
    jnc .got_char
    
    ; Buffer empty - check for new hardware input
    call KBD_CheckInput
    jnc .input_loop         ; No hardware input - keep waiting
    
    ; Read and convert scancode
    call KBD_ReadScancode
    call KBD_ScancodeToASCII
    
    ; Skip if invalid character
    cmp al, 0
    je .input_loop
    
    ; For immediate use, return directly or buffer it
    ; We'll return directly for simplicity
    jmp .got_char
    
.got_char:
    pop bx
    ret

; -----------------------------------------------------------------
; READ_LINE - Read line of input with editing support
; Input: DI = buffer address, CX = maximum length
; Output: DI points to NULL-terminated string
; Destroys: AX, BX, DX
; -----------------------------------------------------------------
READ_LINE:
    push si
    push di
    push cx
    
    mov si, di              ; Save start of buffer
    mov dx, 0               ; Character count
    mov bx, cx              ; Save max length
    
.input_loop:
    call GETC               ; Get character
    
    ; Handle special characters
    cmp al, 0Dh             ; Enter key?
    je .line_complete
    
    cmp al, 08h             ; Backspace?
    je .handle_backspace
    
    cmp al, 09h             ; Tab? (ignore)
    je .input_loop
    
    ; Check for printable character
    cmp al, ' '
    jl .input_loop          ; Skip control characters
    cmp al, 7Eh             ; '~'
    jg .input_loop          ; Skip extended ASCII
    
    ; Check buffer space
    cmp dx, bx
    jge .input_loop         ; Buffer full
    
    ; Add character to buffer
    mov [di], al
    inc di
    inc dx
    
    ; Echo character to screen
    call PUTC
    jmp .input_loop
    
.handle_backspace:
    cmp dx, 0               ; Any characters to delete?
    je .input_loop
    
    ; Remove character from buffer
    dec di
    dec dx
    
    ; Echo backspace to screen
    mov al, 08h
    call PUTC
    jmp .input_loop
    
.line_complete:
    ; NULL-terminate string
    mov byte [di], 0
    
    ; Echo newline
    call PRINT_NEWLINE
    
    ; Restore original buffer pointer
    pop cx
    pop di
    pop si
    ret
