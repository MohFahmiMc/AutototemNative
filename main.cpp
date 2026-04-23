#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>

typedef void*(*t_getArmor)(void*, int);
typedef int(*t_getDmg)(void*);
typedef int(*t_getMaxDmg)(void*);
typedef void(*t_displayMsg)(void*, std::string const&);

t_getArmor _getArmor = nullptr;
t_getDmg _getDmg = nullptr;
t_getMaxDmg _getMaxDmg = nullptr;
t_displayMsg _displayMsg = nullptr;

void (*LocalPlayer_tick_orig)(void*) = nullptr;

void LocalPlayer_tick_hook(void* p) {
    if (p != nullptr && _getArmor && _displayMsg) {
        std::string hud = "§l§fArmor: [ ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            void* stack = _getArmor(p, i);
            if (stack != nullptr) {
                int max = _getMaxDmg(stack);
                if (max > 0) {
                    int cur = _getDmg(stack);
                    int val = max - cur;
                    
                    std::string col = "§a";
                    if (val < (max * 0.25)) col = "§c";
                    else if (val < (max * 0.5)) col = "§e";
                    
                    hud += col + std::to_string(val) + " §f| ";
                    hasArmor = true;
                }
            }
        }

        if (hasArmor) {
            _displayMsg(p, hud + "§f]");
        }
    }

    if (LocalPlayer_tick_orig) {
        LocalPlayer_tick_orig(p);
    }
}

__attribute__((constructor))
void init() {
    void* h = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (h) {
        _getArmor = (t_getArmor)dlsym(h, "_ZNK4Player8getArmorEi");
        _getDmg = (t_getDmg)dlsym(h, "_ZNK12ItemStackBase14getDamageValueEv");
        _getMaxDmg = (t_getMaxDmg)dlsym(h, "_ZNK12ItemStackBase12getMaxDamageEv");
        _displayMsg = (t_displayMsg)dlsym(h, "_ZN11LocalPlayer20displayClientMessageERKSs");
        
        void* t = dlsym(h, "_ZN11LocalPlayer10normalTickEv");
        if (t) {
            LocalPlayer_tick_orig = (void(*)(void*))t;
        }
    }
}
