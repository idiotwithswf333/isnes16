# dsisnes (skeleton)

DSi-only SNES emulator for real hardware, launched via Unlaunch. Output: `dsisnes.nds`.

Put ROMs in `sd:/roms` (or the SD root). Stage 1 only boots, loads the ROM into RAM
and prints its header; CPU/PPU/APU cores come next.

Build: push to GitHub, download the `dsisnes-nds` artifact from the Actions run.
