/* Eliot                                                                     */
/* Copyright (C) 1999-2004 Eliot                                             */
/* Antoine Fraboulet <antoine.fraboulet@free.fr>                             */
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

/* $Id: game.c,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "dic.h"
#include "dic_search.h"
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "board.h"
#include "player.h"
#include "game.h"
#include "gameio.h"
#include "game_internals.h"

#include "debug.h"


/*********************************************************
 * Static functions
 *********************************************************/

static int  Game_helper_playround(Game game, Round round);
static int  Game_rackinbag(Rack r, Bag b);
static int  Game_helper_setrackrandom(Game game, int p, int check,
                                      set_rack_mode mode);
static int  Game_helper_setrackmanual(Game game, int p, int check,
                                      const char *t);
static void Game_realbag(Game game, Bag bag);
static int  Game_checkplayedword(Game game, const char coord[COOR_SIZE_MAX],
                                 const char *word, Round round);
static int  Game_freegame_ai(Game game, int n);
static void Game_freegame_end(Game game);
static void Game_duplicate_playround(Game game, Round round, int n);
static int  Game_duplicate_ai(Game game, int n);
static int  Game_duplicate_endturnforreal(Game game);
static void Game_duplicate_end(Game game);


/*********************************************************
 *********************************************************/

Game
Game_create(Dictionary dic)
{
    int i;
    Game g;

    g = (Game)malloc(sizeof(struct tgame));
    if (g == NULL)
        return NULL;

    g->dic           = dic;
    g->bag           = Bag_create();
    g->board         = Board_create();

    for (i = 0; i < PLAYEDRACK_MAX; i++)
    {
        g->playedrounds[i] = Round_create();
        g->playedracks[i]  = Playedrack_create();
    }
    for (i = 0; i < PLAYERS_MAX; i++)
    {
        g->players[i] = Player_create();
    }

    Game_init(g);
    return g;
}


void
Game_destroy(Game g)
{
    int i;
    if (g)
    {
        Bag_destroy(g->bag);
        Board_destroy(g->board);
        for (i = 0; i < PLAYEDRACK_MAX; i++)
        {
            Round_destroy(g->playedrounds[i]);
            Playedrack_destroy(g->playedracks[i]);
        }
        for (i = 0; i < PLAYERS_MAX; i++)
        {
            Player_destroy(g->players[i]);
        }
        free(g);
    }
}

/*********************************************************
 *********************************************************/

void
Game_init(Game g)
{
    int i;
    Bag_init(g->bag);
    Board_init(g->board);
    for (i = 0; i < PLAYEDRACK_MAX; i++)
    {
        Round_init(g->playedrounds[i]);
        Playedrack_init(g->playedracks[i]);
    }
    for (i = 0; i < PLAYERS_MAX; i++)
    {
        Player_inithuman(g->players[i]);
    }
    g->nrounds  = 0;
    g->points = 0;
    g->nplayers = 0;
    g->currplayer = -1;
    g->mode = TRAINING;
}


int
Game_load(Game game, FILE* fin)
{
    return Game_read_game(fin, game);
}


void
Game_save(Game game, FILE* fout)
{
    Game_print_game(fout, game);
}


/*********************************************************
 *********************************************************/


Dictionary
Game_getdic(Game g)
{
    return g->dic;
}


void
Game_setdic(Game g, Dictionary d)
{
    g->dic = d;
}


/*********************************************************
 *********************************************************/

int
Game_training_search(Game game)
{
    // Search for the current player
    return Player_ai_search(game->players[game->currplayer],
                            game->dic,
                            game->board,
                            game->nrounds);
}


int
Game_training_play(Game game, const char coord[COOR_SIZE_MAX], const char *word)
{
    int res;
    Round round;

    /* Perform all the validity checks, and fill a round */
    round = Round_create();
    res = Game_checkplayedword(game, coord, word, round);
    if (res != 0)
    {
        Round_destroy(round);
        return res;
    }

    /* Update the rack and the score of the current player */
    Player_addpoints(game->players[game->currplayer], Round_points(round));
    Player_endturn(game->players[game->currplayer], round, game->nrounds);

    /* Everything is OK, we can play the word */
    Game_helper_playround(game, round);
    Round_destroy(round);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_training_endturn(game);

    return 0;
}


int
Game_training_playresult(Game game, int n)
{
    Round round;
    Player player;
    Results results;
    int res;

    if (game->dic == NULL)
        return 1;

    player = game->players[game->currplayer];
    results = Player_ai_getresults(player);
    if ((round = Results_get(results, n)) == NULL)
        return 2;

    /* Update the rack and the score of the current player */
    Player_addpoints(player, Round_points(round));
    Player_endturn(player, round, game->nrounds);

    res = Game_helper_playround(game, round);

    if (res == 0)
        Results_init(results);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_training_endturn(game);

    return res;
}


