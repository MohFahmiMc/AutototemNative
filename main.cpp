#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>
#include "gamepwnage/LocalPlayer.h"
#include "gamepwnage/ItemStack.h"

void (*LocalPlayer_tick_orig)(LocalPlayer*);

void LocalPlayer_tick_hook(LocalPlayer* player) {
    if (player != nullptr) {
        std::string hudText = "§l§f[ ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            ItemStack* stack = player->getArmor(i);
            if (stack != nullptr && !stack->isNull()) {
                int max = stack->getMaxDamage();
                if (max > 0) {
                    int val = max - stack->getDamageValue();
                    std::string col = (val < max * 0.25) ? "§c" : (val < max * 0.5 ? "§e" : "§a");
                    hudText += col + std::to_string(val) + " §f| ";
                    hasArmor = true;
                }
            }
        }

        if (hasArmor) {
            player->displayClientMessage(hudText + "§f]");
        }
    }
    LocalPlayer_tick_orig(player);
}

__attribute__((constructor))
void init() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        void* tick_sym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");
        if (tick_sym) {
            LocalPlayer_tick_orig = (void(*)(LocalPlayer*))tick_sym;
        }
    }
}
