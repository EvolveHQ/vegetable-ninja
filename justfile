# Vegetable Ninja tasks
set shell := ["pwsh", "-NoProfile", "-Command"]

# list available recipes
default:
    @just --list

# build the native Windows exe
build:
    ./build.ps1 native

# build the WebAssembly bundle into web/
web:
    ./build.ps1 web

# build both targets
all:
    ./build.ps1 all

# build and launch the native game
run: build
    Start-Process ./VegetableNinja.exe

# run the scripted native self-test (saves selftest*.png)
selftest: build
    Start-Process ./VegetableNinja.exe -ArgumentList '--selftest' -Wait; Get-ChildItem selftest*.png | Select-Object Name

# assemble the deployable static site (for https://vegetable.ninja) into dist/
# (injects the consent-gated Google Analytics banner - dist only, not dev)
dist: web
    $keep = @('publish.ps1','server.js') | Where-Object { Test-Path "dist\$_" }; foreach ($k in $keep) { Copy-Item "dist\$k" "$env:TEMP\vn-$k" }; Remove-Item dist -Recurse -Force -ErrorAction SilentlyContinue; New-Item -ItemType Directory dist | Out-Null; Copy-Item web\* dist\; Copy-Item favicon.svg dist\; $v = ''; try { $v = (git describe --tags --always --dirty 2>$null) } catch {}; if (-not $v) { $v = 'dev' }; $h = Get-Content dist\index.html -Raw; $s = Get-Content analytics-snippet.html -Raw; $h = $h.Replace('</body>', $s + '</body>').Replace('id="credit">created', 'id="credit">' + $v + ' &#183; created'); Set-Content dist\index.html $h -NoNewline; foreach ($k in $keep) { Copy-Item "$env:TEMP\vn-$k" "dist\$k" }; Get-ChildItem dist | Select-Object Name, Length

# serve web/ at http://localhost:8377 in the background (survives shell exit)
up:
    just down; $p = Start-Process python -ArgumentList '-m','http.server','8377','--directory','web' -WindowStyle Hidden -PassThru; $p.Id | Out-File .server.pid; Write-Host "serving http://localhost:8377 (pid $($p.Id))"

# stop the background web server
down:
    if (Test-Path .server.pid) { $srv = Get-Content .server.pid; Stop-Process -Id $srv -Force -ErrorAction SilentlyContinue; Remove-Item .server.pid; Write-Host "stopped pid $srv" } else { Write-Host "server not running" }
