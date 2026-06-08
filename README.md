# LootMenuKarmaPatch

An FOSE (Fallout Script Extender) plugin designed to dynamically fix an underlying bug in the Fallout 3 "Loot Menu" mod by lStewieAl / Yvileapsis.

## The Issue
When stealing certain items (such as Food or Stimpaks) using the Loot Menu, karma loss and crime detection would not trigger. This caused incompatibilities with mods that intercept crime events, such as my `no_witness_karma`.

The root cause was twofold in the Loot Menu's source code:
1. **Broken Value Calculation**: `GetItemValue()` incorrectly casted `AlchemyItem`s, returning garbage or zero data instead of the actual item value.
2. **Skipped Crime Events**: When taking an item, Loot Menu explicitly checked if `value > 0`. If false, it completely bypassed calling the engine's `SendCrimeEvent` (0x70C030), leaving the game completely unaware that a theft had occurred.

## How it Works
Instead of modifying Loot Menu's source files directly and requiring users to recompile Loot Menu, this mod is a standalone FOSE plugin. When the game loads, it dynamically scans and patches `F3LootMenu.dll` in memory:

- It patches the buggy `AlchemyItem` type-cast in `GetItemValue()`, forcefully routing the logic to the reliable `DYNAMIC_CAST` fallback.
- It scans for the `value > 0` conditional jump instructions right before the `SendCrimeEvent` calls and `NOP`s them out. This forces the crime event to accurately fire for every stolen item, matching vanilla Fallout 3 behavior.

## Support for LootMenuUpdated
This patch has been upgraded to provide **universal support** for both the original Loot Menu mod and the unofficial update `LootMenuUpdated`. It intelligently scans the loaded `F3LootMenu.dll` for both classic (MSVC 2013) and modern (MSVC 2022) assembly instruction patterns. It will seamlessly detect which version of the mod you are using and apply the correct memory patches without requiring separate configurations or downloads.

## Installation
Drop the compiled `LootMenuKarmaPatch.dll` into your Fallout 3 installation folder under:
`Data/FOSE/Plugins/`

Ensure that you have FOSE installed and that the original Loot Menu mod is loaded.

## Compiling
This project uses CMake and requires access to the `stewei_tweaks_src/fose` headers to compile correctly.

From the `loot_menus_patch` directory:
```ps
mkdir build
cd build
cmake .. -A Win32
cmake --build . --config Release
```
The compiled DLL will be located in the `build/Release/` directory.