/* This function plays a round on the board */
static int
Game_helper_playround(Game game, Round round)
{
    /*
     * We remove tiles from the bag only when they are played
     * on the board. When going back in the game, we must only
     * replace played tiles.
     * We test a rack when it is set but tiles are left in the bag.
     */

    int i;

    /* History of the game */
    Round_copy(game->playedrounds[game->nrounds], round);

    game->points += Round_points(round);

    /* Update the board and the bag */
    Board_addround(game->dic, game->board, round);
    for (i = 0; i < Round_wordlen(round); i++)
    {
        if (Round_playedfromrack(round, i))
        {
            if (Round_joker(round, i))
                Bag_taketile(game->bag, (tile_t)JOKER_TILE);
            else
                Bag_taketile(game->bag, Round_gettile(round, i));
        }
    }

    game->nrounds++;
    return 0;
}


int
Game_back(Game game, int n)
{
    int i, j;
    Round lastround;
    Player player;

    if (game->dic == NULL)
        return 1;

    for (i = 0; i < n; i++)
    {
        if (game->nrounds)
        {
            game->nrounds--;
            Game_prevplayer(game);
            player = game->players[game->currplayer];
            lastround = game->playedrounds[game->nrounds];

            /* Remove the points of this round */
            Player_addpoints(player, - Round_points(lastround));
            game->points -= Round_points(lastround);

            /* Remove the word from the board, and put its letters back
             * into the bag */
            Board_removeround(game->dic, game->board, lastround);
            for (j = 0; j < Round_wordlen(lastround); j++)
            {
                if (Round_playedfromrack(lastround, j))
                {
                    if (Round_joker(lastround, j))
                        Bag_replacetile(game->bag, JOKER_TILE);
                    else
                        Bag_replacetile(game->bag, Round_gettile(lastround, j));
                }
            }
            Round_init(lastround);
        }
    }
    return 0;
}


int
Game_testplay(Game game, int n)
{
    Round round;
    Results results = Player_ai_getresults(game->players[game->currplayer]);

    if ((round = Results_get(results, n)) == NULL)
        return 2;
    Board_testround(game->board,round);
    return 0;
}


int
Game_removetestplay(Game game)
{
    Board_removetestround(game->board);
    return 0;
}

/*********************************************************
 *********************************************************/

char
Game_getboardchar(Game g, int r, int c)
{
    tile_t t;
    char l = 0;
    t = Board_tile(g->board, r, c);
    if (t)
    {
        l = codetochar(t);
        if (Board_joker(g->board, r, c))
            l = tolower(l);
    }
    return l;
}


int
Game_getboardcharattr(Game g, int r, int c)
{
    int t = Board_gettestchar(g->board, r, c);
    int j = Board_joker(g->board, r, c);
    return  (t << 1) | j;
}


int
Game_getboardwordmultiplier(Game g, int r, int c)
{
    return Board_getwordmultiplier(g->board, r, c);
}


int
Game_getboardlettermultiplier(Game g, int r, int c)
{
    return Board_getlettermultiplier(g->board, r, c);
}

/*********************************************************
 *********************************************************/

static void
Game_realbag(Game game, Bag bag)
{
    int i, j;
    tile_t tiles[RACK_MAX];
    Playedrack pld;

    Bag_copy(bag, game->bag);

    /* The real content of the bag depends of the game mode */
    if (game->mode == FREEGAME)
    {
        /* In freegame mode, replace the letters from all the racks */
        for (i = 0; i < game->nplayers; i++)
        {
            pld = Player_getplayedrack(game->players[i]);
            Playedrack_getalltiles(pld, tiles);
            for (j = 0; j < Playedrack_ntiles(pld); j++)
            {
                Bag_taketile(bag, tiles[j]);
            }
        }
    }
    else
    {
        /* In training or duplicate mode, replace the rack of the current
         * player only */
        pld = Player_getplayedrack(game->players[game->currplayer]);
        Playedrack_getalltiles(pld, tiles);
        for (j = 0; j < Playedrack_ntiles(pld); j++)
        {
            Bag_taketile(bag, tiles[j]);
        }
    }
}


static int
Game_rackinbag(Rack r, Bag b)
{
    int i;
    for (i = 0; i < TILES_NUMBER; i++)
    {
        if (Rack_in(r, i) > Bag_in(b, i))
            return 1;
    }
    return 0;
}


