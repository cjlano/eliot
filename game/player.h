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

/* $Id: player.h,v 1.1 2004/08/07 18:25:03 ipkiss Exp $ */

#ifndef _PLAYER_H_
#define _PLAYER_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Number of played words.
 * There are only 102 tiles, so we should be safe even if only one tile is
 * played at each turn... */
#define PLAYEDRACK_MAX 102

  /*************************
   * A player can be a human player, or a computer player (AI)
   *************************/

typedef struct tplayer *Player;

  /*************************
   * Player general routines
   *************************/

Player Player_create        (void);
void   Player_inithuman     (Player);
void   Player_initai        (Player);
void   Player_destroy       (Player);

  /**************************
   * General getters
   **************************/

Playedrack  Player_getplayedrack (Player);
Playedrack  Player_getlastrack   (Player);
Round       Player_getlastround  (Player);

  /**************************
   *
   **************************/

void        Player_endturn       (Player,Round,int);

  /**************************
   * Add (or remove, if the given value is negative) points
   * to the player's score.
   **************************/

void        Player_addpoints     (Player,int);
int         Player_getpoints     (Player);

  /**************************
   *
   **************************/

typedef enum {PLAYED, TO_PLAY} play_status;
void        Player_setstatus     (Player,play_status);
play_status Player_getstatus     (Player);

  /**************************
   * AI (Artificial Intelligence) handling
   * The int argument of Player_ai_search() is the 'turn' number
   * (starting from 0)
   * Note: we could implement various strategies:
   *   - best: play the word with the best score (current implementation)
   *   - second: play the word with the second best score (strictly lower than
   *        the best one)
   *   - random: randomly choose one of the possible words
   *   - handicap(p): in the array of the n possible words (sorted by
   *        decreasing scores), play the word number i, where i/n is nearest
   *        from a predefined percentage p.
   *        So 'handicap(0)' should be equivalent to 'best'.
   *        This strategy makes an interesting opponent, because you can adapt
   *        it to your level, with a careful choice of the p value.
   *
   * In fact, instead of working on the score of the words, these strategies
   * could work on any other value. In particular, some heuristics could
   * modulate the score with a value indicating the openings offered by the
   * word (if a word makes accessible a "word counts triple" square, it is less
   * interesting than another word with the same score or even with a slightly
   * lower score, but which does not offer such a square).
   * 
   * More evolved heuristics could even take into account the remaining letters
   * in the bag to guess the 'statistical rack' of the opponent, and play a
   * word both maximizing the score and minimizing the opponent's score...
   * Hmmm... i don't think this one will be implemented in a near future :)
   **************************/

int     Player_ai             (Player);
int     Player_ai_search      (Player,Dictionary,Board,int);
Round   Player_ai_bestround   (Player); // XXX: useful?
Results Player_ai_getresults  (Player);

#if defined(__cplusplus)
       }
#endif
#endif


