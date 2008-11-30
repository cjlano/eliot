/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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
#include <QtGui/QHBoxLayout>
#include <QtGui/QValidator>
#include <QtCore/QStringList>

#include "player_widget.h"
#include "training_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "pldrack.h"
#include "coord.h"
#include "dic.h"
#include "debug.h"

#include "encoding.h"


/// Validator used for the "change letters" line edit
class ChangeValidator: public QValidator
{
public:
    explicit ChangeValidator(QObject *parent,
                             const QLineEdit &iLineEdit);
    virtual State validate(QString &input, int &pos) const;

private:
    const QLineEdit &m_lineEdit;
};


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


PlayerWidget::PlayerWidget(QWidget *parent, unsigned int iPlayerNb, PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_player(iPlayerNb)
{
    setupUi(this);
    lineEditPlay->setFocus();
    // These strings cannot be in the .ui file, because of the newlines
    lineEditPlay->setToolTip(_q("Enter the word to play (case-insensitive).\n"
            "A joker from the rack must be written in parentheses.\n"
            "E.g.: w(o)rd or W(O)RD"));
    lineEditCoords->setToolTip(_q("Enter the coordinates of the word.\n"
            "Specify the row before the column for horizontal words,\n"
            "and the column before the row for vertical words.\n"
            "E.g.: H4 or 4H"));

    if (m_game)
    {
        lineEditPlay->setValidator(new PlayWordValidator(this, m_game->getDic()));
        lineEditCoords->setValidator(new CoordsValidator(this));

        // Do not allow messing with AI players
        if (!m_game->getPlayer(m_player).isHuman())
            setEnabled(false);

        // Changing the rack is authorized only in training mode
        lineEditRack->setReadOnly(m_game->getMode() != PublicGame::kTRAINING);
    }
    if (m_game == NULL || m_game->getMode() != PublicGame::kFREEGAME)
    {
        // Hide the freegame-specific controls
        labelChange->hide();
        lineEditChange->hide();
        pushButtonChange->hide();
        pushButtonPass->hide();
    }
    else
    {
        lineEditChange->setValidator(new ChangeValidator(this, *lineEditRack));
    }

    refresh();
}


QSize PlayerWidget::sizeHint() const
{
    return QSize(100, 100);
}


void PlayerWidget::refresh()
{
    if (m_game == NULL)
    {
        lineEditRack->clear();
        return;
    }

    const PlayedRack &pld = m_game->getPlayer(m_player).getCurrentRack();
    lineEditRack->setText(qfw(pld.toString(PlayedRack::RACK_EXTRA)));
    lineEditPlay->clear();
    lineEditCoords->clear();
    lineEditChange->clear();

    // Do not allow messing with AI players or with players who already played
    setEnabled(!m_game->hasPlayed(m_player) &&
               m_game->getPlayer(m_player).isHuman());
}


void PlayerWidget::on_pushButtonShuffle_clicked()
{
    m_game->shuffleRack();
    emit gameUpdated();
}


void PlayerWidget::on_lineEditPlay_textChanged()
{
    pushButtonPlay->setEnabled(lineEditPlay->hasAcceptableInput() &&
                               lineEditCoords->hasAcceptableInput());
}


void PlayerWidget::on_lineEditChange_textChanged()
{
    pushButtonChange->setEnabled(lineEditChange->hasAcceptableInput() &&
                                 lineEditChange->text() != "");
    pushButtonPass->setEnabled(lineEditChange->text() == "");
}


void PlayerWidget::on_lineEditPlay_returnPressed()
{
    if (!lineEditPlay->hasAcceptableInput() ||
        !lineEditCoords->hasAcceptableInput())
        return;

    // Convert the jokers to lowercase
    QString word = lineEditPlay->text().toUpper();
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

    QString coords = lineEditCoords->text();
    int res = m_game->play(qtw(word), qtw(coords));
    if (res == 0)
    {
        emit gameUpdated();
        lineEditPlay->setFocus();
    }
    else
    {
        // Try to be as explicit as possible concerning the error
        QString msg = _q("Cannot play '%1' at position '%2':\n")
            .arg(lineEditPlay->text()).arg(coords);
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


void PlayerWidget::on_lineEditChange_returnPressed()
{
    ASSERT(m_game->getMode() == PublicGame::kFREEGAME,
           "Trying to pass or change letters while not in free game mode");

    // Pass the turn (and possibly change letters)
    QString letters = lineEditChange->text();
    int res = m_game->freeGamePass(qtw(letters));
    if (res == 0)
        emit gameUpdated();
    else
    {
        // FIXME: the error is too generic
        QString msg;
        if (letters == "")
            msg = _q("Cannot pass turn (%1)").arg(res);
        else
            msg = _q("Cannot change letters '%1' (%2)").arg(letters).arg(res);
        emit notifyProblem(msg);
    }
}



ChangeValidator::ChangeValidator(QObject *parent,
                                 const QLineEdit &iLineEdit)
    : QValidator(parent), m_lineEdit(iLineEdit)
{
}


QValidator::State ChangeValidator::validate(QString &input, int &) const
{
    QString rack = m_lineEdit.text();
    if (input.size() > rack.size())
        return Intermediate;
    // The letters to change must be in the rack
    for (int i = 0; i < input.size(); ++i)
    {
        if (input.count(input[i], Qt::CaseInsensitive) >
            rack.count(input[i], Qt::CaseInsensitive))
        {
            return Intermediate;
        }
    }
    return Acceptable;
}



PlayWordValidator::PlayWordValidator(QObject *parent,
                                     const Dictionary &iDic)
    : QValidator(parent), m_dic(iDic)
{
}


QValidator::State PlayWordValidator::validate(QString &input, int &) const
{
    if (input == "")
        return Intermediate;

    QString copy(input);
    // Strip parentheses
    copy.remove('(');
    copy.remove(')');
    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic.validateLetters(qtw(copy)) || copy.contains('?'))
        return Invalid;

    // Check the parentheses pairs
    copy = input;
    int pos;
    while ((pos = copy.indexOf('(')) != -1)
    {
        if (copy.size() < pos + 3 || copy[pos + 2] != ')' ||
            !m_dic.validateLetters(qtw(QString(copy[pos + 1]))))
        {
            return Intermediate;
        }
        else
        {
            copy.remove(pos, 3);
        }
    }
    if (copy.indexOf(')') != -1)
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



PlayerTabWidget::PlayerTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    QObject::connect(this, SIGNAL(currentChanged(int)),
                     this, SLOT(changeCurrentPlayer(int)));
}


void PlayerTabWidget::setGame(PublicGame *iGame)
{
    m_game = iGame;

    // Remove all the tabs
    int nbTabs = count();
    for (int i = 0; i < nbTabs; ++i)
    {
        setCurrentIndex(0);
        // Cut all the connections with the page (needed because removeTab()
        // doesn't really destroy the widget)
        disconnect(currentWidget());
        removeTab(0);
    }

    if (iGame != NULL)
    {
        // Training mode: use a dedicated widget
        if (iGame->getMode() == PublicGame::kTRAINING)
        {
            const Player &player = iGame->getPlayer(0);
            TrainingWidget *trWidget = new TrainingWidget;
            trWidget->setGame(iGame);
            QObject::connect(this, SIGNAL(refreshSignal()),
                             trWidget, SLOT(refresh()));
            // Forward signals to the outside
            QObject::connect(trWidget, SIGNAL(notifyProblem(QString)),
                             this, SIGNAL(notifyProblem(QString)));
            QObject::connect(trWidget, SIGNAL(notifyInfo(QString)),
                             this, SIGNAL(notifyInfo(QString)));
            QObject::connect(trWidget, SIGNAL(gameUpdated()),
                             this, SIGNAL(gameUpdated()));
            addTab(trWidget, qfw(player.getName()));
        }
        else
        {
            // Add one tab per player
            for (unsigned int i = 0; i < iGame->getNbPlayers(); ++i)
            {
                const Player &player = iGame->getPlayer(i);
                PlayerWidget *p = new PlayerWidget(NULL, i, iGame);
                QObject::connect(this, SIGNAL(refreshSignal()), p, SLOT(refresh()));
                // Forward signals to the outside
                QObject::connect(p, SIGNAL(notifyProblem(QString)),
                                 this, SIGNAL(notifyProblem(QString)));
                QObject::connect(p, SIGNAL(gameUpdated()),
                                 this, SIGNAL(gameUpdated()));
                addTab(p, qfw(player.getName()));
                // Switching to a tab corresponding to an AI player
                // is forbidden
                if (!player.isHuman())
                    setTabEnabled(i, false);
            }
            setCurrentIndex(iGame->getCurrentPlayer().getId());
        }
    }
}


void PlayerTabWidget::refresh()
{
    if (m_game)
        setCurrentIndex(m_game->getCurrentPlayer().getId());
    emit refreshSignal();
}


void PlayerTabWidget::changeCurrentPlayer(int p)
{
    // Change the active player when the active tab changes
    // (only in duplicate mode)
    if (m_game->getMode() == PublicGame::kDUPLICATE &&
        widget(p)->isEnabled())
    {
        m_game->duplicateSetPlayer(p);
    }
}

