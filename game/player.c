/* Eliot                                                                     */
/* Copyright (C) 2004 Eliot                                                  */
/* Olivier Teuliere  <ipkiss@via.ecp.fr>                                     */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* $Id: player.c,v 1.1 2004/08/07 18:25:03 ipkiss Exp $ */

#include <stdlib.h>
#include <string.h>

#include "dic.h"
#include "tiles.h"
#include "rack.h"
#include "pldrack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "player.h"

#include "debug.h"

struct tplayer {
    int human;
    int score;
    play_status status;     /* Only used for duplicate mode currently */

    /* History of the racks and rounds for the player */
    int nracks;
    Playedrack racks[PLAYEDRACK_MAX];
    Round      rounds[PLAYEDRACK_MAX];
    int        turns[PLAYEDRACK_MAX];

    /* Results of a search with the current rack */
    Results searchresults;
};


Player
Player_create(void)
{
    int i;
    Player p;
    p = (Player) malloc(sizeof(struct tplayer));
    if (p == NULL)
        return NULL;

    for (i = 0; i < PLAYEDRACK_MAX; i++)
    {
        p->racks[i] = Playedrack_create();
        p->rounds[i] = Round_create();
    }
    p->searchresults = Results_create();

    return p;
}


static void
Player_init(Player p, int human)
{
    int i;

    p->score = 0;
    p->human = human;
    p->status = TO_PLAY;
    p->nracks = 0;
    for (i = 0; i < PLAYEDRACK_MAX; i++)
    {
        Playedrack_init(p->racks[i]);
        Round_init(p->rounds[i]);
    }
    Results_init(p->searchresults);
}


void
Player_inithuman(Player p)
{
    Player_init(p, 1);
}


void
Player_initai(Player p)
{
    Player_init(p, 0);
}


void
Player_destroy(Player p)
{
    int i;
    if (p)
    {
        for (i = 0; i < PLAYEDRACK_MAX; i++)
        {
            Playedrack_destroy(p->racks[i]);
            Round_destroy(p->rounds[i]);
        }
        Results_destroy(p->searchresults);
        free(p);
    }
}


Playedrack
Player_getplayedrack(Player p)
{
    return p->racks[p->nracks];
}


Playedrack
Player_getlastrack(Player p)
{
    return p->racks[p->nracks - 1];
}


Round
Player_getlastround(Player p)
{
    return p->rounds[p->nracks - 1];
}


int
Player_getpoints(Player p)
{
    return p->score;
}


void
Player_addpoints(Player p, int s)
{
    p->score += s;
}


void
Player_setstatus(Player p, play_status stat)
{
    p->status = stat;
}


play_status
Player_getstatus(Player p)
{
    return p->status;
}


/*
 * This function increments the number of racks, and fills the new rack
 * with the unplayed tiles from the previous one.
 * 03 sept 2000 : We have to sort the tiles according to the new rules
 */
void
Player_endturn(Player p, Round round, int turn)
{
    tile_t i;
    Rack rack;

    p->turns[p->nracks] = turn;
    Round_copy(p->rounds[p->nracks], round);

    /* Increment the number of racks
     * We are now going to deal with p->racks[p->nracks - 1] and
     * p->racks[p->nracks] */
    p->nracks++;

    rack = Rack_create();
    Playedrack_getrack(p->racks[p->nracks - 1], rack);

    /* We remove the played tiles from the rack */
    for (i = 0; i < Round_wordlen(round); i++)
    {
        if (Round_playedfromrack(round, i))
        {
            if (Round_joker(round,i))
                Rack_remove(rack, (tile_t)JOKER_TILE);
            else
                Rack_remove(rack, Round_gettile(round, i));
        }
    }

    Playedrack_init(p->racks[p->nracks]);
    Playedrack_setold(p->racks[p->nracks], rack);
    Rack_destroy(rack);
}


int
Player_ai(Player p)
{
    return ! p->human;
}


int
Player_ai_search(Player p, Dictionary dic, Board board, int turn)
{
    Rack rack;
    if (dic == NULL)
        return 1;

    Results_init(p->searchresults);

    rack = Rack_create();
    Playedrack_getrack(p->racks[p->nracks], rack);
    if (turn == 0)
        Board_search_first(dic, board, rack, p->searchresults);
    else
        Board_search(dic, board, rack, p->searchresults);
    Rack_destroy(rack);
    return 0;
}


Round
Player_ai_bestround(Player p)
{
    return Results_get(p->searchresults, 0);
}


Results
Player_ai_getresults(Player p)
{
    return p->searchresults;
}

