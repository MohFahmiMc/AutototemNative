#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AmbientMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Definisi fungsi untuk status lari
void (*setSprinting)(void*, bool);

// Hook ke fungsi tick supaya lari otomatis aktif terus
void (*Player_normalTick_orig)(void*);
void Player_normalTick_hook(void* player) {
    if (player != nullptr) {
        // Jika fungsi setSprinting ditemukan, paksa lari jadi true
        if (setSprinting != nullptr) {
            setSprinting(player, true);
        }
    }
    // Jalankan fungsi asli Minecraft
    if (Player_normalTick_orig) {
        Player_normalTick_orig(player);
    }
}

__attribute__((constructor))
void init() {
    LOGI("Auto Sprint Ambient v1.26.13.1 Starting...");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Menggunakan simbol mangled name yang tepat untuk 1.26.x
        void* sprintSym = dlsym(handle, "_ZN12EntityContext12setSprintingEb");
        
        if (sprintSym) {
            setSprinting = (void(*)(void*, bool))sprintSym;
            LOGI("Found sprinting function!");
        } else {
            LOGI("Could not find sprinting function symbol.");
        }
    } else {
        LOGI("Failed to open libminecraftpe.so");
    }
}
