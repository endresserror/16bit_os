; CISC-16-A OS Utility Functions

; Convert hex string to 16-bit number
; Input: SI = hex string (no prefix/suffix)
; Output: AX = number, CF = error flag
; Destroys: BX
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


; Convert byte to 2-character hex string
; Input: AL = byte to convert, DI = destination buffer (min 3 bytes)
; Output: DI points to NULL-terminated 2-char hex string
; Destroys: AH, BX
BYTE2HEX:
    push ax
    push bx
    
    mov bl, al          ; Save original byte
    
    ; Convert upper nibble
    shr al, 4
    and al, 0Fh
    cmp al, 9
    jle .upper_digit
    add al, 'A' - 10
    jmp .store_upper
.upper_digit:
    add al, '0'
.store_upper:
    mov [di], al
    inc di
    
    ; Convert lower nibble
    mov al, bl
    and al, 0Fh
    cmp al, 9
    jle .lower_digit
    add al, 'A' - 10
    jmp .store_lower
.lower_digit:
    add al, '0'
.store_lower:
    mov [di], al
    inc di
    
    ; NULL terminate
    mov byte [di], 0
    sub di, 2           ; Reset DI to start of string
    
    pop bx
    pop ax
    ret