static int
Game_helper_setrackrandom(Game game, int p, int check, set_rack_mode mode)
{
    Bag b;
    tile_t l;
    tile_t ttmp[RACK_MAX];
    Playedrack pld;
    int i, nold, min;

    PDEBUG(p < 0 || p >= game->nplayers, "GAME: wrong player number\n");

    pld = Player_getplayedrack(game->players[p]);
    nold = Playedrack_nold(pld);

    /* Create a copy of the bag in which we can do everything we want */
    b = Bag_create();
    /* Remove from the bag the tiles of the racks */
    Game_realbag(game, b);

    /* We may have removed too many letters from the bag (i.e. the 'new'
     * letters of the player) */
    if (mode == RACK_NEW && nold != 0)
    {
        Playedrack_getnewtiles(pld, ttmp);
        for (i = 0; i < Playedrack_nnew(pld); i++)
        {
            Bag_replacetile(b, ttmp[i]);
        }
        Playedrack_resetnew(pld);
    }
    else
    {
        /* "Complement" with an empty rack is equivalent to ALL */
        Playedrack_init(pld);
        /* Do not forget to update nold */
        nold = 0;
    }

    /* Nothing in the rack, nothing in the bag --> end of the game */
    if (Bag_ntiles(b) == 0 && Playedrack_ntiles(pld) == 0)
    {
        Bag_destroy(b);
        return 1;
    }

    /* Make sure there is a way to complete the rack */
    min = 0;
    if (check)
    {
        int oldc, oldv;

        if (Bag_nvowels(b) == 0 || Bag_nconsonants(b) == 0)
        {
            Bag_destroy(b);
            return 1;
        }
        if (Bag_nvowels(b) > 1 && Bag_nconsonants(b) > 1
            && Game_getnrounds(game) < 15)
            min = 2;
        else
            min = 1;

        /* Count the remaining consonants and vowels in the rack */
        Playedrack_getoldtiles(pld, ttmp);
        oldc = 0;
        oldv = 0;
        for (i = 0; i < nold; i++)
        {
            if (Tiles_consonants[ttmp[i]])
                oldc++;
            if (Tiles_vowels[ttmp[i]])
                oldv++;
        }

        /* RACK_MAX - nold is the number of letters to add */
        if (min > oldc + RACK_MAX - nold ||
            min > oldv + RACK_MAX - nold)
        {
            Bag_destroy(b);
            return 3;
        }
    }

    /* Get new tiles from the bag */
    for (i = nold; (Bag_ntiles(b) != 0) && (i < RACK_MAX); i++)
    {
        l = Bag_select_random(b);
        Bag_taketile(b, l);
        Playedrack_addnew(pld, l);
    }

    Bag_destroy(b);

    if (check && Playedrack_checkrack(pld, min) != 0)
        return 2;

    return 0;
}


int
Game_helper_setrackmanual(Game game, int p, int check, const char *t)
{
    int i, min;
    tile_t l;
    Playedrack pld;
    Rack rack;

    pld = Player_getplayedrack(game->players[p]);
    Results_init(Player_ai_getresults(game->players[p]));
    Playedrack_init(pld);

    if (t == NULL || t[0] == 0)
    {
        return 0;
    }

    for (i = 0; t[i] != 0 && t[i] != '+'; i++)
    {
        if ((l = chartocode(t[i])) == 0)
        {
            return 1;
        }
        Playedrack_addold(pld, l);
    }

    if (t[i] == '+')
    {
        for (i++; t[i] != 0; i++)
        {
            if ((l = chartocode(t[i])) == 0)
            {
                return 1;
            }
            Playedrack_addnew(pld, l);
        }
    }

    rack = Rack_create();
    Playedrack_getrack(pld, rack);
    if (Game_rackinbag(rack, game->bag) == 1)
    {
        Playedrack_init(pld);
        Rack_destroy(rack);
        return 1;
    }
    Rack_destroy(rack);

    if (check)
    {
        if (Bag_nvowels(game->bag) > 1 && Bag_nconsonants(game->bag) > 1
            && Game_getnrounds(game) < 15)
            min = 2;
        else
            min = 1;
        return Playedrack_checkrack(pld, min) ? 2 : 0;
    }

    return 0;
}


int
Game_duplicate_setrackrandom(Game game, int p, int check, set_rack_mode mode)
{
    int res;
    do
    {
        res = Game_helper_setrackrandom(game, p, check, mode);
    } while (res == 2);
    return res;
}


