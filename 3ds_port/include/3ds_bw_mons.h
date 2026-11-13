#ifndef CTR_BW_MONS_H
#define CTR_BW_MONS_H

/*
 * The battlers drawn from Black 2 / White 2's animated sprites (optional).
 *
 * The player converts the sprites on a computer (tools/bw_sprites.py) and
 * copies them to sdmc:/3ds/emerald3ds/bw/. When a battler's sprite is loaded
 * (battle_gfx_sfx_util.c, patch 0083) the game side (3ds_bw_mons_game.c)
 * looks for the file of its species, form, gender and shininess; if there is
 * one, the compositor (3ds_video.c, BwDrawInScene) draws that picture,
 * animated, wherever the GBA would draw the battler's 64x64 sprite - its
 * place, affine matrix (the send-out's growth, the moves' squashes), blend
 * and palette fade all taken from the sprite - and leaves its tiles out.
 * Without the file the game's own sprite is shown, as before.
 *
 * The file (little-endian), "BWS1":
 *   u16 w, h            the picture's size, at most CTR_BW_MAX_DIM
 *   u16 frames
 *   u16 anchorX2        twice the middle of the first frame's opaque columns
 *   u16 feetY           the first frame's lowest opaque row
 *   u16 colours         at most 256; colour 0 is transparent
 *   u32 palette[colours]   0xRRGGBBAA
 *   u16 ticks[frames]      how long each frame shows, in 60 Hz frames
 *   u32 offsets[frames + 1] into the data that follows
 *   data: each frame's w*h colour indices, PackBits (a byte c < 128: c + 1
 *         literal bytes follow; c >= 128: the next byte c - 126 times)
 *
 * SDK-free: the game side and the compositor both include it.
 */
#include <stdbool.h>
#include <stdint.h>

#define CTR_BW_BATTLERS 4
#define CTR_BW_MAX_DIM 256
#define CTR_BW_DIR "sdmc:/3ds/emerald3ds/bw/"

typedef struct
{
    /* Drawn from a picture: the file was found. */
    bool active;
    /* The OAM tile of the battler's sprite (live, CtrBwMons_Battlers). */
    uint16_t tile;
    /* The game's own picture of the battler, in its 64x64 box: its lowest
     * opaque row and twice the middle of its opaque columns. The BW picture
     * is put down on the same feet. */
    int8_t feetY;
    int8_t centreX2;
    /* The rows of the box still shown (64: all): an opponent fainting is
     * erased from the bottom as it sinks (SpriteCB_AnimFaintOpponent). */
    uint8_t clipRows;
    /* Asleep or frozen: the animation holds its frame. */
    bool paused;
    /* Transformed: the game washes its palette towards white. */
    bool transformed;
    /* Changes whenever the picture does. */
    uint32_t version;
    char path[64];
} CtrBwBattler;

/* ---- Game side (3ds_bw_mons_game.c), told by battle_gfx_sfx_util.c. ---- */
/* The battler's sprite graphics were just loaded (gMonSpritesGfxPtr has its
 * picture): species as shown (a transformed one's target), `form` the Unown
 * letter, Castform's form or Deoxys' (0 normal, 1 its Emerald form). */
void CtrBwMons_Loaded(unsigned char battler, unsigned short species, unsigned char form,
                      bool female, bool shiny, bool back, bool transformed);
/* The battler shows something else (the Substitute doll). */
void CtrBwMons_Clear(unsigned char battler);
/* An opponent fainting: `rows` of eight still shown. */
void CtrBwMons_Faint(unsigned char battler, int rows);
/* A new battle: nothing drawn from pictures until loaded again. */
void CtrBwMons_Reset(void);
/* For the compositor: the CTR_BW_BATTLERS battlers, their tiles refreshed. */
const CtrBwBattler *CtrBwMons_Battlers(void);

/* ---- Native side (3ds_bw_mons.c): the files. ---- */
typedef struct CtrBwPic CtrBwPic;
bool CtrBwPic_Exists(const char *path);
/* The picture in `slot` (one per battler), read from `path` if it is not
 * the one held; NULL if it cannot be read. */
const CtrBwPic *CtrBwPic_Get(unsigned slot, const char *path);
void CtrBwPic_Release(unsigned slot);
void CtrBwPic_Size(const CtrBwPic *pic, unsigned *w, unsigned *h, unsigned *anchorX2, unsigned *feetY);
/* The frame shown `tick` 60 Hz frames into the loop. */
unsigned CtrBwPic_FrameAt(const CtrBwPic *pic, uint32_t tick);
/* Frame `frame` as 0xRRGGBBAA texels, `stride` texels a row. */
void CtrBwPic_Decode(const CtrBwPic *pic, unsigned frame, uint32_t *out, unsigned stride);

#endif
