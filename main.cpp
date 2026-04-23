#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AutoSprint"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Fungsi pointer untuk memanggil fitur lari
void (*setSprinting)(void*, bool) = nullptr;

// Fungsi Hook untuk memantau pergerakan player
void (*Player_normalTick_orig)(void*) = nullptr;

void Player_normalTick_hook(void* player) {
    if (player != nullptr && setSprinting != nullptr) {
        // PAKSA SPRINT: Begitu player melakukan tick (bergerak/napas), paksa status lari
        setSprinting(player, true);
    }
    
    // Kembalikan ke fungsi asli game
    if (Player_normalTick_orig != nullptr) {
        Player_normalTick_orig(player);
    }
}

__attribute__((constructor))
void init() {
    LOGI("Auto Sprint Mod v1.26.13.1 - Ambient Style");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Mencari simbol berdasarkan versi 1.26.13.1
        void* sprint_sym = dlsym(handle, "_ZN12EntityContext12setSprintingEb");
        void* tick_sym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");

        if (sprint_sym) {
            setSprinting = (void(*)(void*, bool))sprint_sym;
            LOGI("Sprint function linked!");
        }

        if (tick_sym) {
            Player_normalTick_orig = (void(*)(void*))tick_sym;
            LOGI("Movement tick hooked!");
        }
    } else {
        LOGI("Critical Error: libminecraftpe.so not found!");
    }
}