int
Game_freegame_setrackrandom(Game game, int p, int check, set_rack_mode mode)
{
    int res;
    do
    {
        res = Game_helper_setrackrandom(game, p, check, mode);
    } while (res == 2);
    return res;
}


int
Game_training_setrackrandom(Game game, int check, set_rack_mode mode)
{
    int res;
    int p = game->currplayer;
    Results_init(Player_ai_getresults(game->players[p]));
    do
    {
        res = Game_helper_setrackrandom(game, p, check, mode);
    } while (res == 2);

    if (res == 0)
    {
        /* History of the game
         * XXX: we overwrite it every time a new rack is set, because
         * Game_print_playedrack uses the global game racks, instead of
         * the player ones. Perhaps it would be better to have different
         * Game_print_playedrack functions for the different modes? */
        Playedrack_copy(game->playedracks[game->nrounds],
                        Player_getplayedrack(game->players[p]));
    }
    return res;
}


int
Game_training_setrackmanual(Game game, int check, const char *t)
{
    int res;
    int p = game->currplayer;
    Results_init(Player_ai_getresults(game->players[p]));
    do
    {
        res = Game_helper_setrackmanual(game, p, check, t);
    } while (res == 2);

    if (res == 0)
    {
        /* History of the game
        * XXX: we overwrite it every time a new rack is set, because
        * Game_print_playedrack uses the global game racks, instead of
        * the player ones. Perhaps it would be better to have different
        * Game_print_playedrack functions for the different modes? */
        Playedrack_copy(game->playedracks[game->nrounds],
                        Player_getplayedrack(game->players[p]));
    }
    return res;
}

/*********************************************************
 *********************************************************/

int
Game_getcharinbag(Game g, char c)
{
    return Bag_in(g->bag, chartocode(c));
}


/*********************************************************
 *********************************************************/

int
Game_getnrounds(Game g)
{
    return g->nrounds;
}


void
Game_getplayedrack(Game g, int num, char buff[RACK_SIZE_MAX])
{
    int i, l = 0;
    Playedrack pld;
    tile_t bt[RACK_SIZE_MAX];
    buff[0] = 0;
    if (num < 0 || num > g->nrounds)
        return;
    pld = g->playedracks[num];
    Playedrack_getoldtiles(pld, bt);
    for (i = 0; bt[i]; i++)
        buff[l++] = codetochar(bt[i]);
    Playedrack_getnewtiles(pld, bt);

    if (i && bt[0])
        buff[l++] = '+';

    for (i = 0; bt[i]; i++)
        buff[l++] = codetochar(bt[i]);
    buff[l] = 0;
}


void
Game_getplayedword(Game g, int num, char buff[WORD_SIZE_MAX])
{
    Round r;
    char c;
    int i, l = 0;
    buff[0] = 0;
    if (num < 0 || num >= g->nrounds)
        return;
    r = g->playedrounds[num];
    for (i = 0; i < Round_wordlen(r); i++)
    {
        c = codetochar(Round_gettile(r, i));
        if (Round_joker(r, i))
            c = tolower(c);
        buff[l++] = c;
    }
    buff[i] = 0;
}


void
Game_getplayedfirstcoord (Game g, int num, char buff[COOR_SIZE_MAX])
{
    Round r;
    buff[0] = 0;
    if (num < 0 || num >= g->nrounds)
        return;
    r = g->playedrounds[num];
    if (Round_dir(r) == HORIZONTAL)
        sprintf(buff, "%c", Round_row(r) + 'A' - 1);
    else
        sprintf(buff, "%d", Round_column(r));
}


void
Game_getplayedsecondcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
    Round r;
    buff[0] = 0;
    if (num < 0 || num >= g->nrounds)
        return;
    r = g->playedrounds[num];
    if (Round_dir(r) == HORIZONTAL)
        sprintf(buff, "%d", Round_column(r));
    else
        sprintf(buff, "%c", Round_row(r) + 'A' - 1);
}


void
Game_getplayedcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
    char c1[COOR_SIZE_MAX];
    char c2[COOR_SIZE_MAX];
    Game_getplayedfirstcoord(g, num, c1);
    Game_getplayedsecondcoord(g, num, c2);
    sprintf(buff, "%2s%2s", c1, c2);
}


int
Game_getplayedpoints(Game g, int num)
{
    if (num < 0 || num >= g->nrounds)
        return 0;
    return Round_points(g->playedrounds[num]);
}


int
Game_getplayedbonus(Game g, int num)
{
    if (num < 0 || num >= g->nrounds)
        return 0;
    return Round_bonus(g->playedrounds[num]);
}

