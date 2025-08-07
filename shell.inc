; CISC-16-A OS Shell

section .data
input_buffer:   times CMD_BUFFER_SIZE db 0
temp_buffer:    times 8 db 0

; Minimal strings
welcome_msg:    db "CISC-16-A OS", 0Dh, 0Ah, 0
prompt_msg:     db "> ", 0
help_msg:       db "? P O R", 0Dh, 0Ah, 0
err_msg:        db "E", 0Dh, 0Ah, 0

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
; READ_LINE - Read line of input from keyboard (minimal)
; Input: DI = buffer, CX = max chars
; Output: DI contains null-terminated string
; -----------------------------------------------------------------
READ_LINE:
    push ax
    push bx
    push si
    mov si, di          ; SI = start of buffer
    mov bx, 0           ; BX = character count
    
.input_loop:
    call KBD_GetChar    ; Wait for character
    cmp al, 0Dh         ; Enter?
    je .input_done
    cmp al, 08h         ; Backspace?
    je .backspace
    
    ; Normal character
    cmp bx, cx          ; Buffer full?
    jae .input_loop
    mov [di], al        ; Store char
    call PUTC           ; Echo char
    inc di
    inc bx
    jmp .input_loop
    
.backspace:
    cmp bx, 0           ; At start?
    je .input_loop
    dec di
    dec bx
    call PUTC           ; Echo backspace
    jmp .input_loop
    
.input_done:
    mov byte [di], 0    ; Null terminate
    call PRINT_NEWLINE
    pop si
    pop bx 
    pop ax
    ret

; -----------------------------------------------------------------
; KBD_GetChar - Get character from keyboard (blocking)
; Output: AL = character
; -----------------------------------------------------------------
KBD_GetChar:
    push bx
.wait_key:
    call KBD_ReadScancode
    call KBD_ScancodeToASCII
    cmp al, 0
    je .wait_key        ; Invalid key, try again
    pop bx
    ret
