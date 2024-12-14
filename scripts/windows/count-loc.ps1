Clear-Host

tokei                                                                                           `
    "./scripts/windows"                                                                         `
    "./docs"                                                                                    `
                                                                                                `
    "./tools/3ds-max-plugin/include"                                                            `
    --exclude "tools/3ds-max-plugin/include/mikkt/mikktspace.h"                                 `
                                                                                                `
    "./tools/3ds-max-plugin/sources"                                                            `
    --exclude "tools/3ds-max-plugin/sources/mikkt/mikktspace.c"                                 `
                                                                                                `
    "./tools/editor/hlsl"                                                                       `
    "./tools/editor/include"                                                                    `
    --exclude "tools/editor/include/WinPixEventRuntime/*"                                       `
    "./tools/editor/scripts"                                                                    `
    "./tools/editor/src"                                                                        `
                                                                                                `
    "./tools/html-validator/include"                                                            `
    "./tools/html-validator/sources"                                                            `
                                                                                                `
    "./app/src/main/cpp/include"                                                                `
    --exclude "app/src/main/cpp/include/freetype2/*"                                            `
    --exclude "app/src/main/cpp/include/lua/*"                                                  `
    --exclude "app/src/main/cpp/include/ogg/*"                                                  `
    --exclude "app/src/main/cpp/include/stb/*"                                                  `
    --exclude "app/src/main/cpp/include/vorbis/*"                                               `
    "./app/src/main/cpp/sources"                                                                `
    "./app/src/main/hlsl"                                                                       `
    --exclude "app/src/main/hlsl/disassm/*"                                                     `
    --exclude "app/src/main/hlsl/validation/*"                                                  `
    "./app/src/main/kotlin/com/goshidoInc/androidVulkan"                                        `
    "./app/src/main/assets/pbr/assets/Props/experimental/character-sandbox/scripts"             `
    "./app/src/main/assets/pbr/assets/Props/experimental/character-sandbox/bobby/bobby.lua"     `
    "./app/src/main/assets/pbr/assets/Props/experimental/world-1-1"