/*********************************************************
 *********************************************************/

int
Game_getnresults(Game g)
{
    return Results_in(Player_ai_getresults(g->players[g->currplayer]));
}


void
Game_getsearchedword(Game g, int num, char buff[WORD_SIZE_MAX])
{
    Round r;
    char c;
    int i, l = 0;
    Results results = Player_ai_getresults(g->players[g->currplayer]);
    buff[0] = 0;
    if (num < 0 || num >= Results_in(results))
        return;
    r = Results_get(results, num);
    for (i = 0; i < Round_wordlen(r); i++)
    {
        c = codetochar(Round_gettile(r, i));
        if (Round_joker(r,i))
            c = tolower(c);
        buff[l++] = c;
    }
    buff[i] = 0;
}


void
Game_getsearchedfirstcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
    Round r;
    Results results = Player_ai_getresults(g->players[g->currplayer]);
    buff[0] = 0;
    if (num < 0 || num >= Results_in(results))
        return;
    r = Results_get(results, num);
    if (Round_dir(r) == HORIZONTAL)
        sprintf(buff, "%c", Round_row(r) + 'A' - 1);
    else
        sprintf(buff, "%d", Round_column(r));
}


void
Game_getsearchedsecondcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
    Round r;
    Results results = Player_ai_getresults(g->players[g->currplayer]);
    buff[0] = 0;
    if (num < 0 || num >= Results_in(results))
        return;
    r = Results_get(results, num);
    if (Round_dir(r) == HORIZONTAL)
        sprintf(buff, "%d", Round_column(r));
    else
        sprintf(buff, "%c", Round_row(r) + 'A' - 1);
}


void
Game_getsearchedcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
    char c1[COOR_SIZE_MAX];
    char c2[COOR_SIZE_MAX];
    Game_getsearchedfirstcoord(g, num, c1);
    Game_getsearchedsecondcoord(g, num, c2);
    sprintf(buff, "%2s%2s", c1, c2);
}


int
Game_getsearchedpoints(Game g, int num)
{
    Results results = Player_ai_getresults(g->players[g->currplayer]);
    if (num < 0 || num >= Results_in(results))
        return 0;
    return Round_points(Results_get(results, num));
}


int
Game_getsearchedbonus(Game g, int num)
{
    Results results = Player_ai_getresults(g->players[g->currplayer]);
    if (num < 0 || num >= Results_in(results))
        return 0;
    return Round_bonus(Results_get(results, num));
}


int
Game_getnplayers(Game g)
{
    return g->nplayers;
}



int
Game_getplayerpoints(Game g, int num)
{
    if (num < 0 || num >= g->nplayers)
        return 0;
    return Player_getpoints(g->players[num]);
}


void
Game_addhumanplayer(Game g)
{
    if (g->nplayers < PLAYERS_MAX)
        Player_inithuman(g->players[g->nplayers++]);
}


void
Game_addaiplayer(Game g)
{
    if (g->nplayers < PLAYERS_MAX)
        Player_initai(g->players[g->nplayers++]);
}


int
Game_currplayer(Game g)
{
    return g->currplayer;
}


void
Game_prevplayer(Game g)
{
    if (g->nplayers == 0)
        return;

    if (g->currplayer == 0)
        g->currplayer = g->nplayers - 1;
    else
        g->currplayer--;
}


void
Game_nextplayer(Game g)
{
    if (g->nplayers == 0)
        return;

    if (g->currplayer == g->nplayers - 1)
        g->currplayer = 0;
    else
        g->currplayer++;
}


/*
 * This function checks whether it is legal to play the given word at the
 * given coordinates. If so, the function fills a Round object, also given as
 * a parameter (and supposed to be already created).
 * Possible return values:
 *  0: correct word, the Round can be used by the caller
 *  1: no dictionary set
 *  2: invalid coordinates (unreadable or out of the board)
 *  3: word not present in the dictionary
 *  4: not enough letters in the rack to play the word
 *  5: word is part of a longer one
 *  6: word overwriting an existing letter
 *  7: invalid crosscheck, or word going out of the board
 *  8: word already present on the board (no new letter from the rack)
 *  9: isolated word (not connected to the rest)
 * 10: first word not horizontal
 * 11: first word not covering the H8 square
 */
