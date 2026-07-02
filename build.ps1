# Vegetable Ninja build script.
#   .\build.ps1          native exe only
#   .\build.ps1 web      wasm build only
#   .\build.ps1 all      both
param([string]$Target = "native")

$ErrorActionPreference = "Stop"

if ($Target -eq "native" -or $Target -eq "all") {
    $rl = ".\vendor\raylib-5.5_win64_mingw-w64"
    gcc main.c levels.c progress.c -o VegetableNinja.exe -O2 -Wall -Wextra `
        "-I$rl\include" "-L$rl\lib" `
        -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -static
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "native OK -> VegetableNinja.exe"
}

if ($Target -eq "web" -or $Target -eq "all") {
    & .\vendor\emsdk\emsdk_env.ps1 | Out-Null
    # raylib is built from source with the same emsdk (see README) — the
    # prebuilt release lib was made with an older emsdk and its audio JS
    # glue is incompatible with the current emscripten runtime.
    $rl = ".\vendor\raylib-src\raylib-5.5\src"
    New-Item -ItemType Directory -Force web | Out-Null
    emcc main.c levels.c progress.c -o web\index.html -O2 -DPLATFORM_WEB `
        "-I$rl" "$rl\libraylib.web.a" `
        -sUSE_GLFW=3 -sINITIAL_MEMORY=134217728 `
        "-sEXPORTED_RUNTIME_METHODS=HEAPF32,HEAP32,HEAPU8" `
        --shell-file shell.html
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "web OK -> web\index.html (serve the web\ folder over http)"
}
