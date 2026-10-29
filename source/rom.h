#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char     title[22];
    uint8_t  map_mode;      /* raw map byte from header */
    uint8_t  cart_type;     /* raw cartridge type byte */
    uint32_t header_off;    /* offset of the 32-byte header inside the ROM data */
    uint32_t size;          /* actual data size (copier header stripped) */
    uint32_t sram_kb;
    bool     hirom;
    bool     exhirom;
    bool     fastrom;
    bool     checksum_ok;
    const char *chip;       /* "none" or the coprocessor family */
    bool     supported;     /* false if a coprocessor we do not emulate is needed */
} RomInfo;

/* data/len: ROM image. If len % 1024 == 512 a copier header is assumed and
 * *data_out is advanced past it. Returns false if no plausible header found. */
bool rom_parse(const uint8_t *data, uint32_t len, RomInfo *out, const uint8_t **data_out);

#endif
