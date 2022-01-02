; note the '_' prefixes, otherwise symbols won't be visible -- why? no idea


global _damage_hook


extern _damage_hook_normal_retaddr
extern _damage_hook_player_retaddr


section .text

_damage_hook:
    ; save registers
    pushad

    ; new code

    ; if [esi + 0xC] == 0, then the player is taking damage
    mov eax, [esi + 0xC]
    cmp eax, 0
    jnz function_resume

    ; the player receives damage -> skip function

    ; restore registers
    popad
    ; continue rest of the start of the function

    db 0x80, 0xBE, 0x14, 0x02, 0x00, 0x00, 0x00 ; cmp     byte ptr[esi + 214h], 0
    db 0x53                                     ; push    ebx
    db 0x55                                     ; push    ebp
    db 0x8B, 0x6C, 0x24, 0x1C                   ; mov     ebp, [esp + 14h + arg_4]
    db 0x57                                     ; push    edi


    ; jump to the end of the function
    jmp [_damage_hook_player_retaddr]

    ; the player does damage -> resume function as normal
function_resume:
    popad
    db 0x80, 0xBE, 0x14, 0x02, 0x00, 0x00, 0x00 ; cmp     byte ptr[esi + 214h], 0
    jmp [_damage_hook_normal_retaddr]
