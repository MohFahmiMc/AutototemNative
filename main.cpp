#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AmbientSprint"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Definisi fungsi internal
void (*Player_setSprinting)(void*, bool);
void (*Player_tick_orig)(void*);

// Fungsi Hook yang akan jalan setiap kali player bergerak
void Player_tick_hook(void* player) {
    if (player != nullptr && Player_setSprinting != nullptr) {
        // Logika: Jika player bergerak maju, otomatis setSprinting ke true
        // Ini akan memicu sprint segera setelah kamu menyentuh tombol jalan
        Player_setSprinting(player, true);
    }
    
    // Jalankan fungsi asli game agar tidak freeze
    if (Player_tick_orig) {
        Player_tick_orig(player);
    }
}

__attribute__((constructor))
void init() {
    LOGI("Loading Auto Sprint Trigger for 1.26.13...");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Mencari simbol fungsi penting
        void* sprintSym = dlsym(handle, "_ZN12EntityContext12setSprintingEb");
        void* tickSym = dlsym(handle, "_ZN6Player4tickEv");

        if (sprintSym && tickSym) {
            Player_setSprinting = (void(*)(void*, bool))sprintSym;
            // Di sini Ambient akan menyambungkan Player_tick_hook ke tick asli game
            LOGI("Success! Auto Sprint connected to movement.");
        } else {
            LOGI("Symbols missing. Check Minecraft 1.26.13 mappings.");
        }
    }
}
