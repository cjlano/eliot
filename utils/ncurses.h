/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: ncurses.h,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _NCURSES_H_
#define _NCURSES_H_

#include <curses.h>
#include <string>

class Game;
class Training;
class Duplicate;
class FreeGame;

using std::string;


class CursesIntf
{
public:
    CursesIntf(WINDOW *win, Game& iGame);
    bool isDying() const { return m_dying; }
    int handleKey(int iKey);
    void redraw(WINDOW *win);

private:
    enum State
    {
        DEFAULT,    // Default state
        HELP,       // Help panel is shown
        HISTORY,    // Game history panel is shown
        RESULTS     // Search results panel is shown
    };
    // Draw a titled box with the specified position and size
    static void drawBox(WINDOW *win, int y, int x, int h, int w,
                        const string& iTitle);
    // Clear a rectangular zone
    static void clearRect(WINDOW *win, int y, int x, int h, int w);
    // Print a line in a box, taking care of the current offset
    void boxPrint(WINDOW *win, int y, int x, const char *fmt, ...);
    // Write a message in the "status line"
    void drawStatus(WINDOW *win, int y, int x,
                    const string& iMessage, bool error = true);
    // Draw the board, with the coordinates
    void drawBoard(WINDOW *win, int y, int x) const;
    // Draw the boxes for scores and racks
    void drawScoresRacks(WINDOW *win, int y, int x) const;
    // Draw the results panel
    void drawResults(WINDOW *win, int y, int x);
    // Draw the history panel
    void drawHistory(WINDOW *win, int y, int x);
    // Draw the help panel
    void drawHelp(WINDOW *win, int y, int x);
    // Draw the "Play word" box, and handle the played word
    void playWord(WINDOW *win, int y, int x);
    void checkWord(WINDOW *win, int y, int x);
    void passTurn(WINDOW *win, int y, int x, FreeGame &iGame);
    void setRack(WINDOW *win, int y, int x, Training &iGame);
    // Get a string from the user, with a maximum length
    // The string is validated if the user presses Enter (return value: true)
    // and it is cancelled if the user presses Esc (return value: false)
    bool readString(WINDOW *win, int y, int x, int n, string &oString,
                    unsigned int flag = 0);
    // Any combination of the following constants can be used as the "flag"
    // parameter of the readString() method.
    // Indicate that the '?' character is accepted
    static const unsigned int kJOKER = 1 << 0;

    // Handle the key in Training mode
    int handleKeyForGame(int iKey, Training &iGame);
    // Handle the key in Duplicate mode
    int handleKeyForGame(int iKey, Duplicate &iGame);
    // Handle the key in FreeGame mode
    int handleKeyForGame(int iKey, FreeGame &iGame);

    // Main window for drawing
    WINDOW *m_win;
    // Current game;
    Game& m_game;
    // Interface state
    State m_state;
    // True when the user requested to quit
    bool m_dying;
    // Index of the first line of data to be displayed in the current box
    int m_boxStart;
    // Number of lines of the current box (border excluded)
    int m_boxLines;
    // Number of lines of the data to be displayed in the current box
    int m_boxLinesData;
    // Index of the first line of the box where to write
    int m_boxY;
    // True if dots must be shown on empty squares
    bool m_showDots;
};

#endif

