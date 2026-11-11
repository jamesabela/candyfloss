CANDYFLOSS TYCOON - FLOPPY EDITION
==================================

A single-file native Win32 C version designed for a 1.44 MB executable limit.
All graphics are drawn with Windows GDI. There are no external image, audio,
font or data files.

BUILDING ON WINDOWS
-------------------

1. Install MSYS2 or another MinGW-w64 distribution.
2. Open this folder in a terminal that provides gcc.
3. Run build.bat.
4. The result will be CandyflossTycoon.exe.

Manual command:

gcc candyfloss.c -o CandyflossTycoon.exe -std=c11 -Os -s ^
  -ffunction-sections -fdata-sections -Wl,--gc-sections ^
  -mwindows -lgdi32 -lmsimg32 -lm

CONTROLS
--------

- Use the mouse to select options, move sliders and press buttons.
- Press Escape to return to the title screen.
- On the title screen, press Escape to quit.

GAMEPLAY
--------

Each day:

- Check the weather and expected crowd.
- Choose how much candyfloss to make.
- Set a selling price.
- Buy advertising signs.
- Open the stall and review the financial report.

The game includes weather effects, random events, animated procedural graphics,
reputation, several difficulty levels and 7, 10 or 14-day games.

SIZE NOTES
----------

- The executable contains no assets.
- The release build strips symbols and removes unused sections.
- Do not add a large Windows icon if the 1.44 MB limit is strict.
- UPX may reduce the result further, but check the competition rules first.