static int
Game_checkplayedword(Game game, const char coord[COOR_SIZE_MAX],
                     const char *word, Round round)
{
    Rack rack;
    Player player;
    char l[COOR_SIZE_MAX];
    int col, row;
    int i, res;
    tile_t *tiles;
    tile_t t;

    if (game->dic == NULL)
        return 1;

    // TODO: check that there is at least 1 player.

    /* Init the round with the given coordinates */
    Round_init(round);
    if (sscanf(coord, "%1[a-oA-O]%2d", l, &col) == 2)
        Round_setdir(round, HORIZONTAL);
    else if (sscanf(coord, "%2d%1[a-oA-O]", &col, l) == 2)
        Round_setdir(round, VERTICAL);
    else
        return 2;
    row = toupper(*l) - 'A' + 1;
    if (col < BOARD_MIN || col > BOARD_MAX ||
        row < BOARD_MIN || row > BOARD_MAX)
    {
        return 2;
    }
    Round_setcolumn(round, col);
    Round_setrow(round, row);

    /* Check the existence of the word */
    if (Dic_search_word(game->dic, word) == 0)
        return 3;

    /* Set the word */
    // TODO: make this a Round_ function (Round_setwordfromchar for example)
    // or a Tiles_ function (to transform a char* into a tile_t*)
    // Adding a getter on the word could help too...
    tiles = (tile_t*)malloc(strlen(word) + 1);
    for (i = 0; i < strlen(word); i++)
    {
        tiles[i] = chartocode(word[i]);
        if (islower(word[i]))
            Round_setjoker(round, i);
    }
    tiles[strlen(word)] = '\0';
    Round_setword(round, tiles);

    free(tiles);

    /* Check the word position, compute its points,
     * and specify the origin of each letter (board or rack) */
    res = Board_checkround(game->dic, game->board, round, game->nrounds == 0);
    if (res != 0)
        return res + 4;

    /* Check that the word can be formed with the tiles in the rack:
     * we first create a copy of the rack, then we remove the tiles
     * one by one */
    rack = Rack_create();
    player = game->players[game->currplayer];
    Playedrack_getrack(Player_getplayedrack(player), rack);

    for (i = 0; i < Round_wordlen(round); i++)
    {
        if (Round_playedfromrack(round, i))
        {
            if (Round_joker(round, i))
                t = JOKER_TILE;
            else
                t = Round_gettile(round, i);

            if (! Rack_in(rack, t))
            {
                Rack_destroy(rack);
                return 4;
            }
            Rack_remove(rack, t);
        }
    }
    Rack_destroy(rack);

    return 0;
}


int
Game_training_start(Game game)
{
    if (game->nplayers != 0)
        return 1;

    game->mode = TRAINING;

    /* Training mode implicitly uses 1 human player */
    Game_addhumanplayer(game);
    game->currplayer = 0;
    return 0;
}


void
Game_training_endturn(Game game)
{
    /* History of the racks */
    Playedrack_copy(game->playedracks[game->nrounds],
                    Player_getplayedrack(game->players[0]));
}


int
Game_freegame_play(Game game, const char coord[COOR_SIZE_MAX], const char *word)
{
    int res;
    Round round;

    /* Perform all the validity checks, and fill a round */
    round = Round_create();
    res = Game_checkplayedword(game, coord, word, round);
    if (res != 0)
    {
        Round_destroy(round);
        return res;
    }

    /* Update the rack and the score of the current player */
    Player_addpoints(game->players[game->currplayer], Round_points(round));
    Player_endturn(game->players[game->currplayer], round, game->nrounds);

    /* Everything is OK, we can play the word */
    Game_helper_playround(game, round);
    Round_destroy(round);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_freegame_endturn(game);

    return 0;
}


static int
Game_freegame_ai(Game game, int n)
{
    Round round;
    Player player;

    PDEBUG(n < 0 || n >= game->nplayers, "GAME: wrong player number\n");
    player = game->players[n];

    PDEBUG(! Player_ai(player), "GAME: AI requested for a human player!\n");

    Player_ai_search(player, game->dic, game->board, game->nrounds);
    round = Player_ai_bestround(player);
    if (round == NULL)
    {
        /* XXX: a better way to indicate that the AI player passes
         * should be found. In particular, we don't know the letters
         * it wants to change. */
        Game_freegame_pass(game, "", n);
    }
    else
    {
        /* Update the rack and the score of the current player */
        Player_addpoints(player, Round_points(round));
        Player_endturn(player, round, game->nrounds);

        Game_helper_playround(game, round);
        Game_freegame_endturn(game);
    }

    return 0;
}


