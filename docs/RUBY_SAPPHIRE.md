# Ruby and Sapphire's wild Pokémon

OPTIONS > POKÉDEX > **RUBY/SAPPHIRE** (on by default): the seven Hoenn
Pokémon that Ruby and Sapphire have in the wild and Emerald does not, on the
maps and slots where those games have them (pret's pokeruby,
`src/data/wild_encounters.json`), at their level there:

| Pokémon | Where | Met in |
|---|---|---|
| Zangoose | Route 114 (grass) | Ruby |
| Roselia | Route 117 (grass) | Ruby or Sapphire |
| Lunatone | Meteor Falls, every room (land and surfing) | Sapphire |
| Meditite | Mt. Pyre's outside, Victory Road B1F | Ruby or Sapphire |
| Medicham | Victory Road B1F and B2F | Ruby or Sapphire |
| Dusclops | Sky Pillar 1F, 3F and 5F (levels 48, 51, 54) | Ruby |
| Surskit | Routes 102, 111, 114, 117 and 120 (surfing, 1%) | Ruby or Sapphire |

A slot is taken only where the species it held stays on that map, through
another slot or its fishing, so no Emerald Pokémon leaves a map. Dusclops
takes the first of Banette's two Sky Pillar slots at Ruby's levels, above
Emerald's for that tower. Surskit takes the fifth surfing slot (Goldeen's,
still fished there); its grass slot in Ruby and Sapphire is the only Seedot
of those routes and stays Emerald's.

## Met in Ruby or Sapphire

Each is caught as met in the game it comes from (the game of origin a save
holds for every Pokémon): Ruby or Sapphire where only one has it, either at
random where both do. Its map, slot and level are that game's, and the game
draws the slot, the level, the nature and the personality with the random
calls in the same order, so a save checker such as PKHeX finds the encounter
in that game.

Visible wild Pokémon (OPTIONS > FIELD) are made the same way when they
appear (their personality and IVs kept for the battle), so they too are met
as a random encounter would be, whatever their species.

Both hold only with the game's own odds: SHINY ODDS (OPTIONS > CHEATS) above
the first setting rerolls the personality, and a lead whose ability is
Emerald's own (Synchronize's nature, for one) is not a Ruby or Sapphire one.

## Where it is

They are met in battle, as visible wild Pokémon, in the bottom screen's list
of the map's wild Pokémon, and the Pokédex's AREA marks their maps.
`3ds_port/src/3ds_encounter_tables.c` holds the slots, their levels and
their game (checked against the pinned tables by
`3ds_port/tests/encounter_tables_test.py`); patch 0094 makes the game read
the surfing table through it and the AREA screen ask it, records the game
of origin and makes visible wild Pokémon as random encounters are.
