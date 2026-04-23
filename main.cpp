#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <string>

void* (*Player_getArmor)(void*, int);
int (*ItemStack_getDamageValue)(void*);
int (*ItemStack_getMaxDamage)(void*);
void (*LocalPlayer_displayClientMessage)(void*, std::string const&);
void (*LocalPlayer_normalTick_orig)(void*);

void LocalPlayer_normalTick_hook(void* player) {
    if (player != nullptr && Player_getArmor && ItemStack_getDamageValue && LocalPlayer_displayClientMessage) {
        std::string hudText = "";
        for (int i = 0; i < 4; i++) {
            void* armorItem = Player_getArmor(player, i);
            if (armorItem != nullptr) {
                int maxDmg = ItemStack_getMaxDamage(armorItem);
                if (maxDmg > 0) {
                    int curDmg = ItemStack_getDamageValue(armorItem);
                    int dura = maxDmg - curDmg;
                    std::string color = "§a";
                    if (dura < (maxDmg * 0.25)) color = "§c";
                    else if (dura < (maxDmg * 0.5)) color = "§e";
                    hudText += "  " + color + std::to_string(dura);
                }
            }
        }
        if (!hudText.empty()) {
            LocalPlayer_displayClientMessage(player, "§lARMOR: " + hudText);
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
        void* armorSym = dlsym(handle, "_ZNK4Player8getArmorEi");
        void* dmgSym = dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
        void* maxDmgSym = dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
        void* msgSym = dlsym(handle, "_ZN11LocalPlayer20displayClientMessageERKSs");
        void* tickSym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");

        if (armorSym && dmgSym && msgSym && tickSym) {
            Player_getArmor = (void*(*)(void*, int))armorSym;
            ItemStack_getDamageValue = (int(*)(void*))dmgSym;
            ItemStack_getMaxDamage = (int(*)(void*))maxDmgSym;
            LocalPlayer_displayClientMessage = (void(*)(void*, std::string const&))msgSym;
            LocalPlayer_normalTick_orig = (void(*)(void*))tickSym;
        }
    }
}
