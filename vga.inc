; CISC-16-A OS VGA Driver

section .data
cursor_x:       db 0            ; Cursor X position
cursor_y:       db 0            ; Cursor Y position

; NOTE: VGA_ClearScreen inlined in main.asm for size optimization

; Scroll screen up by one line
; Input: None
; Output: None
; Destroys: DI, SI, CX
VGA_Scroll:
    push di
    push si
    push cx
    
    ; Copy lines 2-(SCREEN_HEIGHT) to lines 1-(SCREEN_HEIGHT-1)
    mov di, VRAM_START                          ; Destination: line 1
    mov si, VRAM_START + (SCREEN_WIDTH * 2)     ; Source: line 2
    mov cx, SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * 2
.copy_loop:
    mov al, [si]
    mov [di], al
    inc si
    inc di
    dec cx
    jnz .copy_loop
    
    ; Clear the last line
    mov di, VRAM_START + (SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * 2)
    mov al, ' '
    mov cx, SCREEN_WIDTH
.clear_loop:
    mov [di], al        ; Character
    inc di
    mov al, DEFAULT_COLOR
    mov [di], al        ; Color
    inc di
    mov al, ' '         ; Reset for next iteration
    dec cx
    jnz .clear_loop
    
    pop cx
    pop si
    pop di
    ret

; Display character (minimal version)
; Input: AL = character
; Output: None
; Destroys: BX, DI
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
    add ax, VRAM_START  ; Add base address
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

; Display string
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

; Print newline
PRINT_NEWLINE:
    mov al, 0Ah
    call PUTC
    ret

