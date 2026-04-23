#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AmbientMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Fungsi untuk lari
void (*setSprinting)(void*, bool);

// Hook normalTick agar lari aktif terus saat bergerak
void (*Player_normalTick_orig)(void*);
void Player_normalTick_hook(void* player) {
    if (player != nullptr && setSprinting != nullptr) {
        setSprinting(player, true); 
    }
    if (Player_normalTick_orig) Player_normalTick_orig(player);
}

__attribute__((constructor))
void init() {
    LOGI("Auto Sprint Ambient Loaded for 1.26.13.1");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Simbol khusus untuk Minecraft versi terbaru
        void* sprintSym = dlsym(handle, "_ZN12EntityContext12setSprintingEb");
        void* tickSym = dlsym(handle, "_ZN6Player10normalTickEv");

        if (sprintSym) {
            setSprinting = (void(*)(void*, bool))sprintSym;
            LOGI("Sprint function linked!");
        }
    }
}
