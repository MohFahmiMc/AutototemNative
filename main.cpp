#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <vector>

// Definisi Struktur untuk ScreenContext (untuk Rendering)
class ScreenContext {
public:
    char filler[0x100]; // Dummy size
};

typedef void*(*t_getArmor)(void*, int);
typedef int(*t_getDmg)(void*);
typedef int(*t_getMaxDmg)(void*);
typedef void(*t_drawText)(void*, ScreenContext*, std::string const&, float, float, float, float, float, float);

t_getArmor _getArmor = nullptr;
t_getDmg _getDmg = nullptr;
t_getMaxDmg _getMaxDmg = nullptr;
t_drawText _drawText = nullptr; // Fungsi untuk menggambar teks di layar

void (*ScreenContext_addLayer_orig)(ScreenContext*);

// Fungsi Hook untuk Rendering di Layar
void ScreenContext_addLayer_hook(ScreenContext* ctx) {
    if (ctx != nullptr && _getArmor && _drawText) {
        
        // Kita perlu mencari pointer player. Ini bagian tersulit tanpa SDK asli.
        // Kita pakai trik dlsym untuk mencari pointer LocalPlayer yang sedang aktif.
        void* player = dlsym(RTLD_DEFAULT, "_ZN11LocalPlayer11getInstanceEv");
        if (player) {
            void* lp = ((void*(*)())player)(); // Panggil getInstance()

            if (lp) {
                float x = 10.0f; // Posisi X (10 pixel dari kiri)
                float y = 150.0f; // Posisi Y (150 pixel dari atas - di atas Hotbar)
                float fontSize = 1.0f; // Ukuran Font

                // Loop 4 kali (Helmet, Chest, Legs, Boots)
                for (int i = 0; i < 4; i++) {
                    void* stack = _getArmor(lp, i);
                    if (stack != nullptr) {
                        int max = _getMaxDmg(stack);
                        if (max > 0) {
                            int val = max - _getDmg(stack);
                            
                            std::string col = "§a";
                            if (val < (max * 0.25)) col = "§c";
                            else if (val < (max * 0.5)) col = "§e";

                            std::string text = col + std::to_string(val);

                            // Gambar Teks di Layar
                            // (ctx, teks, x, y, r, g, b, a, font_size)
                            _drawText(nullptr, ctx, text, x, y, 1.0f, 1.0f, 1.0f, fontSize);

                            // Turunkan posisi Y untuk slot armor berikutnya
                            y += 15.0f;
                        }
                    }
                }
            }
        }
    }

    if (ScreenContext_addLayer_orig) {
        ScreenContext_addLayer_orig(ctx);
    }
}

__attribute__((constructor))
void init() {
    void* h = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (h) {
        // Fungsi Inti Armor
        _getArmor = (t_getArmor)dlsym(h, "_ZNK4Player8getArmorEi");
        _getDmg = (t_getDmg)dlsym(h, "_ZNK12ItemStackBase14getDamageValueEv");
        _getMaxDmg = (t_getMaxDmg)dlsym(h, "_ZNK12ItemStackBase12getMaxDamageEv");

        // Fungsi Inti Rendering (Ini sering berubah antar versi MCPE)
        // Kita pakai simbol drawText paling standar di MCPE 1.26
        _drawText = (t_drawText)dlsym(h, "_ZN25UIRendererWrapper_UIRenderer8drawTextEPN17MinecraftRenderer13ScreenContextERKSsffffffM7Font_FontE");

        // Fungsi Hook untuk Render Layer (Memasukkan HUD kita ke layar)
        void* layer_sym = dlsym(h, "_ZN13ScreenContext8addLayerEv");
        if (layer_sym) {
            ScreenContext_addLayer_orig = (void(*)(ScreenContext*))layer_sym;
        }
    }
}
