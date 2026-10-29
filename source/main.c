/* dsisnes - DSi-only SNES emulator (skeleton)
 * Stage 1: boot in DSi mode, 133 MHz, SD browser, load ROM into RAM, show header.
 * The CPU/PPU/APU cores are not wired in yet. */

#include <nds.h>
#include <fat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "rom.h"

#define MAX_FILES   256
#define NAME_LEN    96
#define MAX_ROM     (8u * 1024 * 1024)   /* 48 Mbit ExHiROM is 6 MB; 8 MB cap */
#define ROM_DIR     "sd:/roms"

static char files[MAX_FILES][NAME_LEN];
static int  file_count;

static void wait_key(u32 mask)
{
    for (;;) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & mask) return;
    }
}

static void fatal(const char *msg)
{
    consoleClear();
    iprintf("\x1b[31mError\x1b[39m\n\n%s\n\nPress START to exit.\n", msg);
    wait_key(KEY_START);
    exit(0);
}

static bool has_rom_ext(const char *n)
{
    const char *dot = strrchr(n, '.');
    if (!dot) return false;
    return !strcasecmp(dot, ".sfc") || !strcasecmp(dot, ".smc") ||
           !strcasecmp(dot, ".swc") || !strcasecmp(dot, ".fig");
}

static int cmp_names(const void *a, const void *b)
{
    return strcasecmp((const char *)a, (const char *)b);
}

static const char *scan_dir(void)
{
    const char *dir = ROM_DIR;
    DIR *d = opendir(dir);
    if (!d) { dir = "sd:/"; d = opendir(dir); }
    if (!d) return NULL;

    file_count = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && file_count < MAX_FILES) {
        if (e->d_type == DT_DIR) continue;
        if (!has_rom_ext(e->d_name)) continue;
        strncpy(files[file_count], e->d_name, NAME_LEN - 1);
        files[file_count][NAME_LEN - 1] = 0;
        file_count++;
    }
    closedir(d);
    qsort(files, file_count, NAME_LEN, cmp_names);
    return dir;
}

static int pick_rom(const char *dir)
{
    int sel = 0, top = 0;
    const int rows = 20;

    for (;;) {
        consoleClear();
        iprintf("DSi SNES - select ROM\n%s\n\n", dir);
        if (file_count == 0) {
            iprintf("No .sfc/.smc files found.\nPut ROMs in sd:/roms\n");
        }
        if (sel < top) top = sel;
        if (sel >= top + rows) top = sel - rows + 1;
        for (int i = top; i < file_count && i < top + rows; i++) {
            char line[32];
            strncpy(line, files[i], 30);
            line[30] = 0;
            iprintf("%c %s\n", i == sel ? '>' : ' ', line);
        }
        iprintf("\n[A] load  [START] exit");

        for (;;) {
            swiWaitForVBlank();
            scanKeys();
            u32 k = keysDownRepeat();
            if (keysDown() & KEY_START) exit(0);
            if (file_count == 0) continue;
            if (k & KEY_DOWN) { sel = (sel + 1) % file_count; break; }
            if (k & KEY_UP)   { sel = (sel + file_count - 1) % file_count; break; }
            if (keysDown() & KEY_A) return sel;
        }
    }
}

static u8 *load_file(const char *path, u32 *len_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0 || (u32)n > MAX_ROM) { fclose(f); return NULL; }
    u8 *buf = (u8 *)malloc((size_t)n);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *len_out = (u32)n;
    return buf;
}

int main(void)
{
    consoleDemoInit();   /* console on the top screen */

    if (!isDSiMode()) {
        fatal("Not running in DSi mode.\n\nThis emulator needs the DSi's\n133 MHz CPU and 16 MB RAM.\nCheck the header/launch method.");
    }

    bool fast = setCpuClock(true);   /* 133 MHz ARM9 */

    if (!fatInitDefault()) {
        fatal("Could not mount the SD card.");
    }

    const char *dir = scan_dir();
    if (!dir) fatal("Could not open sd:/roms or sd:/.");

    int sel = pick_rom(dir);

    char path[160];
    snprintf(path, sizeof path, "%s/%s", dir[strlen(dir) - 1] == '/' ? "sd:" : dir, files[sel]);

    consoleClear();
    iprintf("Loading %s ...\n", files[sel]);

    u32 len = 0;
    u8 *raw = load_file(path, &len);
    if (!raw) fatal("Could not read ROM\n(too big or out of memory).");

    RomInfo info;
    const u8 *rom = NULL;
    if (!rom_parse(raw, len, &info, &rom)) {
        fatal("No valid SNES header found.");
    }

    consoleClear();
    iprintf("DSi SNES (skeleton)\n\n");
    iprintf("CPU: %s\n", fast ? "133 MHz" : "67 MHz");
    iprintf("Title: %.21s\n", info.title);
    iprintf("Size: %lu KB\n", (unsigned long)(info.size / 1024));
    iprintf("Map: %s%s  (0x%02X)\n",
            info.exhirom ? "ExHiROM" : (info.hirom ? "HiROM" : "LoROM"),
            info.fastrom ? " Fast" : "", info.map_mode);
    iprintf("SRAM: %lu KB\n", (unsigned long)info.sram_kb);
    iprintf("Chip: %s\n", info.chip);
    iprintf("Checksum: %s\n\n", info.checksum_ok ? "OK" : "BAD");

    if (!info.supported) {
        iprintf("Needs a coprocessor that is\nnot supported yet.\n\n");
    }
    iprintf("ROM loaded into RAM.\nCPU/PPU/APU cores not wired\nin yet.\n\n");
    iprintf("[START] exit");

    wait_key(KEY_START);
    free(raw);
    exit(0);
    return 0;
}
