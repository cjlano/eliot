/*****************************************************************************
 * Copyright (C) 2005-2008 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

// Needed for Mac OS X, apparently (otherwise get_wch is not defined)
#define _XOPEN_SOURCE_EXTENDED 1

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif
#ifdef WIN32
#   include <windows.h>
#endif

#include <ctype.h>
#include <cstring> // For strlen
#include <cwctype> // For iswalnum
#include <algorithm>

#include "ncurses.h"
#include "dic.h"
#include "game_factory.h"
#include "game.h"
#include "public_game.h"
#include "results.h"
#include "player.h"
#include "history.h"
#include "turn.h"
#include "game_exception.h"
#include "encoding.h"

using namespace std;


Box::Box(WINDOW *win, int y, int x, int h, int w,
         unsigned int iHeadingLines)
    : m_win(win),  m_x(x), m_y(y), m_w(w), m_h(h),
    m_topLine(y + 1 + iHeadingLines),
    m_nbLines(h - 2 - iHeadingLines), m_dataStart(0), m_dataSize(0)
{
}


void Box::draw(const string& iTitle) const
{
    if (m_w > 3 && m_h > 2)
    {
        // Add one space before and after the title for readability
        string title;
        if (!iTitle.empty())
            title = " " + iTitle + " ";
        unsigned int l = title.size();
        // Truncate the title if needed
        if ((int)l > m_w - 2)
            l = m_w - 2;

        mvwaddch(m_win, m_y, m_x,    ACS_ULCORNER);
        mvwhline(m_win, m_y, m_x + 1,  ACS_HLINE, (m_w - l - 2)/2);
        mvwprintw(m_win,m_y, m_x + 1 + (m_w - l - 2)/2, "%s", title.c_str());
        mvwhline(m_win, m_y, m_x + (m_w - l)/2 + l,
                 ACS_HLINE, m_w - 1 - ((m_w - l)/2 + l));
        mvwaddch(m_win, m_y, m_x + m_w - 1, ACS_URCORNER);

        mvwvline(m_win, m_y + 1, m_x, ACS_VLINE, m_h - 2);
        mvwvline(m_win, m_y + 1, m_x + m_w - 1, ACS_VLINE, m_h - 2);

        mvwaddch(m_win, m_y + m_h - 1, m_x, ACS_LLCORNER);
        mvwhline(m_win, m_y + m_h - 1, m_x + 1, ACS_HLINE, m_w - 2);
        mvwaddch(m_win, m_y + m_h - 1, m_x + m_w - 1, ACS_LRCORNER);
    }
}


void Box::printDataLine(int n, int x, const char *fmt, ...) const
{
    if (n < getFirstLine() || n >= getLastLine() || m_w <= x - m_x + 1)
        return;

    va_list vl_args;
    char *buf = NULL;
    va_start(vl_args, fmt);
    int res = vasprintf(&buf, fmt, vl_args);
    va_end(vl_args);

    if (buf == NULL || res == -1)
    {
        return;
    }

    mvwprintw(m_win, m_topLine + n - m_dataStart, x, "%s",
              truncString(buf, m_w - 1 - x + m_x).c_str());
    free(buf);
}


bool Box::scrollOneLineUp()
{
    if (m_dataSize <= m_nbLines || m_dataStart == 0)
        return false;
    m_dataStart--;
    return true;
}


bool Box::scrollOneLineDown()
{
    if (m_dataSize <= m_nbLines || m_dataStart >= m_dataSize - 1)
        return false;
    m_dataStart++;
    return true;
}


bool Box::scrollOnePageUp()
{
    if (m_dataSize <= m_nbLines)
        return false;
    m_dataStart -= m_nbLines;
    if (m_dataStart < 0)
        m_dataStart = 0;
    return true;
}


bool Box::scrollOnePageDown()
{
    if (m_dataSize <= m_nbLines)
        return false;
    m_dataStart += m_nbLines;
    if (m_dataStart > m_dataSize - 1)
        m_dataStart = m_dataSize - 1;
    return true;
}


bool Box::scrollBeginning()
{
    if (m_dataSize <= m_nbLines || m_dataStart == 0)
        return false;
    m_dataStart = 0;
    return true;
}


bool Box::scrollEnd()
{
    if (m_dataSize <= m_nbLines || m_dataStart == m_dataSize - 1)
        return false;
    m_dataStart = m_dataSize - 1;
    return true;
}


void Box::clearRect(WINDOW *win, int y, int x, int h, int w)
{
    for (int i = 0; i < h; i++)
    {
        mvwhline(win, y + i, x, ' ', w);
    }
}


CursesIntf::CursesIntf(WINDOW *win, PublicGame& iGame)
    : m_win(win), m_game(&iGame), m_state(DEFAULT), m_dying(false),
    m_box(win, 0, 0, 0, 0), m_showDots(false)
{
}


CursesIntf::~CursesIntf()
{
    //GameFactory::Instance()->releaseGame(*m_game);
    delete m_game;
    GameFactory::Destroy();
}


void CursesIntf::drawStatus(WINDOW *win, const string& iMessage, bool error)
{
    int cols;
    int lines;
    getmaxyx(win, lines, cols);
    int x = 0;
    int y = lines - 1;
    if (error)
        wattron(win, COLOR_PAIR(COLOR_YELLOW));
    mvwprintw(win, y, x, truncOrPad(iMessage, cols).c_str());
    if (error)
        wattron(win, COLOR_PAIR(COLOR_WHITE));
}


void CursesIntf::drawBoard(WINDOW *win, int y, int x) const
{
    // Box around the board
    Box box(win, y + 1, x + 3, 17, 47);
    box.draw();

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
            int wm = m_game->getBoard().GetWordMultiplier(row, col);
            int lm = m_game->getBoard().GetLetterMultiplier(row, col);
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
            const Tile &t = m_game->getBoard().getTile(row, col);
            if (!t.isEmpty())
            {
                const wstring &chr = t.getDisplayStr();
                int offset = 0;
                if (chr.size() > 1)
                    offset = -1;
                if (m_game->getBoard().isJoker(row, col))
                {
                    wattron(win, A_BOLD | COLOR_PAIR(COLOR_GREEN));
                    mvwprintw(win, y + row + 1, x + 3 * col + 2 + offset, lfw(chr).c_str());
                    wattroff(win, A_BOLD);
                }
                else
                {
                    mvwprintw(win, y + row + 1, x + 3 * col + 2 + offset, lfw(chr).c_str());
                }
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
    // Compute the longest player name
    size_t longest = 0;
    for (unsigned int i = 0; i < m_game->getNbPlayers(); i++)
    {
        longest = std::max(longest, m_game->getPlayer(i).getName().size());
    }

    Box box(win, y, x, m_game->getNbPlayers() + 2, 25);
    box.draw(_("Scores"));
    // Magic formula to truncate too long names
    unsigned int maxForScores =
        std::min(longest,
                 box.getWidth() - strlen(_("%s: %d")) - 1);
    unsigned int currId = m_game->getCurrentPlayer().getId();
    for (unsigned int i = 0; i < m_game->getNbPlayers(); i++)
    {
        if (m_game->getMode() != PublicGame::kTRAINING && i == currId)
            attron(A_BOLD);
        mvwprintw(win, y + i + 1, x + 2, _("%s: %d"),
                  truncOrPad(lfw(m_game->getPlayer(i).getName()),
                             maxForScores).c_str(),
                  m_game->getPlayer(i).getPoints());
        if (m_game->getMode() != PublicGame::kTRAINING && i == currId)
            attroff(A_BOLD);
    }

    // Distance between the 2 boxes
    unsigned int yOff = m_game->getNbPlayers() + 3;

    Box box2(win, y + yOff, x, m_game->getNbPlayers() + 2, 25);
    box2.draw(_("Racks"));
    // Magic formula to truncate too long names
    unsigned int maxForRacks =
        std::min(longest,
                 box.getWidth() - strlen(_("%s: %ls")) - 4);
    for (unsigned int i = 0; i < m_game->getNbPlayers(); i++)
    {
        if (m_game->getMode() != PublicGame::kTRAINING && i == currId)
            attron(A_BOLD);
        wstring rack = m_game->getPlayer(i).getCurrentRack().toString(PlayedRack::RACK_SIMPLE);
        mvwprintw(win, y + yOff + i + 1, x + 2, _("%s: %ls"),
                  truncOrPad(lfw(m_game->getPlayer(i).getName()),
                             maxForRacks).c_str(),
                  rack.c_str());
        if (m_game->getMode() != PublicGame::kTRAINING && i == currId)
            attroff(A_BOLD);
        // Force to refresh the whole rack
        whline(win, ' ', 7 - rack.size());
    }

    // Display a message when the search is complete
    if (m_game->getMode() == PublicGame::kTRAINING &&
        m_game->trainingGetResults().size())
    {
        mvwprintw(win, y + 2*yOff - 1, x + 2, _("Search complete"));
    }
    else
        mvwhline(win, y + 2*yOff - 1, x + 2, ' ', strlen(_("Search complete")));
}


void CursesIntf::drawResults(Box &ioBox) const
{
    if (m_game->getMode() != PublicGame::kTRAINING)
        return;

    ioBox.draw(_("Search results"));
    const Results& res = m_game->trainingGetResults();
    ioBox.setDataSize(res.size());

    unsigned int i;
    int x = ioBox.getLeft();
    for (i = (unsigned int)ioBox.getFirstLine();
         i < res.size() && i < (unsigned int)ioBox.getLastLine(); i++)
    {
        const Round &r = res.get(i);
        wstring coord = r.getCoord().toString();
        ioBox.printDataLine(i, x, "%3d %s %3s",
                            r.getPoints(),
                            padAndConvert(r.getWord(), ioBox.getWidth() - 9, false).c_str(),
                            lfw(coord).c_str());
    }
    // Complete the list with empty lines, to avoid trails
    for (; i < (unsigned int)ioBox.getLastLine(); i++)
    {
        ioBox.printDataLine(i, x + 1, string(ioBox.getWidth(), ' ').c_str());
    }
}


void CursesIntf::drawHistory(Box &ioBox) const
{
    // To allow pseudo-scrolling, without leaving trails
    ioBox.clearData();

    ioBox.draw(_("History of the game"));
    ioBox.setDataSize((int)m_game->getHistory().getSize());
    int x = ioBox.getLeft();
    int y = ioBox.getTop();

    // Heading
    string heading = truncString(_(" N |      RACK      |    SOLUTION     | REF | PTS | P | BONUS"),
                                 ioBox.getWidth() - 1);
    mvwprintw(m_win, y, x + 1, "%s", heading.c_str());
    mvwhline(m_win, y + 1, x + 1, ACS_HLINE, heading.size());

    int i;
    for (i = ioBox.getFirstLine();
         i < (int)m_game->getHistory().getSize() && i < ioBox.getLastLine(); i++)
    {
        const Turn& t = m_game->getHistory().getTurn(i);
        const Move& m = t.getMove();
        if (m.getType() == Move::VALID_ROUND)
        {
            // The move corresponds to a played round: display it
            const Round &r = m.getRound();
            wstring coord = r.getCoord().toString();
            ioBox.printDataLine(i, x,
                " %2d   %s   %s   %s   %3d   %1d   %c",
                i + 1, padAndConvert(t.getPlayedRack().toString(), 14, false).c_str(),
                padAndConvert(r.getWord(), 15, false).c_str(),
                padAndConvert(coord, 3).c_str(), r.getPoints(),
                t.getPlayer(), r.getBonus() ? '*' : ' ');
        }
        else if (m.getType() == Move::INVALID_WORD)
        {
            // The move corresponds to an invalid word: display it
            wstring invWord = L"<" + m.getBadWord() + L">";
            ioBox.printDataLine(i, x,
                " %2d   %s   %s   %s   %3d   %1d",
                i + 1, padAndConvert(t.getPlayedRack().toString(), 14, false).c_str(),
                padAndConvert(invWord, 15, false).c_str(),
                padAndConvert(m.getBadCoord(), 3).c_str(), m.getScore(),
                t.getPlayer());
        }
        else
        {
            // The move corresponds to a passed turn or changed letters
            wstring action;
            if (m.getType() == Move::PASS)
                action = wfl(_("(PASS)"));
            else if (m.getType() == Move::CHANGE_LETTERS)
                action = L"(-" + m.getChangedLetters() + L")";

            ioBox.printDataLine(i, x,
                " %2d   %s   %s   %s   %3d   %1d",
                i + 1, padAndConvert(t.getPlayedRack().toString(), 14, false).c_str(),
                padAndConvert(action, 15, false).c_str(),
                " - ", m.getScore(), t.getPlayer());
        }
    }
    int nbLines = min(i + 2 - ioBox.getFirstLine(),
                      ioBox.getLastLine() - ioBox.getFirstLine() + 2);
    mvwvline(m_win, y, x + 4,  ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 21, ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 39, ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 45, ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 51, ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 55, ACS_VLINE, nbLines);
}


void CursesIntf::drawHelp(Box &ioBox) const
{
    // To allow pseudo-scrolling, without leaving trails
    ioBox.clearData();
    ioBox.draw(_("Help"));

    int x = ioBox.getLeft() + 1;
    int n = 0;
    ioBox.printDataLine(n++, x, _("[Global]"));
    ioBox.printDataLine(n++, x, _("   h, H, ?          Show/hide help box"));
    ioBox.printDataLine(n++, x, _("   y, Y             Show/hide history of the game"));
    ioBox.printDataLine(n++, x, _("   b, B             Show/hide contents of the bag (including letters of the racks)"));
    ioBox.printDataLine(n++, x, _("   e, E             Show/hide dots on empty squares of the board"));
    ioBox.printDataLine(n++, x, _("   d, D             Check the existence of a word in the dictionary"));
    ioBox.printDataLine(n++, x, _("   j, J             Play a word"));
    ioBox.printDataLine(n++, x, _("   s, S             Save the game"));
    ioBox.printDataLine(n++, x, _("   l, L             Load a game"));
    ioBox.printDataLine(n++, x, _("   q, Q             Quit"));
    ioBox.printDataLine(n++, x, "");

    ioBox.printDataLine(n++, x, _("[Training mode]"));
    ioBox.printDataLine(n++, x, _("   *                Take a random rack"));
    ioBox.printDataLine(n++, x, _("   +                Complete the current rack randomly"));
    ioBox.printDataLine(n++, x, _("   t, T             Set the rack manually"));
    ioBox.printDataLine(n++, x, _("   c, C             Compute all the possible words"));
    ioBox.printDataLine(n++, x, _("   r, R             Show/hide search results"));
    ioBox.printDataLine(n++, x, "");

    ioBox.printDataLine(n++, x, _("[Duplicate mode]"));
    ioBox.printDataLine(n++, x, _("   n, N             Switch to the next human player"));
    ioBox.printDataLine(n++, x, "");

    ioBox.printDataLine(n++, x, _("[Free game mode]"));
    ioBox.printDataLine(n++, x, _("   p, P             Pass your turn (with or without changing letters)"));
    ioBox.printDataLine(n++, x, "");

    ioBox.printDataLine(n++, x, _("[Miscellaneous]"));
    ioBox.printDataLine(n++, x, _("   <up>, <down>     Navigate in a box line by line"));
    ioBox.printDataLine(n++, x, _("   <pgup>, <pgdown> Navigate in a box page by page"));
    ioBox.printDataLine(n++, x, _("   Ctrl-l           Refresh the screen"));

    ioBox.setDataSize(n);
}


void CursesIntf::drawBag(Box &ioBox) const
{
    // To allow pseudo-scrolling, without leaving trails
    ioBox.clearData();

    ioBox.draw(_("Bag"));
    vector<Tile> allTiles = m_game->getDic().getAllTiles();
    ioBox.setDataSize(allTiles.size());
    int x = ioBox.getLeft();
    int y = ioBox.getTop();

    // Heading
    string heading = truncString(_(" LETTER | POINTS | FREQUENCY | REMAINING"),
                                 ioBox.getWidth() - 1);
    mvwprintw(m_win, y, x + 1, "%s", heading.c_str());
    mvwhline(m_win, y + 1, x + 1, ACS_HLINE, heading.size());

    int i;
    for (i = ioBox.getFirstLine(); i < (int)allTiles.size() && i < ioBox.getLastLine(); i++)
    {
        const wstring &chr = allTiles[i].getDisplayStr();
        wstring str;
        for (unsigned int j = 0; j < m_game->getBag().in(allTiles[i]); ++j)
             str += chr;
        ioBox.printDataLine(i, ioBox.getLeft() + 1,
                            "  %s        %2d        %2d       %s",
                            padAndConvert(allTiles[i].getDisplayStr(), 2).c_str(),
                            allTiles[i].getPoints(),
                            allTiles[i].maxNumber(),
                            lfw(str).c_str());
    }

    int nbLines = min(i + 2 - ioBox.getFirstLine(),
                      ioBox.getLastLine() - ioBox.getFirstLine() + 2);
    mvwvline(m_win, y, x + 9,  ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 18, ACS_VLINE, nbLines);
    mvwvline(m_win, y, x + 30, ACS_VLINE, nbLines);
}


void CursesIntf::setState(State iState)
{
    // Clear the previous box
    m_box.clear();

    // Get the size of the screen (better than using COLS and LINES directly,
    // according to the manual)
    int lines;
    int cols;
    getmaxyx(m_win, lines, cols);

    m_state = iState;
    if (m_state == DEFAULT)
        m_box = Box(m_win, 0, 0, 0, 0);
    else if (m_state == RESULTS)
        m_box = Box(m_win, 3, 54, 17, 25);
    else if (m_state == HISTORY)
        m_box = Box(m_win, 1, 0, lines - 1, cols, 2);
    else if (m_state == HELP)
        m_box = Box(m_win, 1, 0, lines - 1, cols);
    else if (m_state == BAG)
        m_box = Box(m_win, 1, 0, lines - 1, cols, 2);
}


void CursesIntf::playWord(WINDOW *win, int y, int x)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Play a word"));
    mvwprintw(win, y + 1, x + 2, _("Played word:"));
    mvwprintw(win, y + 2, x + 2, _("Coordinates:"));
    wrefresh(win);

    // TRANSLATORS: Align the : when translating "Played word:" and
    // "Coordinates:". For example:
    // Pl. word   :
    // Coordinates:
    int l1 = strlen(_("Played word:"));
    int l2 = strlen(_("Coordinates:"));
    int xOff;
    if (l1 > l2)
        xOff = l1 + 3;
    else
        xOff = l2 + 3;

    wstring word, coord;
    if (readString(win, y + 1, x + xOff, 15, word) &&
        readString(win, y + 2, x + xOff, 3, coord))
    {
        int res = m_game->play(word, coord);
        if (res)
        {
            drawStatus(win, _("Incorrect or misplaced word"));
        }
    }
    box.clear();
}


void CursesIntf::checkWord(WINDOW *win, int y, int x)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Dictionary"));
    mvwprintw(win, y + 1, x + 2, _("Enter the word to check:"));
    wrefresh(win);

    wstring word;
    if (readString(win, y + 2, x + 2, 15, word))
    {
        bool res = m_game->getDic().searchWord(word);
        char s[100];
        if (res)
            snprintf(s, 100, _("The word '%ls' exists"), word.c_str());
        else
            snprintf(s, 100, _("The word '%ls' does not exist"), word.c_str());
        drawStatus(win, s, false);
    }
    box.clear();
}


void CursesIntf::saveGame(WINDOW *win, int y, int x)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Save the game"));
    mvwprintw(win, y + 1, x + 2, _("Enter the file name:"));
    wrefresh(win);

    wstring filename;
    if (readString(win, y + 2, x + 2, 28, filename, kFILENAME))
    {
        char s[100];
        try
        {
            m_game->save(lfw(filename));
            snprintf(s, 100, _("Game saved in '%ls'"), filename.c_str());
            drawStatus(win, s, false);
        }
        catch (std::exception &e)
        {
            snprintf(s, 100, _("Error saving game %s:"), e.what());
            drawStatus(win, s);
        }
    }
    box.clear();
}


void CursesIntf::loadGame(WINDOW *win, int y, int x)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Load a game"));
    mvwprintw(win, y + 1, x + 2, _("Enter the file name:"));
    wrefresh(win);

    wstring filename;
    if (readString(win, y + 2, x + 2, 28, filename, kFILENAME))
    {
        try
        {
            PublicGame *loaded = PublicGame::load(lfw(filename), m_game->getDic());
            //GameFactory::Instance()->releaseGame(*m_game);
            delete m_game;
            m_game = loaded;
            char s[100];
            snprintf(s, 100, _("Game loaded"));
            drawStatus(win, s, false);
        }
        catch (const GameException &e)
        {
            drawStatus(win, _("Unable to load game: ") + string(e.what()));
        }
    }
    box.clear();
}


void CursesIntf::passTurn(WINDOW *win, int y, int x, PublicGame &iGame)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Pass your turn"));
    mvwprintw(win, y + 1, x + 2, _("Enter the letters to change:"));
    wrefresh(win);

    wstring letters;
    if (readString(win, y + 2, x + 2, 7, letters))
    {
        int res = iGame.freeGamePass(letters);
        if (res)
        {
            drawStatus(win, _("Cannot pass the turn"));
        }
    }
    box.clear();
}


void CursesIntf::setRack(WINDOW *win, int y, int x, PublicGame &iGame)
{
    Box box(win, y, x, 4, 32);
    box.draw(_("Set rack"));
    mvwprintw(win, y + 1, x + 2, _("Enter the new letters:"));
    wrefresh(win);

    wstring letters;
    if (readString(win, y + 2, x + 2, 7, letters, kJOKER))
    {
        try
        {
            iGame.trainingSetRackManual(false, letters);
        }
        catch (GameException &e)
        {
            drawStatus(win, _("Cannot take these letters from the bag:"));
        }
    }
    m_state = DEFAULT;
    box.clear();
}


bool CursesIntf::readString(WINDOW *win, int y, int x, int n, wstring &oString,
                            unsigned int flag)
{
    // Save the initial position
    int x0 = x;
    wint_t c;
    wmove(win, y, x);
    curs_set(1);
    int res;
    // Position in the string before which to insert the next character
    // (the character will be added at the end if pos == oString.size())
    unsigned int pos = 0;
    while ((res = get_wch(&c)) != ERR)
    {
        if (c == 0x1b )  // Esc
        {
            curs_set(0);
            return false;
        }
        else if ((c == KEY_ENTER && res == KEY_CODE_YES) || c == 0xD)
        {
            curs_set(0);
            return true;
        }
        else if (c == 0x0c)  // Ctrl-L
        {
            redraw(win);
            wmove(win, y, x);
        }
        else if (c == 0x0b)  // Ctrl-K
        {
            // Remove everything after the cursor position
            int len = oString.size() - pos;
            oString = oString.erase(pos);
            mvwprintw(win, y, x, string(len, ' ').c_str());
            wmove(win, y, x);
        }
        else if (c == 0x15)  // Ctrl-U
        {
            // Remove everything before the cursor position
            oString.erase(0, pos);
            int len = pos;
            x = x0;
            pos = 0;
            mvwprintw(win, y, x0, "%s", lfw(oString + wstring(len, L' ')).c_str());
            wmove(win, y, x);
        }
        else if (res == KEY_CODE_YES)
        {
            if (c == KEY_BACKSPACE && pos != 0)
            {
                x--;
                pos--;
                oString.erase(pos, 1);
                mvwprintw(win, y, x0, "%s", lfw(oString + L" ").c_str());
                wmove(win, y, x);
            }
            else if (c == KEY_DC)
            {
                oString.erase(pos, 1);
                mvwprintw(win, y, x0, "%s", lfw(oString + L" ").c_str());
                wmove(win, y, x);
            }
            else if (c == KEY_LEFT && pos != 0)
            {
                x--;
                pos--;
                wmove(win, y, x);
            }
            else if (c == KEY_RIGHT && pos != oString.size())
            {
                x++;
                pos++;
                wmove(win, y, x);
            }
            else if (c == KEY_HOME)
            {
                x = x0;
                pos = 0;
                wmove(win, y, x);
            }
            else if (c == KEY_END)
            {
                x = x0 + oString.size();
                pos = oString.size();
                wmove(win, y, x);
            }
            else
                beep();
        }
        else if (res == OK && iswalnum(c) && oString.size() < (unsigned int)n)
        {
            x++;
            oString.insert(pos++, 1, c);
            mvwprintw(win, y, x0, "%s", lfw(oString).c_str());
            wmove(win, y, x);
        }
        else if (flag & kJOKER && c == L'?')
        {
            x++;
            oString.insert(pos++, 1, c);
            mvwprintw(win, y, x0, "%s", lfw(oString).c_str());
            wmove(win, y, x);
        }
        else if (flag & kFILENAME)
        {
            if (c == L'/' || c == L'.' || c == L'-' || c == L'_' || c == L' ')
            {
                x++;
                oString += c;
                mvwprintw(win, y, x0, "%s", lfw(oString).c_str());
                wmove(win, y, x);
            }
            else
                beep();
        }
        else
            beep();
    }
    curs_set(0);
    return false;
}


int CursesIntf::handleKeyForTraining(int iKey, PublicGame &iGame)
{
    switch (iKey)
    {
        case '*':
            if (m_state != DEFAULT)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            iGame.trainingSetRackRandom(false, PublicGame::kRACK_ALL);
            return 1;

        case '+':
            if (m_state != DEFAULT)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            iGame.trainingSetRackRandom(false, PublicGame::kRACK_NEW);
            return 1;

        case 't':
        case 'T':
            if (m_state != DEFAULT)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            setRack(m_win, 22, 10, iGame);
            return 1;

        case 'c':
        case 'C':
            iGame.trainingSearch();
            return 1;

        default:
            return 2;
    }
}


int CursesIntf::handleKeyForDuplicate(int iKey, PublicGame &iGame)
{
    switch (iKey)
    {
        case 'n':
        case 'N':
        {
            // Get the human players who have not played yet
            set<unsigned int> humans;
            for (unsigned int id = 0; id < iGame.getNbPlayers(); ++id)
            {
                if (iGame.getPlayer(id).isHuman() && !iGame.hasPlayed(id))
                    humans.insert(id);
            }
            unsigned int currId = iGame.getCurrentPlayer().getId();
            // Try to find a player with a bigger ID
            set<unsigned int>::const_iterator it = humans.upper_bound(currId);
            if (it != humans.end())
                iGame.duplicateSetPlayer(*it);
            else
                iGame.duplicateSetPlayer(*humans.begin());
            return 1;
        }

        default:
            return 2;
    }
}


int CursesIntf::handleKeyForFreeGame(int iKey, PublicGame &iGame)
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
    // Remove any error message in the status line
    if (m_state == DEFAULT || m_state == RESULTS)
        drawStatus(m_win, "", false);

    // Handle game-specific keys
    int res;
    if (m_game->getMode() == PublicGame::kTRAINING)
    {
        res = handleKeyForTraining(iKey, *m_game);
    }
    else if (m_game->getMode() == PublicGame::kDUPLICATE)
    {
        res = handleKeyForDuplicate(iKey, *m_game);
    }
    else
    {
        res = handleKeyForFreeGame(iKey, *m_game);
    }
    if (res != 2)
        return res;

    // Handle scrolling keys
    if (m_state != DEFAULT)
    {
        switch (iKey)
        {
            case KEY_HOME:
                return m_box.scrollBeginning() ? 1 : 0;
            case KEY_END:
                return m_box.scrollEnd() ? 1 : 0;
            case KEY_UP:
                return m_box.scrollOneLineUp() ? 1 : 0;
            case KEY_DOWN:
                return m_box.scrollOneLineDown() ? 1 : 0;
            case KEY_PPAGE:
                return m_box.scrollOnePageUp() ? 1 : 0;
            case KEY_NPAGE:
                return m_box.scrollOnePageDown() ? 1 : 0;
        }
    }

    // Handle other global keys
    switch (iKey)
    {
        // Toggle help
        case 'h':
        case 'H':
        case '?':
            if (m_state == HELP)
                setState(DEFAULT);
            else
                setState(HELP);
            clear();
            return 1;

        // Toggle history
        case 'y':
        case 'Y':
            if (m_state == HISTORY)
                setState(DEFAULT);
            else
                setState(HISTORY);
            clear();
            return 1;

        // Toggle results (training mode only)
        case 'r':
        case 'R':
            if (m_game->getMode() != PublicGame::kTRAINING)
            {
                beep();
                return 0;
            }
            if (m_state == RESULTS)
                setState(DEFAULT);
            else
                setState(RESULTS);
            Box::clearRect(m_win, 3, 54, 30, 25);
            return 1;

        // Toggle bag
        case 'b':
        case 'B':
            if (m_state == BAG)
                setState(DEFAULT);
            else
                setState(BAG);
            clear();
            return 1;

        // Toggle dots display
        case 'e':
        case 'E':
            m_showDots = !m_showDots;
            return 1;

        // Ctrl-L should clear and redraw the screen
        case 0x0c:
            clear();
            // Force the re-definition of the current box
            setState(m_state);
            return 1;

        // Check a word in the dictionary
        case 'd':
        case 'D':
            if (m_state != DEFAULT && m_state != RESULTS)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            checkWord(m_win, 22, 10);
            return 0;

        // Play a word
        case 'j':
        case 'J':
            if (m_state != DEFAULT && m_state != RESULTS)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            playWord(m_win, 22, 10);
            return 1;

        case 'l':
        case 'L':
            if (m_state != DEFAULT)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            loadGame(m_win, 22, 10);
            return 1;

        case 's':
        case 'S':
            if (m_state != DEFAULT)
            {
                setState(DEFAULT);
                redraw(m_win);
            }
            saveGame(m_win, 22, 10);
            return 0;

        // Quit
        case 'q':
        case 'Q':
            m_dying = true;
            return 0;

        default:
            beep();
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
        drawResults(m_box);
        drawBoard(win, 2, 0);
    }
    else if (m_state == HELP)
    {
        drawHelp(m_box);
    }
    else if (m_state == HISTORY)
    {
        drawHistory(m_box);
    }
    else if (m_state == BAG)
    {
        drawBag(m_box);
    }

    // Title
    attron(A_REVERSE);
    string mode;
    if (m_game->getMode() == PublicGame::kTRAINING)
        mode = _("Training mode");
    else if (m_game->getMode() == PublicGame::kFREEGAME)
        mode = _("Free game mode");
    else if (m_game->getMode() == PublicGame::kDUPLICATE)
        mode = _("Duplicate mode");
    string variant = "";
    if (m_game->getVariant() == PublicGame::kJOKER)
        variant = string(" - ") + _("Joker game");
    string title = "Eliot (" + mode + variant + ") " + _("[h for help]");

    int lines;
    int cols;
    getmaxyx(m_win, lines, cols);
    mvwprintw(win, 0, 0, truncOrPad(title, cols).c_str());
    attroff(A_REVERSE);

    wrefresh(win);
}


int main(int argc, char ** argv)
{
#if HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
    // Set the message domain
#ifdef WIN32
    // Get the absolute path, as returned by GetFullPathName()
    char baseDir[MAX_PATH];
    GetFullPathName(argv[0], MAX_PATH, baseDir, NULL);
    char *pos = strrchr(baseDir, L'\\');
    if (pos)
        *pos = '\0';
    const string localeDir = baseDir + string("\\locale");
#else
    static const string localeDir = LOCALEDIR;
#endif
    bindtextdomain(PACKAGE, localeDir.c_str());
    textdomain(PACKAGE);
#endif

    srand(time(NULL));

    Game *realGame = GameFactory::Instance()->createFromCmdLine(argc, argv);
    if (realGame == NULL)
    {
        GameFactory::Destroy();
        return 1;
    }
    PublicGame *game = new PublicGame(*realGame);

    game->start();

    // Initialize the ncurses library
    WINDOW *wBoard = initscr();
    keypad(wBoard, true);
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
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_RED);

        init_pair(COLOR_BLUE, COLOR_BLACK, COLOR_BLUE);
        init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
        init_pair(COLOR_MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(COLOR_RED, COLOR_BLACK, COLOR_RED);
    }

    // Do not echo
    noecho();

    // mainIntf will take care of destroying game for us
    CursesIntf mainIntf(wBoard, *game);
    mainIntf.redraw(wBoard);

    while (!mainIntf.isDying())
    {
        int c = getch();
        if (mainIntf.handleKey(c) == 1)
        {
            mainIntf.redraw(wBoard);
        }
    }

    delwin(wBoard);

    // Exit the ncurses library
    endwin();

    GameFactory::Destroy();

    return 0;
}