int
Game_freegame_start(Game game)
{
    int i;
    if (game->nplayers == 0)
        return 1;

    game->mode = FREEGAME;

    /* Set the initial racks of the players */
    for (i = 0; i < game->nplayers; i++)
    {
        Game_freegame_setrackrandom(game, i, 0, RACK_ALL);
    }

    // XXX
    game->currplayer = 0;

    /* History of the racks */
    Playedrack_copy(game->playedracks[game->nrounds],
                    Player_getplayedrack(game->players[game->currplayer]));

    /* If the first player is an AI, make it play now */
    if (Player_ai(game->players[0]))
    {
        if (Game_freegame_ai(game, 0))
            return 2;
    }

    return 0;
}


int
Game_freegame_endturn(Game game)
{
    /* Complete the rack for the player that just played */
    if (Game_freegame_setrackrandom(game, game->currplayer, 0, RACK_NEW) == 1)
    {
        /* End of the game */
        Game_freegame_end(game);
        return 1;
    }

    /* Next player */
    Game_nextplayer(game);

    /* History of the racks */
    Playedrack_copy(game->playedracks[game->nrounds],
                    Player_getplayedrack(game->players[game->currplayer]));

    /* If this player is an AI, make it play now */
    if (Player_ai(game->players[game->currplayer]))
    {
        if (Game_freegame_ai(game, game->currplayer))
            return 2;
    }

    return 0;
}


/* Adjust the scores of the players with the points of the remaining tiles */
static void
Game_freegame_end(Game game)
{
    int i, j;
    tile_t tiles[RACK_MAX];
    Playedrack pld;

    /* Add the points of the remaining tiles to the score of the current
     * player (i.e. the first player with an empty rack), and remove them
     * from the score of the players who still had tiles */
    for (i = 0; i < game->nplayers; i++)
    {
        if (i != game->currplayer)
        {
            pld = Player_getplayedrack(game->players[i]);
            Playedrack_getalltiles(pld, tiles);
            for (j = 0; j < Playedrack_ntiles(pld); j++)
            {
                Player_addpoints(game->players[i],
                                 - Tiles_points[tiles[j]]);
                Player_addpoints(game->players[game->currplayer],
                                 Tiles_points[tiles[j]]);
            }
        }
    }
}


int
Game_freegame_pass(Game game, const char *tochange, int n)
{
    int i;
    tile_t tile;
    Bag bag;
    Playedrack pld;
    Rack rack;
    Player player;

    PDEBUG(n < 0 || n >= game->nplayers, "GAME: wrong player number\n");
    player = game->players[n];

    /* You cannot change more letters than what is left in the bag! */
    bag = Bag_create();
    Game_realbag(game, bag);
    if (Bag_ntiles(bag) < strlen(tochange))
    {
        Bag_destroy(bag);
        return 1;
    }
    Bag_destroy(bag);

    rack = Rack_create();
    pld = Player_getplayedrack(player);
    Playedrack_getrack(pld, rack);

    for (i = 0; i < strlen(tochange); i++)
    {
        /* Transform into a tile */
        tile = chartocode(tochange[i]);
        if (islower(tochange[i]))
            tile = JOKER_TILE;

        /* Remove it from the rack */
        if (! Rack_in(rack, tile))
        {
            Rack_destroy(rack);
            return 2;
        }
        Rack_remove(rack, tile);
    }

    Playedrack_empty(pld);
    Playedrack_setold(pld, rack);

    // FIXME: the letters to change should not be in the bag while generating
    // the new rack!

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_freegame_endturn(game);

    return 0;
}


int
Game_duplicate_play(Game game, const char coord[COOR_SIZE_MAX], const char *word)
{
    int res;
    Round round;

    /* Perform all the validity checks, and fill a round */
    round = Round_create();
    res = Game_checkplayedword(game, coord, word, round);
    if (res != 0)
    {
        Round_destroy(round);
        return res;
    }

    /* Everything is OK, we can play the word */
    Game_duplicate_playround(game, round, game->currplayer);
    Round_destroy(round);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_duplicate_endturn(game);

    return 0;
}


static int
Game_duplicate_ai(Game game, int n)
{
    Round round;
    Player player;

    PDEBUG(n < 0 || n >= game->nplayers, "GAME: wrong player number\n");
    player = game->players[n];

    PDEBUG(! Player_ai(player), "GAME: AI requested for a human player!\n");

    Player_ai_search(player, game->dic, game->board, game->nrounds);
    round = Player_ai_bestround(player);
    if (round == NULL)
    {
        /* The AI player has nothing to play.
         * XXX: Is it even possible? */
        PDEBUG(1, "GAME: Yes, this is possible...");
        return 1;
    }
    else
    {
        Game_duplicate_playround(game, round, n);
    }

    return 0;
}


