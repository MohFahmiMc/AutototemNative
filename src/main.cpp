#include <dlfcn.h>
#include <string>
#include <vector>
#include <cstdio>
#include "memscan.h"
#include "inlinehook.h"

typedef void*(*t_getArmor)(void*, int);
typedef int(*t_getDmg)(void*);
typedef int(*t_getMaxDmg)(void*);
typedef void(*t_displayMsg)(void*, std::string const&);

t_getArmor _getArmor = nullptr;
t_getDmg _getDmg = nullptr;
t_getMaxDmg _getMaxDmg = nullptr;
t_displayMsg _displayMsg = nullptr;

void (*original_tick)(void*);

void hooked_tick(void* p) {
    if (p != nullptr && _getArmor && _displayMsg) {
        std::string hudText = "§l§fArmor: ";
        bool hasArmor = false;
        for (int i = 0; i < 4; i++) {
            void* stack = _getArmor(p, i);
            if (stack != nullptr) {
                int max = _getMaxDmg(stack);
                if (max > 0 && max < 10000) {
                    int val = max - _getDmg(stack);
                    std::string col = (val < max * 0.25) ? "§c" : (val < max * 0.5 ? "§e" : "§a");
                    hudText += col + std::to_string(val) + " §f| ";
                    hasArmor = true;
                }
            }
        }
        if (hasArmor) _displayMsg(p, hudText);
    }
    if (original_tick) original_tick(p);
}

__attribute__((constructor))
void StartUp() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) return;

    _getArmor = (t_getArmor)dlsym(handle, "_ZNK4Player8getArmorEi");
    _getDmg = (t_getDmg)dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
    _getMaxDmg = (t_getMaxDmg)dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
    _displayMsg = (t_displayMsg)dlsym(handle, "_ZN11LocalPlayer20displayClientMessageERKSs");

    void* target = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");
    if (!target) target = dlsym(handle, "_ZN4Player10normalTickEv");

    if (target) {
        hook_addr(target, (void*)hooked_tick, (void**)&original_tick, GPWN_AARCH64_MICROHOOK);
    }
}
