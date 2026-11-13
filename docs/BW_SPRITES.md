# Sprites animados de Black 2 / White 2 nas batalhas

Opcional. Com os arquivos no cartão SD, os Pokémon em batalha (frente,
costas, shiny, formas, fêmeas com sprite próprio) são desenhados com os
sprites animados de Black 2 / White 2 no lugar dos sprites estáticos do
Emerald. Sem os arquivos, nada muda.

## 1. Converter os sprites (no PC)

Requer Python 3 e Pillow (`pip install pillow`).

```
python tools/bw_sprites.py --out bw_out
```

O script lista as quatro categorias do Bulbagarden Archives (frente, costas
e as versões shiny), baixa os APNGs dos Pokémon #001 a #386 para `bw_cache/`
e grava os arquivos convertidos (formato BWS1, `3ds_port/include/3ds_bw_mons.h`)
em `bw_out/front` e `bw_out/back`. Para um teste rápido:
`--only 1-9,25,384`. Se os PNGs já estiverem em `bw_cache/`, use
`--offline`.

## 2. Copiar para o SD

```
sdmc:/3ds/emerald3ds/bw/front/001.bin
sdmc:/3ds/emerald3ds/bw/back/001.bin
...
```

Nomes: `NNN[forma][_f][_s].bin`. A forma segue o Bulbagarden (Unown `B`-`Z`,
`EX`, `QU`; Castform `S`, `R`, `H`; Deoxys `S`), `_f` é o sprite próprio da
fêmea e `_s` o shiny. Sem o arquivo da fêmea, o comum é usado; sem o da
forma, o da espécie.

## 3. Compilar o port com o patch

Os sprites só aparecem num build que inclua este código (patch
`patches/pokeemerald/0083-bw-animated-battlers.patch` e
`3ds_port/src/3ds_bw_mons*.c`). Veja [DEVELOPMENT.md](DEVELOPMENT.md).

## Como funciona

- `battle_gfx_sfx_util.c` (patch 0083) avisa o lado do jogo
  (`3ds_bw_mons_game.c`) toda vez que os gráficos de um battler são
  carregados (entrada, Transform, Castform, Substitute) e, quando um oponente
  desmaia, até onde o sprite já foi apagado.
- O compositor (`3ds_video.c`, `BwFrame`/`BwDrawInScene`) troca cada entrada
  de OAM que mostra os tiles do battler pelo quadro atual da animação, com a
  posição, a matriz afim (o crescimento ao sair da Poké Ball, as deformações
  dos golpes), o blend e o fade de paleta do próprio sprite. A imagem fica em
  pé sobre os "pés" do sprite original; 1 pixel BW = 2/3 de pixel GBA
  (`BW_SCALE`).
- A animação para enquanto o Pokémon está dormindo ou congelado.

## Limitações conhecidas

- Golpes que copiam o sprite para uma camada de fundo (por exemplo, alguns
  que movem o Pokémon para fora da tela) mostram o sprite original do Emerald
  enquanto duram.
- Não muda a Pokédex, o resumo nem o PC, só a batalha.
- Sprites muito grandes (Rayquaza, Wailord, Groudon) passam um pouco da caixa
  de 64x64 do GBA. Isso é intencional, para manter a escala dos sprites do BW.
