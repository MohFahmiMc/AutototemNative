#include <filesystem>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <unordered_map>
#include <cstdio>
#include <dlfcn.h>

#include "memscan.h"
#include "inlinehook.h"

typedef void*(*t_getArmor)(void*, int);
typedef int(*t_getDmg)(void*);
typedef int(*t_getMaxDmg)(void*);
typedef void(*t_displayMsg)(void*, std::string const&);

t_getArmor _getArmor = nullptr;
t_getDmg _getDmg = nullptr;
t_getMaxDmg _getMaxDmg = nullptr;
t_displayMsg _displayMsg = nullptr;

void (*original_baseTick)(void*);

nlohmann::json outputJson;
std::string dirPath = "";
std::string filePath = "";

bool testDirWritable(const std::string &dir) {
    std::error_code _;
    std::filesystem::create_directories(dir, _);
    std::string testFile = dir + "._perm_test";
    std::ofstream ofs(testFile);
    bool ok = ofs.is_open();
    ofs.close();
    if (ok) std::filesystem::remove(testFile, _);
    return ok;
}

std::string getConfigDir() {
    std::string base = "/sdcard/Android/media/io.kitsuri.mayape/modules/ArmorHUD/";
    if (testDirWritable(base)) return base;
    return "/sdcard/games/ArmorHUD/";
}

void hooked_baseTick(void* p) {
    if (p != nullptr && _getArmor && _displayMsg) {
        std::string hudText = "§l§fArmor: ";
        bool hasArmor = false;
        for (int i = 0; i < 4; i++) {
            void* stack = _getArmor(p, i);
            if (stack != nullptr) {
                int max = _getMaxDmg(stack);
                if (max > 0 && max < 5000) {
                    int val = max - _getDmg(stack);
                    std::string col = (val < max * 0.25) ? "§c" : (val < max * 0.5 ? "§e" : "§a");
                    hudText += col + std::to_string(val) + " §f| ";
                    hasArmor = true;
                }
            }
        }
        if (hasArmor) _displayMsg(p, hudText);
    }
    if (original_baseTick) original_baseTick(p);
}

hook_handle* g_hook = nullptr;

void hookArmor() {
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) return;

    _getArmor = (t_getArmor)dlsym(handle, "_ZNK4Player8getArmorEi");
    _getDmg = (t_getDmg)dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
    _getMaxDmg = (t_getMaxDmg)dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
    _displayMsg = (t_displayMsg)dlsym(handle, "_ZN11LocalPlayer20displayClientMessageERKSs");

    const char* patterns[] = {
        "?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 FD 03 00 91 ?? ?? ?? D1 ?? ?? ?? D5 FA 03 00 AA F5 03 07 AA",
        "?? ?? ?? D1 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? A9 ?? ?? ?? 91 ?? ?? ?? F9 ?? ?? ?? D5 FB 03 00 AA ?? ?? ?? F9 F5 03 07 AA"
    };

    void* func = nullptr;
    for (const char* pattern : patterns) {
        sigscan_handle* scanner = sigscan_setup(pattern, "libminecraftpe.so", GPWN_SIGSCAN_XMEM);
        if (scanner) {
            func = get_sigscan_result(scanner);
            sigscan_cleanup(scanner);
            if (func && func != (void*)-1) break;
        }
    }

    if (func && func != (void*)-1) {
        g_hook = hook_addr(func, (void*)hooked_baseTick, (void**)&original_baseTick, GPWN_AARCH64_MICROHOOK);
    }
}

__attribute__((constructor))
void StartUp() {
    dirPath = getConfigDir();
    hookArmor();
}

__attribute__((destructor))
void Shutdown() {
    if (g_hook) rm_hook(g_hook);
}
