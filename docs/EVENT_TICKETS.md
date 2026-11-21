# The event islands without the Mystery Gift

Emerald's four event islands are in every cartridge, but their tickets only
came through Nintendo's Mystery Gift distributions, which ended long ago.
OPTIONS > POKEDEX > **EVENT TICKETS** (on by default) lets the player earn
them in the game. After the Hall of Fame, the MYSTERY GIFT man upstairs in a
POKéMON CENTER appears when a ticket is due and hands it over, as he handed
over the EON TICKET in the original:

| Item | Island and Pokémon | Earned by |
|---|---|---|
| AURORA TICKET | BIRTH ISLAND: DEOXYS | beating STEVEN at METEOR FALLS |
| EON TICKET | SOUTHERN ISLAND: the LATIOS or LATIAS the player did not choose | beating four GYM LEADERS in a rematch |
| OLD SEA MAP | FARAWAY ISLAND: MEW | beating all eight GYM LEADERS in a rematch |
| MYSTIC TICKET | NAVEL ROCK: LUGIA and HO-OH | winning the POKéMON LEAGUE a second time |

The conditions read what the save already keeps, so a save that met one
before the option existed gets its ticket too. A ticket is given once: when
the ferry to its island is enabled (by this or by a real Mystery Gift), it is
done. The islands, the ferry and the Pokémon are the game's own; the ticket
only sets the same flags the Mystery Gift scripts set.

- `patches/pokeemerald/0099-event-tickets.patch`:
  - `src/event_tickets.c`: `CtrEventTicket_Due`, the special that names the
    next ticket earned and not had (a rematch sets the flag of its
    `TRAINER_*_2`, the Hall of Fame counts its entries in
    `GAME_STAT_ENTERED_HOF`);
  - `data/scripts/cable_club.inc`: the MYSTERY GIFT man shows when a ticket
    is due and hands over each one, with a line saying who left it;
  - `data/specials.inc`: the special, at the end of the table.
- `3ds_port/src/3ds_extras.c`: the `event_tickets` option.
- `tools/locales/pt_br/data/scripts/cable_club.inc.txt`: the three new lines
  in Brazilian Portuguese; the other languages show them in English.

No art. The rest of the man's lines are the game's own Mystery Gift texts.
