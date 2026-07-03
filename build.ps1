# Vegetable Ninja build script.
#   .\build.ps1          native exe only
#   .\build.ps1 web      wasm build only
#   .\build.ps1 all      both
#   .\build.ps1 dist     web build + deployable static site in dist/
param([string]$Target = "native")

$ErrorActionPreference = "Stop"

# Build identity: generated header, single source of truth is git describe.
# See .docflow/adr/0104-git-derived-build-version.md
$ver = ""
try { $ver = (git describe --tags --always --dirty 2>$null) } catch {}
if (-not $ver) { $ver = "dev" }
Set-Content version.h "#define GAME_VERSION `"$ver`"" -NoNewline
Write-Host "version: $ver"

if ($Target -eq "native" -or $Target -eq "all") {
    $rl = ".\vendor\raylib-5.5_win64_mingw-w64"
    gcc main.c levels.c progress.c -o VegetableNinja.exe -O2 -Wall -Wextra `
        "-I$rl\include" "-L$rl\lib" `
        -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -static
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "native OK -> VegetableNinja.exe"
}

if ($Target -eq "web" -or $Target -eq "all" -or $Target -eq "dist") {
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

if ($Target -eq "dist") {
    # Assemble the deployable static site: game bundle + favicon, inject the
    # consent-gated analytics snippet and the build version into the footer.
    # Unversioned deploy files (publish.ps1, server.js) survive the rebuild.
    $keep = @("publish.ps1", "server.js") | Where-Object { Test-Path "dist\$_" }
    foreach ($k in $keep) { Copy-Item "dist\$k" "$env:TEMP\vn-$k" -Force }
    Remove-Item dist -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory dist | Out-Null
    Copy-Item web\* dist\
    Copy-Item favicon.svg dist\
    $h = Get-Content dist\index.html -Raw
    $h = $h.Replace("</body>", (Get-Content analytics-snippet.html -Raw) + "</body>")
    # emcc minifies the shell at -O2, so match both quoted and unquoted markup
    $stamp = "$ver &#183; created"
    $h = $h.Replace('id="credit">created', "id=`"credit`">$stamp").Replace('id=credit>created', "id=credit>$stamp")
    if ($h -notmatch [regex]::Escape($ver)) { throw "version stamp failed to apply" }
    Set-Content dist\index.html $h -NoNewline
    foreach ($k in $keep) { Copy-Item "$env:TEMP\vn-$k" "dist\$k" -Force }
    Write-Host "dist OK ($ver) -> dist\"
    Get-ChildItem dist | Select-Object Name, Length
}