int
Game_duplicate_start(Game game)
{
    int res, i;
    Playedrack pld, pldtmp;

    if (game->nplayers == 0)
        return 1;

    game->mode = DUPLICATE;

    game->currplayer = 0;

    /* XXX: code similar with endturn() */
    /* Complete the rack for the player that just played */
    res = Game_duplicate_setrackrandom(game, 0, 1, RACK_NEW);
    /* End of the game? */
    if (res == 1)
    {
        Game_duplicate_end(game);
        return 1;
    }

    pld = Player_getplayedrack(game->players[game->currplayer]);
    /* All the players have the same rack */
    for (i = 0; i < game->nplayers; i++)
    {
        if (i != game->currplayer)
        {
            pldtmp = Player_getplayedrack(game->players[i]);
            Playedrack_copy(pldtmp, pld);
        }
        /* Nobody has played yet in this round */
        Player_setstatus(game->players[i], TO_PLAY);
    }

    /* History of the racks */
    Playedrack_copy(game->playedracks[game->nrounds],
                    Player_getplayedrack(game->players[game->currplayer]));

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    Game_duplicate_endturn(game);

    return 0;
}


/*
 * This function does not terminate the turn itself, but performs some
 * checks to know whether or not it should be terminated (with a call to
 * Game_duplicate_endturnforreal()).
 *
 * For the turn to be terminated, all the players must have played.
 * Since the AI players play after the human players, we check whether
 * one of the human players has not played yet:
 *   - if so, we have nothing to do (we are waiting for him)
 *   - if not (all human players have played), the AI players can play,
 *     and we finish the turn.
 */
void
Game_duplicate_endturn(Game game)
{
    int i;
    for (i = 0; i < game->nplayers; i++)
    {
        if (! Player_ai(game->players[i]) &&
            Player_getstatus(game->players[i]) == TO_PLAY)
        {
            /* A human player has not played... */
            return;
        }
    }

    /* If all the human players have played */
    if (i == game->nplayers)
    {
        /* Make AI players play their turn */
        for (i = 0; i < game->nplayers; i++)
        {
            if (Player_ai(game->players[i]))
            {
                // XXX: handle the return value?
                if (Game_duplicate_ai(game, i))
                    return;
            }
        }

        /* Next turn */
        Game_duplicate_endturnforreal(game);
    }
}


static void
Game_duplicate_playround(Game game, Round round, int n)
{
    Player player;

    PDEBUG(n < 0 || n >= game->nplayers, "GAME: wrong player number\n");
    player = game->players[n];

    /* Update the rack and the score of the current player */
    Player_addpoints(player, Round_points(round));
    Player_endturn(player, round, game->nrounds);

    Player_setstatus(player, PLAYED);
}


/*
 * This function really changes the turn, i.e. the best word is played and
 * a new rack is given to the players.
 * We suppose that all the players have finished to play for this turn (this
 * should have been checked by Game_duplicate_endturn())
 */
static int
Game_duplicate_endturnforreal(Game game)
{
    int res, i, imax;
    Playedrack pld, pldtmp;

    /* Play the best word on the board */
    imax = 0;
    for (i = 1; i < game->nplayers; i++)
    {
        if (Round_points(Player_getlastround(game->players[i])) >
            Round_points(Player_getlastround(game->players[imax])))
        {
            imax = i;
        }
    }
    Game_helper_playround(game, Player_getlastround(game->players[imax]));

    /* Complete the rack for the player that just played */
    res = Game_duplicate_setrackrandom(game, imax, 1, RACK_NEW);
    /* End of the game? */
    if (res == 1)
    {
        Game_duplicate_end(game);
        return 1;
    }

    pld = Player_getplayedrack(game->players[imax]);
    /* All the players have the same rack */
    for (i = 0; i < game->nplayers; i++)
    {
        if (i != imax)
        {
            pldtmp = Player_getplayedrack(game->players[i]);
            Playedrack_copy(pldtmp, pld);
        }
        /* Nobody has played yet in this round */
        Player_setstatus(game->players[i], TO_PLAY);
    }

    /* History of the racks */
    Playedrack_copy(game->playedracks[game->nrounds], pld);

    /* XXX: Little hack to handle the games with only AI players.
     * This will have no effect when there is at least one human player */
    Game_duplicate_endturn(game);

    return 0;
}


static void
Game_duplicate_end(Game game)
{
    // TODO
}


int
Game_duplicate_setplayer(Game game, int n)
{
    if (n < 0 || n >= game->nplayers)
        return 1;
    /* Forbid switching to an AI player */
    if (Player_ai(game->players[n]))
        return 2;

    game->currplayer = n;
    return 0;
}

