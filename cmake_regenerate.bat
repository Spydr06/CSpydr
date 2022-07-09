@echo off
:: A small script to delete the CMake cache and regenerate the project

PUSHD %~dp0

IF EXIST CMakeCache.txt DEL /F CMakeCache.txt
IF EXIST cmake_install.cmake DEL /F cmake_install.cmake
IF EXIST bin DEL /F BIN
IF EXIST .cache DEL /F .cache

cmake . %*

POPD