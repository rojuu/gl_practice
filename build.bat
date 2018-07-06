@echo off
SETLOCAL
set TARGET=x64

set EXE_NAME=gl_prac.exe

set SDL2=%CD%\libs\SDL2-2.0.5
set SDL_INC=%SDL2%\include
set SDL_LIB=%SDL2%\lib\%TARGET%

set GLEW=%CD%\libs\glew-2.0.0
set GLEW_BIN=%GLEW%\bin\Release\%TARGET%
set GLEW_INC=%GLEW%\include
set GLEW_LIB=%GLEW%\lib\Release\%TARGET%


set ASSIMP=%CD%\libs\Assimp
set ASSIMP_INC=%ASSIMP%\include
set ASSIMP_LIB=%ASSIMP%\lib\x64

set STB_INC=%CD%\libs\stb

set CommonCompilerFlags=-Zi -Od -EHsc -nologo -FC -I%SDL_INC% -I%GLEW_INC% -I%STB_INC% -I%ASSIMP_INC%

set CommonLinkerFlags=-debug -libpath:%SDL_LIB% -libpath:%GLEW_LIB% -libpath:%ASSIMP_LIB% SDL2.lib SDL2main.lib glew32.lib opengl32.lib assimp-vc140-mt.lib

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

cl %CommonCompilerFlags% ..\src\main.cpp /link /subsystem:windows %CommonLinkerFlags% /out:%EXE_NAME%
popd
echo Done
