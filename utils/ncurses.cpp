/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: ncurses.cpp,v 1.3 2005/02/07 22:20:32 ipkiss Exp $
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

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include <ctype.h>

#include "ncurses.h"
#include "dic.h"
#include "dic_search.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"

using namespace std;


CursesIntf::CursesIntf(WINDOW *win, Game& iGame)
    : m_win(win), m_game(iGame), m_state(DEFAULT), m_dying(false),
    m_boxStart(0), m_boxLines(0), m_boxLinesData(0), m_boxY(0),
    m_showDots(false)
{
}


void CursesIntf::drawBox(WINDOW *win, int y, int x, int h, int w,
                         const string& iTitle)
{
    if (w > 3 && h > 2)
    {
        int i_len = iTitle.size();

        if (i_len > w - 2) i_len = w - 2;

        mvwaddch(win, y, x,    ACS_ULCORNER);
        mvwhline(win, y, x+1,  ACS_HLINE, ( w-i_len-2)/2);
        mvwprintw(win,y, x+1+(w-i_len-2)/2, "%s", iTitle.c_str());
        mvwhline(win, y, x+(w-i_len)/2+i_len,
                 ACS_HLINE, w - 1 - ((w-i_len)/2+i_len));
        mvwaddch(win, y, x+w-1,ACS_URCORNER);

        mvwvline(win, y+1, x,     ACS_VLINE, h-2);
        mvwvline(win, y+1, x+w-1, ACS_VLINE, h-2);

        mvwaddch(win, y+h-1, x,     ACS_LLCORNER);
        mvwhline(win, y+h-1, x+1,   ACS_HLINE, w - 2);
        mvwaddch(win, y+h-1, x+w-1, ACS_LRCORNER);
    }
}


void CursesIntf::clearRect(WINDOW *win, int y, int x, int h, int w)
{
    for (int i = 0; i < h; i++)
    {
        mvwhline(win, y + i, x, ' ', w);
    }
}


void CursesIntf::boxPrint(WINDOW *win, int y, int x, const char *fmt, ...)
{
    if (y < m_boxStart || y - m_boxStart >= m_boxLines)
        return;

    va_list vl_args;
    char *buf = NULL;
    va_start(vl_args, fmt);
    vasprintf(&buf, fmt, vl_args);
    va_end(vl_args);

    if (buf == NULL)
    {
        return;
    }
    mvwprintw(win, m_boxY + y - m_boxStart, x, "%s", buf);
}


void CursesIntf::drawStatus(WINDOW *win, int y, int x,
                            const string& iMessage, bool error)
{
    if (error)
        wattron(win, COLOR_PAIR(COLOR_YELLOW));
    mvwprintw(win, y, x, iMessage.c_str());
    whline(win, ' ', COLS - x - 1 - iMessage.size());
    if (error)
        wattron(win, COLOR_PAIR(COLOR_WHITE));
}


void CursesIntf::drawBoard(WINDOW *win, int y, int x) const
{
    // Box around the board
    drawBox(win, y + 1, x + 3, 17, 47, "");

    // Print the coordinates
    for (int i = 0; i < 15; i++)
    {
        mvwaddch(win, y + i + 2, x + 1,  'A' + i);
        mvwaddch(win, y + i + 2, x + 51, 'A' + i);
        mvwprintw(win, y,      x + 3 * i + 5, "%d", i + 1);
        mvwprintw(win, y + 18, x + 3 * i + 5, "%d", i + 1);
    }

    // The board itself
    for (int row = 1; row < 16; row++)
    {
        for (int col = 1; col < 16; col++)
        {
            // Handle colors
            int wm = m_game.getBoardWordMultiplier(row, col);
            int lm = m_game.getBoardLetterMultiplier(row, col);
            if (wm == 3)
                wattron(win, COLOR_PAIR(COLOR_RED));
            else if (wm == 2)
                wattron(win, COLOR_PAIR(COLOR_MAGENTA));
            else if (lm == 3)
                wattron(win, COLOR_PAIR(COLOR_BLUE));
            else if (lm == 2)
                wattron(win, COLOR_PAIR(COLOR_CYAN));
            else
                wattron(win, COLOR_PAIR(COLOR_WHITE));

            // Empty square
            mvwprintw(win, y + row + 1, x + 3 * col + 1, "   ");

            // Now add the letter
            char c = m_game.getBoardChar(row, col);
            if (c)
            {
                if (islower(c))
                    mvwaddch(win, y + row + 1, x + 3 * col + 2,
                             c | A_BOLD | COLOR_PAIR(COLOR_GREEN));
                else
                    mvwaddch(win, y + row + 1, x + 3 * col + 2, c);
            }
            else
            {
                // Empty square... should we display a dot?
                if (m_showDots)
                    mvwaddch(win, y + row + 1, x + 3 * col + 2, '.');
            }
        }
    }
    wattron(win, COLOR_PAIR(COLOR_WHITE));
}


