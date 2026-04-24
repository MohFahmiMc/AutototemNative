set_project("ArmorHUD")
set_version("1.0.0")

set_languages("cxx23")

set_plat("android")
set_arch("arm64-v8a")

add_rules("mode.release")

-- Optimasi agar file .so super kecil dan cepat di Vivo Y12
add_cxflags("-O2", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-flto", "-w", "-fdeclspec", {force = true})
add_ldflags("-Wl,--gc-sections", "-Wl,--strip-all", "-s")

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")
add_requires("nlohmann_json v3.11.3")

target("ArmorHUD")
    set_kind("shared")
    add_packages("nlohmann_json")
    -- Mengikuti struktur folder kamu
    add_files("src/*.cpp", "gamepwnage/*.c")
    add_includedirs("src", "gamepwnage", {public = true})
