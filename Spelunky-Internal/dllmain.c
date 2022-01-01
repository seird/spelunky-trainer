#include "trainer/trainer.h"
#include <stdio.h>
#include <stdbool.h>
#include "hooks.h"


enum Property {
    P_HEALTH,
    P_BOMB,
    P_ROPE,
    P_GOLD,
    P_COUNT
};


uintptr_t base;


DWORD WINAPI
InjectThread(HMODULE hModule)
{
    /////////////////////////////////////////////////////////////////////////
    TOffsets offsets[P_COUNT] = {
        {(uintptr_t[]) { 0x154510, 0x30, 0x140 }, 3}, // health
        {(uintptr_t[]) {0x154510, 0x30, 0x280, 0x10}, 4}, // bomb
        {(uintptr_t[]) {0x154510, 0x30, 0x280, 0x14}, 4}, // rope
        {(uintptr_t[]) {0x15446c, 0x0044592C       }, 2}, // gold
    };

    TOffsets offsets_x = { (uintptr_t[]) { 0x154510, 0x30, 0x30 }, 3 };
    TOffsets offsets_y = { (uintptr_t[]) { 0x154510, 0x30, 0x34 }, 3 };


    int property_keys[P_COUNT] = {
        '1', // health
        '2', // bomb
        '3', // rope
        '4', // gold
    };

    int key_gold_hook = '5';

    int key_stop = VK_END;

    int property_values[P_COUNT] = {
        99,     // health
        98,     // bomb
        97,     // rope
        100000, // gold
    };

    bool property_status[P_COUNT] = { 0 };
    bool gold_hook_status = false;


    uintptr_t offset_damage = 0x15D8B; // disable player and enemy damage
    bool damage_enabled = true;
    int key_damage = '0';
    uint8_t damage_bytes_patched[] = {0xBF, 0x02, 0x00, 0x00, 0x00, 0x90};
    uint8_t damage_bytes_original[] = {0x8B, 0xBE, 0x40, 0x01, 0x00, 0x00};

    /////////////////////////////////////////////////////////////////////////


    AllocConsole();
    FILE * fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    base = trainer_internal_address_module_base();

    printf("Module base = %x\n\n", base);
    printf("Press 'End' to exit.\n\n");

    while (true) {
        // quit
        if (GetAsyncKeyState(key_stop) & 1) {
            printf("\nStopping .. \n");
            Sleep(1000);
            break;
        }

        // gold hook
        if (GetAsyncKeyState(key_gold_hook) & 1) {
            gold_hook_status = !gold_hook_status;

            if (gold_hook_status) {                
                gold_hook_inject();
            } else {
                gold_hook_eject();
            }
        }

        for (enum Property p = P_HEALTH; p < P_COUNT; ++p) {
            // toggle status
            if (GetAsyncKeyState(property_keys[p]) & 1) {
                property_status[p] = !property_status[p];
            }

            // apply status
            if (property_status[p]) {
                uintptr_t address = trainer_internal_address_get(base, &offsets[p]);
                if (address < base) continue; // when a new level loads the addresses become invalid for a small period of time
                trainer_internal_memory_write((void *)address, &property_values[p], sizeof(int));
            }
        }

        // collision
        if (GetAsyncKeyState(key_damage) & 1) {
            if (damage_enabled = !damage_enabled) {
                trainer_internal_memory_write_protect((uint8_t *) (base + offset_damage), damage_bytes_original, sizeof(damage_bytes_original));
            } else {
                trainer_internal_memory_write_protect((uint8_t *) (base + offset_damage), damage_bytes_patched, sizeof(damage_bytes_patched));
            }
        }

        // read the player position from memory
        float fx = 0;
        uintptr_t address_x = trainer_internal_address_get(base, &offsets_x);
        if (address_x < base) continue;
        trainer_internal_memory_read((void *)address_x, &fx, sizeof(fx));

        float fy = 0;
        uintptr_t address_y = trainer_internal_address_get(base, &offsets_y);
        if (address_y < base) continue;
        trainer_internal_memory_read((void *)address_y, &fy, sizeof(fy));

        // display status
        printf("\rHealth %3s (%c), Bomb %3s (%c), Rope %3s (%c), Gold %3s (%c), GoldHook %3s (%c), Damage %3s (%c), Postion: [%5.1f, %5.1f]",
            property_status[P_HEALTH] ? "on" : "off", property_keys[P_HEALTH],
            property_status[P_BOMB] ? "on" : "off", property_keys[P_BOMB],
            property_status[P_ROPE] ? "on" : "off", property_keys[P_ROPE],
            property_status[P_GOLD] ? "on" : "off", property_keys[P_GOLD],
            gold_hook_status ? "on" : "off", key_gold_hook,
            damage_enabled ? "on" : "off", key_damage,
            fx, fy);

        Sleep(10);
    }

    // restore original code bytes to avoid accessing this dll after it's been ejected
    gold_hook_eject();

    fclose(fp);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    HANDLE th;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InjectThread, hModule, 0, NULL);
        if (th != NULL)
            CloseHandle(th);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
