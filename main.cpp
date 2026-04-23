#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>

// Kita panggil langsung file-nya
#include "LocalPlayer.h"
#include "ItemStack.h"

void (*LocalPlayer_tick_orig)(void*);

void LocalPlayer_tick_hook(void* p) {
    auto* player = (LocalPlayer*)p;
    if (player != nullptr) {
        std::string hudText = "§l§f[ ";
        bool hasArmor = false;

        for (int i = 0; i < 4; i++) {
            auto* stack = player->getArmor(i);
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
    LocalPlayer_tick_orig(p);
}

__attribute__((constructor))
void init() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        void* tick_sym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");
        if (tick_sym) {
            LocalPlayer_tick_orig = (void(*)(void*))tick_sym;
        }
    }
}
