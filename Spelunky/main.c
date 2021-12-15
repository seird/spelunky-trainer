#include "trainer/trainer.h"
#include <stdio.h>
#include <stdbool.h>


#define PROCESS_NAME "Spelunky.exe"


enum Property {
    P_HEALTH,
    P_BOMB,
    P_ROPE,
    P_GOLD,
    P_COUNT
};


int
main(void)
{
    /////////////////////////////////////////////////////////////////////////
    TOffsets offsets[P_COUNT] = {
        {(uintptr_t[]) {0x154510, 0x30, 0x140 }, 3}, // health
        {(uintptr_t[]) {0x154510, 0x30, 0x280, 0x10}, 4}, // bomb
        {(uintptr_t[]) {0x154510, 0x30, 0x280, 0x14}, 4}, // rope
        {(uintptr_t[]) {0x15446c, 0x0044592C       }, 2}, // gold
    };

    TOffsets offsets_x = {(uintptr_t[]) {0x154510, 0x30, 0x30}, 3};
    TOffsets offsets_y = {(uintptr_t[]) {0x154510, 0x30, 0x34}, 3};

    int property_keys[P_COUNT] = {
        '1', // health
        '2', // bomb
        '3', // rope
        '4', // gold
    };

    int key_stop = VK_END;

    int property_values[P_COUNT] = {
        99,     // health
        98,     // bomb
        97,     // rope
        100000, // gold
    };

    bool property_status[P_COUNT] = {0};


    uintptr_t offset_damage = 0x15D8B;
    bool damage_enabled = true;
    int key_damage = '0';
    uint8_t damage_bytes_patched[] = { 0xBF, 0x02, 0x00, 0x00, 0x00, 0x90 };
    uint8_t damage_bytes_original[] = { 0x8B, 0xBE, 0x40, 0x01, 0x00, 0x00 };

    /////////////////////////////////////////////////////////////////////////


    TProcess process = trainer_process_connect(PROCESS_NAME);
    if (process.id == 0 || process.handle == NULL) {
        return 0;
    }

    uintptr_t base = trainer_address_module_base(process.id, PROCESS_NAME);

    DWORD exit_code = 0;

    printf("Press 'End' to exit.\n\n");
    
    while (GetExitCodeProcess(process.handle, &exit_code) && exit_code == STILL_ACTIVE) {
        // quit
        if (GetAsyncKeyState(key_stop) & 1) {
            printf("\nStopping .. \n");
            Sleep(1000);
            break;
        }

        for (enum Property p = P_HEALTH; p < P_COUNT; ++p) {
            // toggle status
            if (GetAsyncKeyState(property_keys[p]) & 1) {
                property_status[p] = !property_status[p];
            }

            // apply status
            if (property_status[p]) {
                uintptr_t address = trainer_address_get(process.handle, base, &offsets[p]);
                trainer_memory_write(process, (void *)address, &property_values[p], sizeof(int));
            }
        }

        // collision
        if (GetAsyncKeyState(key_damage) & 1) {
            if (damage_enabled = !damage_enabled) {
                trainer_memory_write_protect((uint8_t *)(base + offset_damage), damage_bytes_original, sizeof(damage_bytes_original), process.handle);
            }
            else {
                trainer_memory_write_protect((uint8_t *)(base + offset_damage), damage_bytes_patched, sizeof(damage_bytes_patched), process.handle);
            }
        }

        // read the player position from memory
        float fx = 0;
        uintptr_t address_x = trainer_address_get(process.handle, base, &offsets_x);
        trainer_memory_read(process, (void *)address_x, &fx, sizeof(fx));

        float fy = 0;
        uintptr_t address_y = trainer_address_get(process.handle, base, &offsets_y);
        trainer_memory_read(process, (void *)address_y, &fy, sizeof(fy));

        // display status
        printf("\rHealth %3s (%c), Bomb %3s (%c), Rope %3s (%c), Gold %3s (%c), Damage %3s (%c), Postion: [%5.1f, %5.1f]",
            property_status[P_HEALTH] ? "on" : "off", property_keys[P_HEALTH],
            property_status[P_BOMB] ? "on" : "off", property_keys[P_BOMB],
            property_status[P_ROPE] ? "on" : "off", property_keys[P_ROPE],
            property_status[P_GOLD] ? "on" : "off", property_keys[P_GOLD],
            damage_enabled ? "on" : "off", key_damage,
            fx, fy);

        Sleep(10);
    }

    return CloseHandle(process.handle) == 0;
}
