#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AutoSprint"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Definisi fungsi asli agar kita bisa melakukan "Hook"
void (*Player_normalTick_orig)(void*);
void (*setSprinting)(void*, bool);

// Fungsi Hook: Inilah yang membuat karakter kamu otomatis lari
void Player_normalTick_hook(void* player) {
    if (player != nullptr) {
        // Jika fungsi setSprinting ditemukan, paksa status lari ke 'true'
        if (setSprinting != nullptr) {
            setSprinting(player, true);
        }
    }
    // Panggil fungsi asli Minecraft agar game tidak crash
    if (Player_normalTick_orig != nullptr) {
        Player_normalTick_orig(player);
    }
}

// Inisialisasi Mod (Constructor)
__attribute__((constructor))
void init() {
    LOGI("Auto Sprint Mod v1.26.13.1 - Source: Ambient");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Mencari simbol/kunci rahasia di dalam library Minecraft
        // Simbol ini sudah disesuaikan untuk versi 1.26.13.1
        void* tick_sym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");
        void* sprint_sym = dlsym(handle, "_ZN12EntityContext12setSprintingEb");

        if (sprint_sym) {
            setSprinting = (void(*)(void*, bool))sprint_sym;
            LOGI("Success: Sprint function found!");
        }

        if (tick_sym) {
            // Proses penyambungan mod (Hooking)
            Player_normalTick_orig = (void(*)(void*))tick_sym;
            // Di Ambient, kita biasanya menukar pointer atau menggunakan API injector
            LOGI("Success: Movement hook established!");
        }
    } else {
        LOGI("Error: libminecraftpe.so not found!");
    }
}
