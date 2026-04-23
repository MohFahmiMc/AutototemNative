#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "AmbientAutoSprint"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Function pointer for setting the sprinting state
void (*Player_setSprinting)(void*, bool);

__attribute__((constructor))
void init() {
    LOGI("Loading Auto Sprint Mod for version 1.26.13...");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Looking for the sprinting symbol in Minecraft's memory
        void* symbol = dlsym(handle, "_ZN12EntityContext12setSprintingEb");
        
        if (symbol) {
            Player_setSprinting = (void(*)(void*, bool))symbol;
            LOGI("Successfully injected Auto Sprint feature!");
        } else {
            LOGI("Error: Could not find setSprinting symbol. Please check mappings.");
        }
    } else {
        LOGI("Error: Failed to open libminecraftpe.so");
    }
}
