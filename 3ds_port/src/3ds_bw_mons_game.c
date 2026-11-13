/*
 * The battlers' Black 2 / White 2 pictures, game side (3ds_bw_mons.h): which
 * file each battler is drawn from, where its feet are, and what its sprite
 * is doing this frame.
 *
 * Files are looked up as the battler's graphics are loaded, never during a
 * frame: CTR_BW_DIR front|back / NNN[form][_f][_s].bin, the national number
 * on three digits, the form as Bulbagarden names it (Unown B-Z, EX, QU;
 * Castform S, R, H; Deoxys S), _f a female's own picture, _s a shiny one. A
 * female without her own picture takes the common one; a form without its
 * own, the plain species'. With none at all, the game's sprite is shown.
 */
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "main.h"
#include "battle.h"
#include "battle_anim.h"
#include "pokemon.h"
#include "sprite.h"
#include "constants/battle.h"
#include "constants/species.h"

#include "3ds_bw_mons.h"

static CtrBwBattler sBattlers[CTR_BW_BATTLERS];
static uint32_t sVersion;

/* The form's part of a file name. */
static const char *FormSuffix(u16 species, u8 form)
{
    static const char *const unown[28] = {
        "", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
        "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "EX", "QU",
    };
    /* CASTFORM_NORMAL, _FIRE (sunny), _WATER (rainy), _ICE (snowy). */
    static const char *const castform[4] = {"", "S", "R", "H"};

    if (species == SPECIES_UNOWN) return form < 28 ? unown[form] : "";
    if (species == SPECIES_CASTFORM) return form < 4 ? castform[form] : "";
    if (species == SPECIES_DEOXYS) return form ? "S" : "";
    return "";
}

static bool Find(char *path, size_t size, const char *side, unsigned dex, const char *form, bool female, bool shiny)
{
    const char *shinyPart = shiny ? "_s" : "";

    if (female)
    {
        snprintf(path, size, CTR_BW_DIR "%s/%03u%s_f%s.bin", side, dex, form, shinyPart);
        if (CtrBwPic_Exists(path)) return true;
    }
    snprintf(path, size, CTR_BW_DIR "%s/%03u%s%s.bin", side, dex, form, shinyPart);
    if (CtrBwPic_Exists(path)) return true;
    if (form[0])
        return Find(path, size, side, dex, "", female, shiny);
    return false;
}

/* The game's own picture: its lowest opaque row and the middle of its
 * opaque columns, from the 4bpp tiles (8x8 of them, a row of tiles at a
 * time) gMonSpritesGfxPtr holds. */
static void MeasurePicture(const u8 *pic, int8_t *feetY, int8_t *centreX2)
{
    int bottom = -1, left = 64, right = -1;

    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
        {
            const u8 *tile = pic + ((y / 8) * 8 + x / 8) * 32;
            u8 byte = tile[(y % 8) * 4 + (x % 8) / 2];
            u8 index = (x & 1) ? byte >> 4 : byte & 15;

            if (!index) continue;
            if (y > bottom) bottom = y;
            if (x < left) left = x;
            if (x > right) right = x;
        }
    if (bottom < 0)
    {
        *feetY = 56;
        *centreX2 = 64;
        return;
    }
    *feetY = (int8_t)bottom;
    *centreX2 = (int8_t)(left + right + 1);
}

void CtrBwMons_Loaded(unsigned char battler, unsigned short species, unsigned char form,
                      bool female, bool shiny, bool back, bool transformed)
{
    CtrBwBattler *b;
    unsigned dex;
    u8 position;
    const u8 *pic;

    if (battler >= CTR_BW_BATTLERS) return;
    b = &sBattlers[battler];
    b->active = false;
    b->clipRows = 64;
    b->transformed = transformed;
    ++sVersion;
    b->version = sVersion;
    dex = species && species < NUM_SPECIES ? SpeciesToNationalPokedexNum(species) : 0;
    if (!dex || dex > 386) return;
    if (!Find(b->path, sizeof(b->path), back ? "back" : "front", dex, FormSuffix(species, form), female, shiny))
        return;
    position = GetBattlerPosition(battler);
    pic = gMonSpritesGfxPtr ? gMonSpritesGfxPtr->sprites.byte[position] : NULL;
    if (pic && species == SPECIES_CASTFORM) pic += (form & 3) * MON_PIC_SIZE;
    if (pic)
        MeasurePicture(pic, &b->feetY, &b->centreX2);
    else
    {
        b->feetY = 56;
        b->centreX2 = 64;
    }
    b->active = true;
}

void CtrBwMons_Clear(unsigned char battler)
{
    if (battler >= CTR_BW_BATTLERS) return;
    sBattlers[battler].active = false;
    sBattlers[battler].version = ++sVersion;
}

void CtrBwMons_Faint(unsigned char battler, int rows)
{
    if (battler >= CTR_BW_BATTLERS) return;
    sBattlers[battler].clipRows = (uint8_t)(rows < 0 ? 0 : rows > 8 ? 64 : rows * 8);
}

void CtrBwMons_Reset(void)
{
    for (unsigned i = 0; i < CTR_BW_BATTLERS; ++i)
        CtrBwMons_Clear((unsigned char)i);
}

const CtrBwBattler *CtrBwMons_Battlers(void)
{
    for (unsigned i = 0; i < CTR_BW_BATTLERS; ++i)
    {
        CtrBwBattler *b = &sBattlers[i];
        const struct Sprite *sprite;

        b->tile = 0xFFFF;
        if (!b->active || !gMain.inBattle || i >= gBattlersCount || gBattlerSpriteIds[i] >= MAX_SPRITES)
            continue;
        sprite = &gSprites[gBattlerSpriteIds[i]];
        /* The battler's Pokémon, not its trainer (the intro, a trainer
         * sliding back in), which takes the same sprite id: the controllers
         * give a Pokémon the battler's palette and number (data[0]). */
        if (!sprite->inUse || sprite->oam.paletteNum != i || sprite->data[0] != (s16)i) continue;
        b->tile = sprite->oam.tileNum;
        b->paused = (gBattleMons[i].status1 & (STATUS1_SLEEP | STATUS1_FREEZE)) != 0;
    }
    return sBattlers;
}
