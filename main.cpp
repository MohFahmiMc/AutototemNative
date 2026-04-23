#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <string>
#include <vector>

#define LOG_TAG "AmbientArmorHUD"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Definisi fungsi Minecraft
void* (*Player_getArmor)(void*, int) = nullptr;
int (*ItemStack_getDamageValue)(void*) = nullptr;
int (*ItemStack_getMaxDamage)(void*) = nullptr;
void* (*MinecraftUIRenderContext_drawText)(void*, void const*, std::string const&, float const*, float, float, float const*, float) = nullptr;
void* (*Font_getFont)(void*, std::string const&) = nullptr;

// Data Armor yang akan dirender
struct ArmorDurability {
    int current;
    int max;
    float color[4];
};
std::vector<ArmorDurability> armorData;

// Hook Screen::render (Inilah jantung overlay grafis!)
void (*Screen_render_orig)(void*, void*, int, int, float) = nullptr;
void Screen_render_hook(void* screen, void* ctx, int mx, int my, float a) {
    if (Screen_render_orig) Screen_render_orig(screen, ctx, mx, my, a);

    if (ctx && !armorData.empty() && MinecraftUIRenderContext_drawText && Font_getFont) {
        // Alamat Font Minecraft
        void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
        void* font = Font_getFont(handle, "minecraft_font"); 

        if (font) {
            float yPos = 400.0f; // Koordinat Y di atas hotbar
            for (const auto& armor : armorData) {
                if (armor.max > 0) {
                    float durabilityPercent = (float)armor.current / (float)armor.max;
                    std::string text = std::to_string(armor.current) + "/" + std::to_string(armor.max);
                    
                    // Koordinat X (Tengah layar), Posisi Y, Skala, Warna[4]
                    float pos[2] = {100.0f, yPos}; 
                    
                    // TENTUKAN WARNA: Hijau (>50%), Kuning (>25%), Merah (<25%)
                    float red[4] = {1.0f, 0.0f, 0.0f, 1.0f};
                    float yellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
                    float green[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                    float* finalColor = green;
                    
                    if (durabilityPercent < 0.25f) finalColor = red;
                    else if (durabilityPercent < 0.50f) finalColor = yellow;

                    MinecraftUIRenderContext_drawText(ctx, font, text, pos, 0.5f, 1.0f, finalColor, 1.0f);
                    yPos -= 10.0f; // Geser ke atas buat armor selanjutnya
                }
            }
        }
    }
}

// Hook LocalPlayer::normalTick buat update data armor
void (*LocalPlayer_normalTick_orig)(void*) = nullptr;
void LocalPlayer_normalTick_hook(void* player) {
    if (player != nullptr && Player_getArmor && ItemStack_getDamageValue && ItemStack_getMaxDamage) {
        armorData.clear();
        for (int i = 0; i < 4; i++) {
            void* armorItem = Player_getArmor(player, i);
            if (armorItem != nullptr) {
                int curDamage = ItemStack_getDamageValue(armorItem);
                int maxDamage = ItemStack_getMaxDamage(armorItem);
                armorData.push_back({maxDamage - curDamage, maxDamage});
            }
        }
    }
    if (LocalPlayer_normalTick_orig) LocalPlayer_normalTick_orig(player);
}

__attribute__((constructor))
void init() {
    LOGI("=== ARMOR HUD OVERLAY LOADED ===");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (handle) {
        // Simbol v1.26.13.1
        void* armorSym = dlsym(handle, "_ZNK4Player8getArmorEi");
        void* dmgSym = dlsym(handle, "_ZNK12ItemStackBase14getDamageValueEv");
        void* maxDmgSym = dlsym(handle, "_ZNK12ItemStackBase12getMaxDamageEv");
        void* renderSym = dlsym(handle, "_ZN11HudElements6renderER22MinecraftUIRenderContextRi"); 
        void* drawSym = dlsym(handle, "_ZNK24MinecraftUIRenderContext8drawTextERK4FontRK6Vec2iRfRSK_S3_RK5Colorf");
        void* fontSym = dlsym(handle, "_ZN4Font13getFontHandleERKSs");
        void* tickSym = dlsym(handle, "_ZN11LocalPlayer10normalTickEv");

        if (armorSym && dmgSym && maxDmgSym && renderSym && drawSym && tickSym) {
            Player_getArmor = (void*(*)(void*, int))armorSym;
            ItemStack_getDamageValue = (int(*)(void*))dmgSym;
            ItemStack_getMaxDamage = (int(*)(void*))maxDmgSym;
            MinecraftUIRenderContext_drawText = (void*(*)(void*, void const*, std::string const&, float const*, float, float, float const*, float))drawSym;
            LocalPlayer_normalTick_orig = (void(*)(void*))tickSym;
            // Di sini kitsuri akan otomatis meng-hook renderSym dengan Screen_render_hook
            LOGI("All Vision Symbols Hooked!");
        }
    }
}
