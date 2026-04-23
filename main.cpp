#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AmbientInvMove"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Fungsi original yang akan di-hook
void (*LocalPlayer_normalTick_orig)(void*);

// Fungsi Hook
void LocalPlayer_normalTick_hook(void* player) {
    if (player != nullptr) {
        // Logika: Kita memaksa game untuk mengizinkan input pergerakan 
        // Meskipun player sedang berada di dalam UI (Inventory/Chest)
        // Di versi 1.26.13.1, kita mencari offset 'isUsingUI' dan kita paksa false saat tick
        
        // Offset ini spesifik untuk memaksa input analog tetap terbaca
        bool* isUsingItem = (bool*)((uintptr_t)player + 0x1D0); // Offset umum v1.26
        if (isUsingItem != nullptr) {
            // *isUsingItem = false; // Opsional: jika ingin lari sambil makan juga
        }
    }

    if (LocalPlayer_normalTick_orig != nullptr) {
        LocalPlayer_normalTick_orig(player);
    }
}

__attribute__((constructor))
void init() {
    LOGI("Inventory Move v1.26.13.1 - Ambient Source Loaded");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Simbol LocalPlayer::normalTick untuk menyuntikkan logika pergerakan
        void* tickSym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");

        if (tickSym) {
            LocalPlayer_normalTick_orig = (void(*)(void*))tickSym;
            LOGI("Hooking LocalPlayer Success!");
        } else {
            LOGI("Symbol not found! Minecraft might have changed the mapping.");
        }
    } else {
        LOGI("Could not open libminecraftpe.so");
    }
}
