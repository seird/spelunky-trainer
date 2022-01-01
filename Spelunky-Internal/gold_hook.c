#include <stdint.h>
#include <string.h>
#include "hooks.h"
#include "trainer/trainer.h"


extern uintptr_t base; // module base initialized in dllmain.c


static uintptr_t gold_hook_addr; // address to start the hook
static uintptr_t gold_hook_retaddr; // address to jump back to after the hook is finished


__declspec(naked) // make sure no extra instructions get inserted
static void
gold_hook()
{
    // pushad / popad

    // new code
    // set the item price to 0
    __asm mov ecx, 0
    
    // jump back to game code
    __asm jmp gold_hook_retaddr
}


void
gold_hook_inject()
{
    gold_hook_addr = base + 0x56FA0;
    gold_hook_retaddr = base + 0x57165;

    // jump to hook: 0xE9 (addr_to_jump_to - (addr_to_jump_from + 0x5))
    uint8_t bytes[1 + sizeof(uintptr_t) + 3]; // jmp loc + 3 nops
    bytes[0] = 0xE9; // jmp
    *(uintptr_t *)(bytes + 1) = ((uintptr_t)&gold_hook - (gold_hook_addr + 0x5)); // loc
    memset(bytes + 1 + sizeof(uintptr_t), 0x90, 3); //nops

    trainer_internal_memory_write_protect((uint8_t *)gold_hook_addr, bytes, sizeof(bytes));
}


void
gold_hook_eject()
{
    gold_hook_addr = base + 0x56FA0;

    // restore the original bytes
    uint8_t bytes[1 + sizeof(uintptr_t) + 3] = { 0x3B, 0xC1, 0x0F, 0x8C, 0x61, 0x03, 0x00, 0x00 };

    trainer_internal_memory_write_protect((uint8_t *)gold_hook_addr, bytes, sizeof(bytes));
}
