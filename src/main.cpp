#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>
#include "memscan.h"
#include "inlinehook.h"

// Definisi fungsi Minecraft
typedef void*(*t_getArmor)(void*, int);
typedef int(*t_getDmg)(void*);
typedef int(*t_getMaxDmg)(void*);
typedef void(*t_displayMsg)(void*, std::string const&);

t_getArmor _getArmor = nullptr;
t_getDmg _getDmg = nullptr;
t_getMaxDmg _getMaxDmg = nullptr;
t_displayMsg _displayMsg = nullptr;

void (*original_baseTick)(void*);

// Fungsi Logic Armor HUD
void hooked_baseTick(void* p) {
    if (p != nullptr && _getArmor && _displayMsg) {
        std::string hud = "§l§fArmor: ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            void* stack = _getArmor(p, i);
            if (stack != nullptr) {
                int max = _getMaxDmg(stack);
                if (max > 0) {
                    int val = max - _getDmg(stack);
                    std::string col = (val < max * 0.25) ? "§c" : (val < max * 0.5 ? "§e" : "§a");
                    hud += col + std::to_string(val) + "§f | ";
                    hasArmor = true;
                }
            }
        }
        if (hasArmor) _displayMsg(p, hud);
    }
    original_baseTick(p);
}

void hookArmor() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) return;

    _getArmor = (t_getArmor)dlsym(handle, "_ZNK4Player8getArmorEi");
    _getDmg = (t_getDmg)dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
    _getMaxDmg = (t_getMaxDmg)dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
    _displayMsg = (t_displayMsg)dlsym(handle, "_ZN11LocalPlayer20displayClientMessageERKSs");

    // Sigscan agar Ambient mendeteksi mod sebagai "Active"
    const char* pattern = "?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 FD 03 00 91 ?? ?? ?? D1 ?? ?? ?? D5 FA 03 00 AA F5 03 07 AA";
    sigscan_handle* scanner = sigscan_setup(pattern, "libminecraftpe.so", GPWN_SIGSCAN_XMEM);
    
    if (scanner) {
        void* func = get_sigscan_result(scanner);
        sigscan_cleanup(scanner);
        if (func && func != (void*)-1) {
            // Gunakan Microhook sesuai gaya Ambient
            hook_addr(func, (void*)hooked_baseTick, (void**)&original_baseTick, GPWN_AARCH64_MICROHOOK);
        }
    }
}

__attribute__((constructor))
void StartUp() {
    hookArmor();
}
