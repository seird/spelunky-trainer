#include <stdint.h>
#include <string.h>
#include "hooks.h"
#include "trainer/trainer.h"


extern uintptr_t base; // module base initialized in dllmain.c

extern void damage_hook();


static uintptr_t damage_hook_addr; // address to start the hook
uintptr_t damage_hook_player_retaddr; // to be accessed by damage_hook_asm.asm
uintptr_t damage_hook_normal_retaddr; // to be accessed by damage_hook_asm.asm


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
