/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2013 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#ifndef NCURSES_H_
#define NCURSES_H_

#include "config.h"

#if defined HAVE_NCURSESW_CURSES_H
#   include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#   include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#   include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#   include <ncurses.h>
#elif defined HAVE_CURSES_H
#   include <curses.h>
#else
//  Should not happen, if configure did its job correctly
#   error "No header to include for ncursesw. Bad configure detection?"
#endif

#include <string>

#include "logging.h"

class PublicGame;

using std::string;
using std::wstring;


class Box
{
    DEFINE_LOGGER();

    public:
        // Create a titled box with the specified position and size,
        // containing iHeadingLines non-scrolling lines.
        // The number of data to display (with the printLine() method)
        // can be set later using the setDataSize() method.
        Box(WINDOW *win, int y, int x, int h, int w,
            unsigned int iHeadingLines = 0);

        // Simply draw the box (without any content)
        void draw(const string& iTitle = "") const;

        // Print data line number n (starting at 0), taking care of
        // the current scrolling state
        void printDataLine(int n, int x, const char *fmt, ...) const;

        // Set the number of lines of the data to display
        void setDataSize(unsigned int iNbLines) { m_dataSize = iNbLines; }

        // Control scrolling
        // Return true if redrawing is needed, false otherwise
        bool scrollOneLineUp();
        bool scrollOneLineDown();
        bool scrollOnePageUp();
        bool scrollOnePageDown();
        bool scrollBeginning();
        bool scrollEnd();

        // Clear the box completely
        void clear() const { clearRect(m_win, m_y, m_x, m_h, m_w); }
        // Clear the scrolling zone of the box
        void clearData() const { clearRect(m_win, m_topLine, m_x + 1,
                                           m_nbLines, getWidth()); }
        // Clear an arbitrary rectangular zone
        static void clearRect(WINDOW *win, int y, int x, int h, int w);

        // First line of data to display (included)
        int getFirstLine() const { return m_dataStart; }
        // Last line of data to display (excluded)
        int getLastLine() const { return m_dataStart + m_nbLines; }

        // First line inside the box
        int getTop() const { return m_y + 1; }
        // First column available for writing
        int getLeft() const { return m_x + 1; }
        // Client width
        int getWidth() const { return m_w - 2; }

    private:
        WINDOW *m_win;
        int m_x;
        int m_y;
        int m_w;
        int m_h;
        string m_title;
        int m_topLine;
        int m_nbLines;
        int m_dataStart;
        int m_dataSize;
};


/**
 * This class implements the ncurses interface.
 */
class CursesIntf
{
    DEFINE_LOGGER();
public:
    // Pre-requisite: the given Game object MUST have been allocated with new
    // (in particular: not on the stack)
    // This class also takes the responsability of destroying the Game object.
    CursesIntf(WINDOW *win, PublicGame& iGame);
    ~CursesIntf();
    bool isDying() const { return m_dying; }
    int handleKey(int iKey);
    void redraw(WINDOW *win);

private:
    enum State
    {
        DEFAULT,    // Default state
        HELP,       // Help panel is shown
        HISTORY,    // Game history panel is shown
        RESULTS,    // Search results panel is shown
        BAG         // Bag contents panel is shown
    };

    // Write a message in the "status line"
    void drawStatus(WINDOW *win, const string& iMessage, bool error = true);
    // Draw the board, with the coordinates
    void drawBoard(WINDOW *win, int y, int x) const;
    // Draw the boxes for scores and racks
    void drawScoresRacks(WINDOW *win, int y, int x) const;
    // Draw the results panel
    void drawResults(Box &ioBox) const;
    // Draw the history panel
    void drawHistory(Box &ioBox) const;
    // Draw the help panel
    void drawHelp(Box &ioBox) const;
    // Draw the bag panel
    void drawBag(Box &ioBox) const;

    // Change the inner state, and initialize the corresponding box
    void setState(State iState);
    // Draw the "Play word" box, and handle the played word
    void playWord(WINDOW *win, int y, int x);
    void checkWord(WINDOW *win, int y, int x);
    void saveGame(WINDOW *win, int y, int x);
    void loadGame(WINDOW *win, int y, int x);
    void passTurn(WINDOW *win, int y, int x, PublicGame &iGame);
    void setRack(WINDOW *win, int y, int x, PublicGame &iGame);

    // Get a string from the user, with a maximum length
    // The string is validated if the user presses Enter (return value: true)
    // and it is cancelled if the user presses Esc (return value: false)
    bool readString(WINDOW *win, int y, int x, int n, wstring &oString,
                    unsigned int flag = 0);
    // Any combination of the following constants can be used as the "flag"
    // parameter of the readString() method.
    // Indicate that the '?' character is accepted
    static const unsigned int kJOKER = 1 << 0;
    // Accept characters for a file name
    static const unsigned int kFILENAME = 1 << 1;

    // Handle the key in Training mode
    int handleKeyForTraining(int iKey, PublicGame &iGame);
    // Handle the key in Duplicate mode
    int handleKeyForDuplicate(int iKey, PublicGame &iGame);
    // Handle the key in FreeGame mode
    int handleKeyForFreeGame(int iKey, PublicGame &iGame);

    // Main window for drawing
    WINDOW *m_win;
    // Current game
    // Invariant: the pointer will always point to a valid PublicGame object
    PublicGame *m_game;
    // Interface state
    State m_state;
    // True when the user requested to quit
    bool m_dying;
    // Scrolling box for the current panel
    Box m_box;
    // True if dots must be shown on empty squares
    bool m_showDots;
};

#endif