void CursesIntf::drawScoresRacks(WINDOW *win, int y, int x) const
{
    drawBox(win, y, x, m_game.getNPlayers() + 2, 25, _(" Scores "));
    for (int i = 0; i < m_game.getNPlayers(); i++)
    {
        if (m_game.getMode() != Game::kTRAINING && i == m_game.currPlayer())
            attron(A_BOLD);
        mvwprintw(win, y + i + 1, x + 2,
                  _("Player %d: %d"), i, m_game.getPlayerPoints(i));
        if (m_game.getMode() != Game::kTRAINING && i == m_game.currPlayer())
            attroff(A_BOLD);
    }

    // Distance between the 2 boxes
    int yOff = m_game.getNPlayers() + 3;

    drawBox(win, y + yOff, x, m_game.getNPlayers() + 2, 25, _(" Racks "));
    for (int i = 0; i < m_game.getNPlayers(); i++)
    {
        if (m_game.getMode() != Game::kTRAINING && i == m_game.currPlayer())
            attron(A_BOLD);
        mvwprintw(win, y + yOff + i + 1, x + 2,
                  _("Player %d: %s"), i, m_game.getPlayerRack(i).c_str());
        if (m_game.getMode() != Game::kTRAINING && i == m_game.currPlayer())
            attroff(A_BOLD);
        // Force to refresh the whole rack
        whline(win, ' ', 7 - m_game.getPlayerRack(i).size());
    }

    // Display a message when the search is complete
    if (m_game.getMode() == Game::kTRAINING && m_game.getNResults())
        mvwprintw(win, y + 2*yOff - 1, x + 2, _("Search complete"));
    else
        mvwhline(win, y + 2*yOff - 1, x + 2, ' ', strlen(_("Search complete")));
}


void CursesIntf::drawResults(WINDOW *win, int y, int x)
{
    int h = 17;
    drawBox(win, y, x, h, 25, _(" Search results "));
    m_boxY = y + 1;
    m_boxLines = h - 2;
    m_boxLinesData = m_game.getNResults();

    int i;
    for (i = m_boxStart; i < m_game.getNResults() &&
                         i < m_boxStart + m_boxLines; i++)
    {
        string coord = m_game.getSearchedCoords(i);
        boxPrint(win, i, x + 1, "%3d %s%s %3s",
                 m_game.getSearchedPoints(i),
                 m_game.getSearchedWord(i).c_str(),
                 string(h - 3 - m_game.getSearchedWord(i).size(), ' ').c_str(),
                 coord.c_str());
    }
    // Complete the list with empty lines, to avoid trails
    for (; i < m_boxStart + m_boxLines; i++)
    {
        boxPrint(win, i, x + 1, string(23, ' ').c_str());
    }
}


