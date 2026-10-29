#include "rom.h"
#include <string.h>

static const char *chip_name(uint8_t type, bool *supported)
{
    *supported = true;
    if ((type & 0x0F) < 3) return "none";
    *supported = false;
    switch (type >> 4) {
        case 0x0: return "DSP";
        case 0x1: return "Super FX";
        case 0x2: return "OBC1";
        case 0x3: return "SA-1";
        case 0x4: return "S-DD1";
        case 0x5: return "S-RTC";
        case 0xE: return "other (SPC7110/ST01x/...)";
        case 0xF: return "custom";
        default:  return "unknown";
    }
}

static int score_header(const uint8_t *d, uint32_t len, uint32_t off, int expect_hi)
{
    if (off + 32 > len) return -1;
    const uint8_t *h = d + off;
    int score = 0;
    uint16_t cksum = h[28] | (h[29] << 8);
    uint16_t comp  = h[30] | (h[31] << 8);
    if ((uint16_t)(cksum ^ comp) == 0xFFFF) score += 4;
    uint8_t map = h[21];
    if ((map & 0xE0) == 0x20) score += 2;                 /* 0x2x / 0x3x */
    bool is_hi = (map & 0x01) != 0;                       /* bit0 = HiROM */
    if (is_hi == (expect_hi != 0)) score += 2;
    if (h[23] >= 7 && h[23] <= 13) score += 1;            /* rom size exponent */
    if (h[24] <= 8) score += 1;                           /* ram size exponent */
    for (int i = 0; i < 21; i++)                          /* title mostly printable / SJIS */
        if (h[i] == 0) { score -= 1; break; }
    return score;
}

bool rom_parse(const uint8_t *data, uint32_t len, RomInfo *out, const uint8_t **data_out)
{
    memset(out, 0, sizeof *out);
    if (len % 1024 == 512) { data += 512; len -= 512; }
    if (data_out) *data_out = data;
    out->size = len;

    struct { uint32_t off; int hi; bool ex; } cand[3] = {
        { 0x007FC0, 0, false },
        { 0x00FFC0, 1, false },
        { 0x40FFC0, 1, true  },
    };

    int best = -1, best_score = -1;
    for (int i = 0; i < 3; i++) {
        int s = score_header(data, len, cand[i].off, cand[i].hi);
        if (s > best_score) { best_score = s; best = i; }
    }
    if (best < 0 || best_score < 4) return false;

    const uint8_t *h = data + cand[best].off;
    out->header_off = cand[best].off;
    out->hirom   = cand[best].hi != 0;
    out->exhirom = cand[best].ex;
    memcpy(out->title, h, 21);
    out->title[21] = 0;
    out->map_mode  = h[21];
    out->cart_type = h[22];
    out->fastrom   = (h[21] & 0x10) != 0;
    out->sram_kb   = h[24] ? (1u << h[24]) : 0;
    uint16_t cksum = h[28] | (h[29] << 8);
    uint16_t comp  = h[30] | (h[31] << 8);
    out->checksum_ok = (uint16_t)(cksum ^ comp) == 0xFFFF;
    out->chip = chip_name(h[22], &out->supported);
    return true;
}
