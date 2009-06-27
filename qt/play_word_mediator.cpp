/*****************************************************************************
 * Eliot
 * Copyright (C) 2009 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

#include "play_word_mediator.h"
#include "coord_model.h"
#include "qtcommon.h"

#include "public_game.h"
#include "coord.h"
#include "dic.h"
#include "header.h"
#include "debug.h"


/// Validator used for the "play word" line edit
class PlayWordValidator: public QValidator
{
public:
    explicit PlayWordValidator(QObject *parent,
                               const Dictionary &iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary &m_dic;
};


/// Validator used for the "coords" line edit
class CoordsValidator: public QValidator
{
public:
    explicit CoordsValidator(QObject *parent);
    virtual State validate(QString &input, int &pos) const;
};



PlayWordMediator::PlayWordMediator(QObject *parent, QLineEdit &iEditPlay,
                                   QLineEdit &iEditCoord, QPushButton &iButtonPlay,
                                   CoordModel &iCoordModel, PublicGame *iGame)
    : QObject(parent), m_game(iGame), m_lineEditPlay(iEditPlay),
    m_lineEditCoord(iEditCoord), m_pushButtonPlay(iButtonPlay),
    m_coordModel(iCoordModel)
{
    m_lineEditPlay.setFocus();
    // These strings cannot be in the .ui file, because of the newlines
    m_lineEditPlay.setToolTip(_q("Enter the word to play (case-insensitive).\n"
            "A joker from the rack must be written in parentheses.\n"
            "E.g.: w(o)rd or W(O)RD"));
    m_lineEditCoord.setToolTip(_q("Enter the coordinates of the word.\n"
            "Specify the row before the column for horizontal words,\n"
            "and the column before the row for vertical words.\n"
            "E.g.: H4 or 4H"));

    /// Set validators;
    if (m_game)
    {
        m_lineEditPlay.setValidator(new PlayWordValidator(this, m_game->getDic()));
        m_lineEditCoord.setValidator(new CoordsValidator(this));
    }

    // Set all the connections
    QObject::connect(&m_lineEditPlay, SIGNAL(textChanged(const QString&)),
                     this, SLOT(lineEditPlay_textChanged()));
    QObject::connect(&m_lineEditPlay, SIGNAL(returnPressed()),
                     this, SLOT(lineEditPlay_returnPressed()));
    QObject::connect(&m_lineEditCoord, SIGNAL(textChanged(const QString&)),
                     this, SLOT(lineEditCoord_textChanged(const QString&)));
    QObject::connect(&m_lineEditCoord, SIGNAL(returnPressed()),
                     this, SLOT(lineEditCoord_returnPressed()));
    QObject::connect(&m_pushButtonPlay, SIGNAL(clicked()),
                     this, SLOT(pushButtonPlay_clicked()));
    QObject::connect(&m_coordModel, SIGNAL(coordChanged(const Coord&)),
                     this, SLOT(updateCoord(const Coord&)));
}


void PlayWordMediator::lineEditPlay_textChanged()
{
    m_pushButtonPlay.setEnabled(m_lineEditPlay.hasAcceptableInput() &&
                                m_lineEditCoord.hasAcceptableInput());
}


void PlayWordMediator::lineEditPlay_returnPressed()
{
    if (!m_lineEditPlay.hasAcceptableInput() ||
        !m_lineEditCoord.hasAcceptableInput())
        return;

    // Convert the jokers to lowercase
    const wistring &inputWord = qtw(m_lineEditPlay.text().toUpper());
    // Convert to internal representation, then back to QString
    QString word = qfw(m_game->getDic().getHeader().convertFromInput(inputWord));

    int pos;
    while ((pos = word.indexOf('(')) != -1)
    {
        if (word.size() < pos + 3 || word[pos + 2] != ')' ||
            !m_game->getDic().validateLetters(qtw(QString(word[pos + 1]))))
        {
            // Bug in validate()!
            // This should never happen
            QString msg = _q("Cannot play word: misplaced parentheses");
            emit notifyProblem(msg);
            break;
        }
        else
        {
            QChar chr = word[pos + 1].toLower();
            word.remove(pos, 3);
            word.insert(pos, chr);
        }
    }

    // Convert the input string into an internal one
    const wstring intWord =
        m_game->getDic().getHeader().convertFromInput(qtw(word));

    QString coords = m_lineEditCoord.text();
    int res = m_game->play(intWord, qtw(coords));
    if (res == 0)
    {
        emit gameUpdated();
        m_lineEditPlay.setFocus();
    }
    else
    {
        // Try to be as explicit as possible concerning the error
        QString msg = _q("Cannot play '%1' at position '%2':\n")
            .arg(m_lineEditPlay.text()).arg(coords);
        switch (res)
        {
            case 1:
                msg += _q("Some letters are not valid for the current dictionary");
                break;
            case 2:
                msg += _q("Invalid coordinates");
                break;
            case 3:
                msg += _q("The word does not exist");
                break;
            case 4:
                msg += _q("The rack doesn't contain the letters needed to play this word");
                break;
            case 5:
                msg += _q("The word is part of a longer one");
                break;
            case 6:
                msg += _q("The word tries to replace an existing letter");
                break;
            case 7:
                msg += _q("An orthogonal word is not valid");
                break;
            case 8:
                msg += _q("The word is already present on the board at these coordinates");
                break;
            case 9:
                msg += _q("A word cannot be isolated (not connected to the placed words)");
                break;
            case 10:
                msg += _q("The first word of the game must be horizontal");
                break;
            case 11:
                msg += _q("The first word of the game must cover the H8 square");
                break;
            case 12:
                msg += _q("The word is going out of the board");
                break;
            default:
                msg += _q("Incorrect or misplaced word (%1)").arg(1);
        }
        // FIXME: the error is too generic
        emit notifyProblem(msg);
    }
}


void PlayWordMediator::lineEditCoord_textChanged(const QString &iText)
{
    Coord c(qtw(iText));
    if (!(m_coordModel.getCoord() == c))
        m_coordModel.setCoord(Coord(qtw(iText)));
    lineEditPlay_textChanged();
}


void PlayWordMediator::updateCoord(const Coord &iCoord)
{
    if (iCoord.isValid() && m_lineEditCoord.text() != qfw(iCoord.toString()))
        m_lineEditCoord.setText(qfw(iCoord.toString()));
    else if (!iCoord.isValid() && m_lineEditCoord.hasAcceptableInput())
        m_lineEditCoord.setText("");
    lineEditPlay_textChanged();
}


// ------ Validators ------

PlayWordValidator::PlayWordValidator(QObject *parent,
                                     const Dictionary &iDic)
    : QValidator(parent), m_dic(iDic)
{
}


QValidator::State PlayWordValidator::validate(QString &input, int &) const
{
    if (input == "")
        return Intermediate;

    const wistring &winput = qtw(input);
    // The string is invalid if it contains invalid input characters
    if (!m_dic.validateInputChars(winput, L"()") || input.contains('?'))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = m_dic.getHeader().convertFromInput(winput);
    // The string is invalid if it contains characters not present
    // in the dictionary (ignoring parentheses)
    if (!m_dic.validateLetters(intInput, L"()"))
        return Intermediate;

    // Check the parentheses pairs
    QString qintInput = qfw(intInput);
    int pos;
    while ((pos = qintInput.indexOf('(')) != -1)
    {
        if (qintInput.size() < pos + 3 || qintInput[pos + 2] != ')' ||
            !m_dic.validateLetters(qtw(QString(qintInput[pos + 1]))))
        {
            return Intermediate;
        }
        else
        {
            qintInput.remove(pos, 3);
        }
    }
    if (qintInput.indexOf(')') != -1)
        return Intermediate;

    return Acceptable;
}



CoordsValidator::CoordsValidator(QObject *parent)
    : QValidator(parent)
{
}


QValidator::State CoordsValidator::validate(QString &input, int &) const
{
    // Only authorize characters part of a valid coordinate
    wstring copy = qtw(input.toUpper());
    wstring authorized = L"ABCDEFGHIJKLMNO1234567890";
    if (copy.find_first_not_of(authorized) != wstring::npos)
        return Invalid;

    // Check coordinates
    Coord c(qtw(input));
    if (!c.isValid())
        return Intermediate;

    return Acceptable;
}