void CursesIntf::drawHistory(WINDOW *win, int y, int x)
{
    // To allow pseudo-scrolling, without leaving trails
    clear();

    drawBox(win, y, x, LINES - y, COLS - x, _(" History of the game "));
    m_boxY = y + 1;
    m_boxLines = LINES - y - 2;
    m_boxLinesData = m_game.getNRounds();

    // Heading
    boxPrint(win, m_boxStart, x + 2,
             _(" N |   RACK   |    SOLUTION     | REF | PTS | P | BONUS"));
    mvwhline(win, y + 2, x + 2, ACS_HLINE, 55);

    int i;
    for (i = m_boxStart + 0; i < m_game.getNRounds() &&
                         i < m_boxStart + m_boxLines; i++)
    {
        string word = m_game.getPlayedWord(i);
        string coord = m_game.getPlayedCoords(i);
        boxPrint(win, i + 2, x + 2,
                 "%2d   %8s   %s%s   %3s   %3d   %1d   %c",
                 i + 1, m_game.getPlayedRack(i).c_str(), word.c_str(),
                 string(15 - word.size(), ' ').c_str(),
                 coord.c_str(), m_game.getPlayedPoints(i),
                 m_game.getPlayedPlayer(i),
                 m_game.getPlayedBonus(i) ? '*' : ' ');
    }
    mvwvline(win, y + 1, x + 5,  ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
    mvwvline(win, y + 1, x + 16, ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
    mvwvline(win, y + 1, x + 34, ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
    mvwvline(win, y + 1, x + 40, ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
    mvwvline(win, y + 1, x + 46, ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
    mvwvline(win, y + 1, x + 50, ACS_VLINE, min(i + 2 - m_boxStart, m_boxLines));
}


void CursesIntf::drawHelp(WINDOW *win, int y, int x)
{
    // To allow pseudo-scrolling, without leaving trails
    clear();

    drawBox(win, y, x, LINES - y, COLS - x, _(" Help "));
    m_boxY = y + 1;
    m_boxLines = LINES - y - 2;

    int n = 0;
    boxPrint(win, n++, x + 2, _("[Global]"));
    boxPrint(win, n++, x + 2, _("   h, H, ?          Show/hide help box"));
    boxPrint(win, n++, x + 2, _("   y, Y             Show/hide history of the game"));
    boxPrint(win, n++, x + 2, _("   e, E             Show/hide dots on empty squares of the board"));
    boxPrint(win, n++, x + 2, _("   q, Q             Quit"));
    boxPrint(win, n++, x + 2, _("   d, D             Check the existence of a word in the dictionary"));
    boxPrint(win, n++, x + 2, _("   j, J             Play a word"));
    boxPrint(win, n++, x + 2, "");

    boxPrint(win, n++, x + 2, _("[Training mode]"));
    boxPrint(win, n++, x + 2, _("   *                Take a random rack"));
    boxPrint(win, n++, x + 2, _("   +                Complete the current rack randomly"));
    boxPrint(win, n++, x + 2, _("   t, T             Set the rack manually"));
    boxPrint(win, n++, x + 2, _("   s, S             Search (compute all the possible words)"));
    boxPrint(win, n++, x + 2, _("   r, R             Show/hide search results"));
    boxPrint(win, n++, x + 2, "");

    boxPrint(win, n++, x + 2, _("[Duplicate mode]"));
    boxPrint(win, n++, x + 2, _("   n, N             Switch to the next human player"));
    boxPrint(win, n++, x + 2, "");

    boxPrint(win, n++, x + 2, _("[Free game mode]"));
    boxPrint(win, n++, x + 2, _("   p, P             Pass your turn (with or without changing letters)"));
    boxPrint(win, n++, x + 2, "");

    boxPrint(win, n++, x + 2, _("[Miscellaneous]"));
    boxPrint(win, n++, x + 2, _("   <up>, <down>     Navigate in a box line by line"));
    boxPrint(win, n++, x + 2, _("   <pgup>, <pgdown> Navigate in a box page by page"));
    boxPrint(win, n++, x + 2, _("   Ctrl-l           Refresh the screen"));

    m_boxLinesData = n;
}


void CursesIntf::playWord(WINDOW *win, int y, int x)
{
    drawBox(win, y, x, 4, 32, _(" Play a word "));
    mvwprintw(win, y + 1, x + 2, _("Played word:"));
    mvwprintw(win, y + 2, x + 2, _("Coordinates: "));
    wrefresh(win);

    string word, coord;
    int xOff = strlen(_("Coordinates: ")) + 2;
    if (readString(win, y + 1, x + xOff, 15, word) &&
        readString(win, y + 2, x + xOff, 3, coord))
    {
        int res = m_game.play(coord, word);
        if (res)
        {
            drawStatus(win, LINES - 1, 0, _("Incorrect or misplaced word"));
        }
    }
    m_state = DEFAULT;
    clearRect(win, y, x, 4, 32);
}


void CursesIntf::checkWord(WINDOW *win, int y, int x)
{
    drawBox(win, y, x, 4, 32, _(" Dictionary "));
    mvwprintw(win, y + 1, x + 2, _("Enter the word to check:"));
    wrefresh(win);

    string word;
    if (readString(win, y + 2, x + 2, 15, word))
    {
        int res = Dic_search_word(m_game.getDic(), word.c_str());
        char s[100];
        if (res)
            snprintf(s, 100, _("The word '%s' exists"), word.c_str());
        else
            snprintf(s, 100, _("The word '%s' does not exist"), word.c_str());
        drawStatus(win, LINES - 1, 0, s);
    }
    m_state = DEFAULT;
    clearRect(win, y, x, 4, 32);
}


void CursesIntf::passTurn(WINDOW *win, int y, int x, FreeGame &iGame)
{
    drawBox(win, y, x, 4, 32, _(" Pass your turn "));
    mvwprintw(win, y + 1, x + 2, _("Enter the letters to change:"));
    wrefresh(win);

    string letters;
    if (readString(win, y + 2, x + 2, 7, letters))
    {
        int res = iGame.pass(letters, m_game.currPlayer());
        if (res)
        {
            drawStatus(win, LINES - 1, 0, _("Cannot pass the turn"));
        }
    }
    m_state = DEFAULT;
    clearRect(win, y, x, 4, 32);
}


void CursesIntf::setRack(WINDOW *win, int y, int x, Training &iGame)
{
    drawBox(win, y, x, 4, 32, _(" Set rack "));
    mvwprintw(win, y + 1, x + 2, _("Enter the new letters:"));
    wrefresh(win);

    string letters;
    if (readString(win, y + 2, x + 2, 7, letters, kJOKER))
    {
        iGame.setRackManual(false, letters);
    }
    m_state = DEFAULT;
    clearRect(win, y, x, 4, 32);
}


bool CursesIntf::readString(WINDOW *win, int y, int x, int n, string &oString,
                            unsigned int flag)
{
    int c;
    wmove(win, y, x);
    curs_set(1);
    while ((c = getch()) != 0)
    {
        if (c == 0x1b )  // Esc
        {
            curs_set(0);
            return false;
        }
        else if (c == KEY_ENTER || c == 0xD)
        {
            curs_set(0);
            return true;
        }
        else if (c == 0x0c)  // Ctrl-L
        {
//             clear();
            redraw(win);
            wmove(win, y, x);
        }
        else if (c == KEY_BACKSPACE && oString.size() > 0)
        {
            x--;
            mvwprintw(win, y, x, " ");
            wmove(win, y, x);
            oString.erase(oString.size() - 1);
        }
        else if (isalnum(c) && oString.size() < (unsigned int)n)
        {
            mvwprintw(win, y, x, "%c", c);
            x++;
            oString += (char)c;
        }
        else if (flag & kJOKER && c == '?')
        {
            mvwprintw(win, y, x, "%c", c);
            x++;
            oString += (char)c;
        }
//         else
//             mvwprintw(win, 0, 0, "%3d", c);
    }
    curs_set(0);
    return 0;
}


int CursesIntf::handleKeyForGame(int iKey, Training &iGame)
{
    switch (iKey)
    {
        case '*':
            iGame.setRackRandom(0, false, Game::RACK_ALL);
            return 1;

        case '+':
            iGame.setRackRandom(0, false, Game::RACK_NEW);
            return 1;

        case 't':
        case 'T':
            setRack(m_win, 22, 10, iGame);
            return 1;

        case 's':
        case 'S':
            iGame.search();
            return 1;

        default:
            return 2;
    }
}


int CursesIntf::handleKeyForGame(int iKey, Duplicate &iGame)
{
    switch (iKey)
    {
        case 'n':
        case 'N':
            iGame.nextHumanPlayer();
            return 1;

        default:
            return 2;
    }
}


int CursesIntf::handleKeyForGame(int iKey, FreeGame &iGame)
{
    switch (iKey)
    {
        case 'p':
        case 'P':
            passTurn(m_win, 22, 10, iGame);
            return 1;

        default:
            return 2;
    }
}


int CursesIntf::handleKey(int iKey)
{
    if (m_state == DEFAULT)
    {
        int res;
        if (m_game.getMode() == Game::kTRAINING)
        {
            res = handleKeyForGame(iKey, (Training&)m_game);
        }
        else if (m_game.getMode() == Game::kDUPLICATE)
        {
            res = handleKeyForGame(iKey, (Duplicate&)m_game);
        }
        else
        {
            res = handleKeyForGame(iKey, (FreeGame&)m_game);
        }

        if (res != 2)
            return res;
    }
    else // m_state is in {HELP, RESULTS, HISTORY}
    {
        switch (iKey)
        {
            case KEY_HOME:
                if (m_boxLinesData <= m_boxLines && m_boxStart > 0)
                    return 0;
                m_boxStart = 0;
                return 1;
            case KEY_END:
                if (m_boxLinesData <= m_boxLines &&
                    m_boxStart < m_boxLinesData - 1)
                    return 0;
                m_boxStart = m_boxLinesData - 1;
                return 1;
            case KEY_UP:
                if (m_boxLinesData <= m_boxLines || m_boxStart <= 0)
                    return 0;
                m_boxStart--;
                return 1;
            case KEY_DOWN:
                if (m_boxLinesData <= m_boxLines ||
                    m_boxStart >= m_boxLinesData - 1)
                    return 0;
                m_boxStart++;
                return 1;
            case KEY_PPAGE:
                if (m_boxLinesData <= m_boxLines)
                    return 0;
                m_boxStart -= m_boxLines;
                if (m_boxStart < 0)
                    m_boxStart = 0;
                return 1;
            case KEY_NPAGE:
                if (m_boxLinesData <= m_boxLines)
                    return 0;
                m_boxStart += m_boxLines;
                if (m_boxStart > m_boxLinesData - 1)
                    m_boxStart = m_boxLinesData - 1;
                return 1;
        }
    }

    switch (iKey)
    {
        // Toggle help
        case 'h':
        case 'H':
        case '?':
            if (m_state == HELP)
                m_state = DEFAULT;
            else
                m_state = HELP;
            m_boxStart = 0;
            clear();
            return 1;

        // Toggle history
        case 'y':
        case 'Y':
            if (m_state == HISTORY)
                m_state = DEFAULT;
            else
                m_state = HISTORY;
            m_boxStart = 0;
            clear();
            return 1;

        // Toggle results (training mode only)
        case 'r':
        case 'R':
            if (m_game.getMode() != Game::kTRAINING)
                return 0;
            if (m_state == RESULTS)
                m_state = DEFAULT;
            else
                m_state = RESULTS;
            m_boxStart = 0;
            clear();
            return 1;

        // Toggle dots display
        case 'e':
        case 'E':
            m_showDots = !m_showDots;
            return 1;

        // Check a word in the dictionary
        case 'd':
        case 'D':
            if (m_state != DEFAULT)
                return 0;
            checkWord(m_win, 22, 10);
            return 1;

        // Play a word
        case 'j':
        case 'J':
            if (m_state != DEFAULT)
                return 0;
            playWord(m_win, 22, 10);
            return 1;

        // Ctrl-L should clear and redraw the screen
        case 0x0c:
            clear();
            return 1;

        // Quit
        case 'q':
        case 'Q':
        case 0x1b: // Esc
            m_dying = true;
            return 0;

        default:
            return 0;
    }
}


void CursesIntf::redraw(WINDOW *win)
{
    if (m_state == DEFAULT)
    {
        drawScoresRacks(win, 3, 54);
        drawBoard(win, 2, 0);
    }
    else if (m_state == RESULTS)
    {
        drawResults(win, 3, 54);
        drawBoard(win, 2, 0);
    }
    else if (m_state == HELP)
    {
        drawHelp(win, 1, 0);
    }
    else if (m_state == HISTORY)
    {
        drawHistory(win, 1, 0);
    }

    // Title
    attron(A_REVERSE);
    string mode;
    if (m_game.getMode() == Game::kTRAINING)
        mode = _("Training mode");
    else if (m_game.getMode() == Game::kFREEGAME)
        mode = _("Free game mode");
    else if (m_game.getMode() == Game::kDUPLICATE)
        mode = _("Duplicate mode");
    string title = "Eliot (" + mode + ") " + _("[h for help]");
    mvwprintw(win, 0, 0, title.c_str());
    whline(win, ' ', COLS - title.size());
    attroff(A_REVERSE);

    wrefresh(win);
}


int main(int argc, char ** argv)
{
#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
    // Set the message domain
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    // Initialize the ncurses library
    WINDOW *wBoard = initscr();
    keypad( wBoard, true );
    // Take input chars one at a time
    cbreak();
    // Do not do NL -> NL/CR
    nonl();
    // Hide the cursor
    curs_set(0);

    if (has_colors())
    {
        start_color();

        // Simple color assignment
        init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);

        init_pair(COLOR_BLUE, COLOR_BLACK, COLOR_BLUE);
        init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
        init_pair(COLOR_MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(COLOR_RED, COLOR_BLACK, COLOR_RED);
    }

    char dic_path[100];
    Dictionary dic = NULL;
    srand(time(NULL));

    if (argc != 2)
    {
        endwin();
        fprintf(stdout, _("Usage: eliotcurses /path/to/ods4.dawg\n"));
        exit(1);
    }
    else
        strcpy(dic_path, argv[1]);
    if (Dic_load(&dic, dic_path))
        return -1;

    FreeGame game(dic);
    game.addHumanPlayer();
    game.addAIPlayer();
//     game.addAIPlayer();
//     game.addAIPlayer();
    game.start();

    // Do not echo
    noecho();

    CursesIntf mainIntf(wBoard, game);
    mainIntf.redraw(wBoard);

    while (! mainIntf.isDying())
    {
        int c = getch();
        if (mainIntf.handleKey(c) == 1)
        {
            mainIntf.redraw(wBoard);
        }
    }

    Dic_destroy(dic);

    delwin(wBoard);

    // Exit the ncurses library
    endwin();

    return 0;
}
