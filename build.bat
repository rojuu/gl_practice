@echo off
SETLOCAL
set TARGET=x64

set SDL_INC=%SDL2%\include
set SDL_LIB=%SDL2%\lib\%TARGET%
set GLEW=C:\glew-2.0.0
set GLEW_BIN=%GLEW%\bin\Release\%TARGET%
set GLEW_INC=%GLEW%\include
set GLEW_LIB=%GLEW%\lib\Release\%TARGET%

set CommonCompilerFlags=/Zi /Od /EHsc /nologo /FC /I%SDL_INC% /I%GLEW_INC%

set CommonLinkerFlags=/DEBUG /LIBPATH:%SDL_LIB% /LIBPATH:%GLEW_LIB% SDL2.lib SDL2main.lib glew32.lib opengl32.lib

if not exist bin (
    mkdir bin
)
pushd bin

if not exist SDL2.dll (
    robocopy %SDL_LIB% . SDL2.dll
)
if not exist glew32.dll (
    robocopy %GLEW_BIN% . glew32.dll
)

cl %CommonCompilerFlags% ..\src\*.cpp /link /subsystem:console %CommonLinkerFlags% /out:gl_prac.exe
popd