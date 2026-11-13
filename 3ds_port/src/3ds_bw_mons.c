/*
 * The battlers' Black 2 / White 2 pictures, native side (3ds_bw_mons.h): the
 * files read from the card, one per battler slot, and their frames decoded
 * for the compositor. A file is read once, when the battler's sprite is
 * loaded, and kept until another takes its slot.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "3ds_log.h"
#include "3ds_bw_mons.h"

struct CtrBwPic
{
    char path[64];
    unsigned w, h, frames, anchorX2, feetY, colours;
    uint32_t loopTicks;
    uint32_t palette[256];
    uint16_t *ticks;
    uint32_t *offsets;
    const uint8_t *data;
    uint32_t dataSize;
    uint8_t *file;
};

static CtrBwPic sSlots[CTR_BW_BATTLERS];
static bool sHeld[CTR_BW_BATTLERS];

static unsigned U16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t U32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }

bool CtrBwPic_Exists(const char *path)
{
    struct stat st;

    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

void CtrBwPic_Release(unsigned slot)
{
    if (slot >= CTR_BW_BATTLERS || !sHeld[slot]) return;
    free(sSlots[slot].file);
    free(sSlots[slot].ticks);
    free(sSlots[slot].offsets);
    memset(&sSlots[slot], 0, sizeof(sSlots[slot]));
    sHeld[slot] = false;
}

static bool Parse(CtrBwPic *pic, uint8_t *file, long size)
{
    const uint8_t *p = file;
    long need;

    if (size < 16 || memcmp(p, "BWS1", 4)) return false;
    pic->w = U16(p + 4);
    pic->h = U16(p + 6);
    pic->frames = U16(p + 8);
    pic->anchorX2 = U16(p + 10);
    pic->feetY = U16(p + 12);
    pic->colours = U16(p + 14);
    if (!pic->w || !pic->h || pic->w > CTR_BW_MAX_DIM || pic->h > CTR_BW_MAX_DIM || !pic->frames
        || !pic->colours || pic->colours > 256)
        return false;
    need = 16 + 4L * pic->colours + 2L * pic->frames + 4L * (pic->frames + 1);
    if (size < need) return false;
    p += 16;
    for (unsigned i = 0; i < pic->colours; ++i, p += 4)
        pic->palette[i] = i ? U32(p) : 0;
    pic->ticks = malloc(sizeof(uint16_t) * pic->frames);
    pic->offsets = malloc(sizeof(uint32_t) * (pic->frames + 1));
    if (!pic->ticks || !pic->offsets) return false;
    pic->loopTicks = 0;
    for (unsigned i = 0; i < pic->frames; ++i, p += 2)
    {
        pic->ticks[i] = (uint16_t)U16(p);
        if (!pic->ticks[i]) pic->ticks[i] = 1;
        pic->loopTicks += pic->ticks[i];
    }
    for (unsigned i = 0; i <= pic->frames; ++i, p += 4)
        pic->offsets[i] = U32(p);
    pic->data = p;
    pic->dataSize = (uint32_t)(size - need);
    for (unsigned i = 0; i < pic->frames; ++i)
        if (pic->offsets[i] > pic->offsets[i + 1] || pic->offsets[i + 1] > pic->dataSize) return false;
    return true;
}

const CtrBwPic *CtrBwPic_Get(unsigned slot, const char *path)
{
    CtrBwPic *pic;
    FILE *f;
    long size;
    uint8_t *file;

    if (slot >= CTR_BW_BATTLERS || !path || !path[0]) return NULL;
    pic = &sSlots[slot];
    if (sHeld[slot] && !strcmp(pic->path, path)) return pic;
    /* Another battler may hold the same picture (a mirror match): read it
     * again all the same, the slots stay apart. */
    CtrBwPic_Release(slot);
    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    file = size > 0 && size < 8L * 1024 * 1024 ? malloc((size_t)size) : NULL;
    if (!file || fread(file, 1, (size_t)size, f) != (size_t)size)
    {
        fclose(f);
        free(file);
        CtrLog_Write(CTR_LOG_ERROR, "bw: cannot read %s", path);
        return NULL;
    }
    fclose(f);
    pic->file = file;
    sHeld[slot] = true;
    if (!Parse(pic, file, size))
    {
        CtrLog_Write(CTR_LOG_ERROR, "bw: %s is not a BWS1 picture", path);
        CtrBwPic_Release(slot);
        return NULL;
    }
    snprintf(pic->path, sizeof(pic->path), "%s", path);
    return pic;
}

void CtrBwPic_Size(const CtrBwPic *pic, unsigned *w, unsigned *h, unsigned *anchorX2, unsigned *feetY)
{
    *w = pic->w;
    *h = pic->h;
    *anchorX2 = pic->anchorX2;
    *feetY = pic->feetY;
}

unsigned CtrBwPic_FrameAt(const CtrBwPic *pic, uint32_t tick)
{
    tick %= pic->loopTicks;
    for (unsigned i = 0; i < pic->frames; ++i)
    {
        if (tick < pic->ticks[i]) return i;
        tick -= pic->ticks[i];
    }
    return 0;
}

void CtrBwPic_Decode(const CtrBwPic *pic, unsigned frame, uint32_t *out, unsigned stride)
{
    const uint8_t *p = pic->data + pic->offsets[frame], *end = pic->data + pic->offsets[frame + 1];
    unsigned total = pic->w * pic->h, n = 0;

    while (n < total && p < end)
    {
        unsigned c = *p++;

        if (c < 128)
        {
            for (unsigned k = 0; k <= c && n < total && p < end; ++k, ++n)
                out[(n / pic->w) * stride + n % pic->w] = pic->palette[*p++];
        }
        else if (p < end)
        {
            uint32_t colour = pic->palette[*p++];

            for (unsigned k = 0; k < c - 126 && n < total; ++k, ++n)
                out[(n / pic->w) * stride + n % pic->w] = colour;
        }
    }
    for (; n < total; ++n)
        out[(n / pic->w) * stride + n % pic->w] = 0;
}
