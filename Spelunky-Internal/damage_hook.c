#include <stdint.h>
#include <string.h>
#include "hooks.h"
#include "trainer/trainer.h"


extern uintptr_t base; // module base initialized in dllmain.c


static uintptr_t damage_hook_addr; // address to start the hook
static uintptr_t damage_hook_player_retaddr;
static uintptr_t damage_hook_normal_retaddr;


__declspec(naked) // make sure no extra instructions get inserted
static void
damage_hook()
{
    // save registers
    __asm pushad

    // new code

    // if [esi + 0xC] == 0, then the player is taking damage
    __asm {
        mov eax, [esi + 0xC]
        cmp eax, 0
        jnz function_resume
    }

    // the player receives damage -> skip function

    // restore registers
    __asm popad
    // continue rest of the start of the function
    __asm {
        _emit 0x80
        _emit 0xBE
        _emit 0x14
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00 ; cmp     byte ptr[esi + 214h], 0
        _emit 0x53 ; push    ebx
        _emit 0x55 ; push    ebp
        _emit 0x8B
        _emit 0x6C
        _emit 0x24
        _emit 0x1C ; mov     ebp, [esp + 14h + arg_4]
        _emit 0x57 ; push    edi
    }

    // jump to the end of the function
    __asm jmp damage_hook_player_retaddr

    // the player does damage -> resume function as normal
function_resume:
    __asm {
        __asm popad
        _emit 0x80
        _emit 0xBE
        _emit 0x14
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00 ; cmp     byte ptr[esi + 214h], 0
        jmp damage_hook_normal_retaddr
    }    
}


void
damage_hook_inject()
{
    damage_hook_addr = base + 0x15973;
    damage_hook_player_retaddr = base + 0x1632F;
    damage_hook_normal_retaddr = base + 0x1597A;

    // jump to hook: 0xE9 (addr_to_jump_to - (addr_to_jump_from + 0x5))
    uint8_t bytes[1 + sizeof(uintptr_t) + 2]; // push + jmp loc + 1 nops
    bytes[0] = 0xE9; // jmp
    *(uintptr_t *)(bytes + 1) = ((uintptr_t)&damage_hook - (damage_hook_addr + 0x5)); // loc
    bytes[1 + sizeof(uintptr_t)] = 0x90; //nops
    bytes[1 + sizeof(uintptr_t) + 1] = 0x90; //nops

    trainer_internal_memory_write_protect((uint8_t *)damage_hook_addr, bytes, sizeof(bytes));
}


void
damage_hook_eject()
{
    damage_hook_addr = base + 0x15973;

    // restore the original bytes
    uint8_t bytes[ 1 + sizeof(uintptr_t) + 2] = { 0x80, 0xBE, 0x14, 0x02, 0x00, 0x00, 0x00 };

    trainer_internal_memory_write_protect((uint8_t *)damage_hook_addr, bytes, sizeof(bytes));
}
