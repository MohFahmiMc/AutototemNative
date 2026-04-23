#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>

typedef void(*t_displayClientMessage)(void*, std::string const&);
t_displayClientMessage displayClientMessage = nullptr;

typedef void*(*t_getArmor)(void*, int);
t_getArmor getArmor = nullptr;

typedef int(*t_getDamageValue)(void*);
t_getDamageValue getDamageValue = nullptr;

typedef int(*t_getMaxDamage)(void*);
t_getMaxDamage getMaxDamage = nullptr;

void (*LocalPlayer_normalTick_orig)(void*);

void LocalPlayer_normalTick_hook(void* player) {
    if (player != nullptr && getArmor && getDamageValue && displayClientMessage) {
        std::string out = "§l§f[ ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            void* stack = getArmor(player, i);
            if (stack != nullptr) {
                int max = getMaxDamage(stack);
                if (max > 0) {
                    int val = max - getDamageValue(stack);
                    std::string col = "§a";
                    if (val < (max * 0.25)) col = "§c";
                    else if (val < (max * 0.5)) col = "§e";
                    out += col + std::to_string(val) + " §f| ";
                    hasArmor = true;
                }
            }
        }

        if (hasArmor) {
            out += " ]";
            displayClientMessage(player, out);
        }
    }

    if (LocalPlayer_normalTick_orig) {
        LocalPlayer_normalTick_orig(player);
    }
}

__attribute__((constructor))
void init() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        void* tick_sym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");
        void* armor_sym = dlsym(handle, "_ZNK4Player8getArmorEi");
        void* dmg_sym = dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
        void* max_dmg_sym = dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
        void* msg_sym = dlsym(handle, "_ZN11LocalPlayer20displayClientMessageERKSs");

        if (tick_sym && armor_sym && dmg_sym && msg_sym) {
            getArmor = (t_getArmor)armor_sym;
            getDamageValue = (t_getDamageValue)dmg_sym;
            getMaxDamage = (t_getMaxDamage)max_dmg_sym;
            displayClientMessage = (t_displayClientMessage)msg_sym;
            LocalPlayer_normalTick_orig = (void(*)(void*))tick_sym;
        }
    }
}
