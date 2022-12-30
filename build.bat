@echo off
mkdir build
pushd build
cl ../src/main.cpp /nologo /Zi /Fe:Solar_System.exe /I"../dp" /I"../dp/include" ^
/MD /link user32.lib shell32.lib gdi32.lib kernel32.lib ../dp/glfw3.lib opengl32.lib

if %ERRORLEVEL% EQU 0 (
    Solar_System.exe
    cls
)

popd 
