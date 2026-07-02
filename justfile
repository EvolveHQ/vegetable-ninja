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

# serve web/ at http://localhost:8377 in the background (survives shell exit)
up:
    just down; $p = Start-Process python -ArgumentList '-m','http.server','8377','--directory','web' -WindowStyle Hidden -PassThru; $p.Id | Out-File .server.pid; Write-Host "serving http://localhost:8377 (pid $($p.Id))"

# stop the background web server
down:
    if (Test-Path .server.pid) { $srv = Get-Content .server.pid; Stop-Process -Id $srv -Force -ErrorAction SilentlyContinue; Remove-Item .server.pid; Write-Host "stopped pid $srv" } else { Write-Host "server not running" }
