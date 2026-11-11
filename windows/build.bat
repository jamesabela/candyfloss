@echo off
setlocal
where gcc >nul 2>nul
if errorlevel 1 (
  echo MinGW GCC was not found.
  echo Install MSYS2 or MinGW-w64, then run this file again.
  pause
  exit /b 1
)
windres resource.rc -o resource.o
gcc candyfloss.c resource.o -o CandyflossTycoon.exe -std=c11 -finput-charset=UTF-8 -fexec-charset=UTF-16LE -Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -mwindows -lgdi32 -lmsimg32 -lm
if errorlevel 1 (
  echo Build failed.
  pause
  exit /b 1
)
for %%A in (CandyflossTycoon.exe) do echo Built CandyflossTycoon.exe - %%~zA bytes
pause
