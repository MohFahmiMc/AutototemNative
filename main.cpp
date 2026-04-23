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

void (*LocalPlayer_baseTick_orig)(void*);

void LocalPlayer_baseTick_hook(void* p) {
    if (p != nullptr && _getArmor && _displayMsg) {
        // Pakai stringstream manual biar lebih ringan di Vivo Y12
        std::string hud = "§l§fArmor: ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            void* stack = _getArmor(p, i);
            if (stack != nullptr) {
                int max = _getMaxDmg(stack);
                // Di server, max damage harus lebih dari 0
                if (max > 0) {
                    int cur = _getDmg(stack);
                    int val = max - cur;
                    
                    std::string col = "§a";
                    if (val < (max * 0.2)) col = "§c"; 
                    else if (val < (max * 0.4)) col = "§e";
                    
                    hud += col + std::to_string(val) + "§f | ";
                    hasArmor = true;
                }
            }
        }

        if (hasArmor) {
            // Gunakan displayClientMessage yang paling stabil buat server
            _displayMsg(p, hud);
        }
    }
    LocalPlayer_baseTick_orig(p);
}

__attribute__((constructor))
void init() {
    void* h = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (h) {
        _getArmor = (t_getArmor)dlsym(h, "_ZNK4Player8getArmorEi");
        _getDmg = (t_getDmg)dlsym(h, "_ZNK12ItemStackBase14getDamageValueEv");
        _getMaxDmg = (t_getMaxDmg)dlsym(h, "_ZNK12ItemStackBase12getMaxDamageEv");
        _displayMsg = (t_displayMsg)dlsym(h, "_ZN11LocalPlayer20displayClientMessageERKSs");
        
        // baseTick lebih stabil untuk menangkap data dari server dibanding normalTick
        void* bt = dlsym(h, "_ZN11LocalPlayer8baseTickEv");
        if (bt) {
            LocalPlayer_baseTick_orig = (void(*)(void*))bt;
        }
    }
}
