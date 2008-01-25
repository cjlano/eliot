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
#include "game.h"
#include "freegame.h"
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


/// Validator used for the "change letters" line edit
class PlayWordValidator: public QValidator
{
public:
    explicit PlayWordValidator(QObject *parent,
                               const Dictionary &iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary &m_dic;
};


PlayerWidget::PlayerWidget(QWidget *parent, unsigned int iPlayerNb, Game *iGame)
    : QWidget(parent), m_game(iGame), m_player(iPlayerNb)
{
    setupUi(this);

    if (m_game)
    {
        lineEditPlay->setValidator(new PlayWordValidator(this, m_game->getDic()));

        // Do not allow messing with AI players
        if (!m_game->getPlayer(m_player).isHuman())
            setEnabled(false);

        // Changing the rack is authorized only in training mode
        lineEditRack->setReadOnly(m_game->getMode() != Game::kTRAINING);
    }
    if (m_game == NULL || m_game->getMode() != Game::kFREEGAME)
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
    lineEditPlay->clear();

    if (m_game == NULL)
    {
        lineEditRack->clear();
        return;
    }

    const PlayedRack &pld = m_game->getPlayer(m_player).getCurrentRack();
    lineEditRack->setText(qfw(pld.toString(PlayedRack::RACK_EXTRA)));
    lineEditPlay->clear();
    lineEditChange->clear();
}


void PlayerWidget::on_pushButtonShuffle_clicked()
{
    // TODO: (not supported in the core yet)
}


void PlayerWidget::on_lineEditPlay_textChanged()
{
    pushButtonPlay->setEnabled(lineEditPlay->hasAcceptableInput());
}


void PlayerWidget::on_lineEditChange_textChanged()
{
    pushButtonChange->setEnabled(lineEditChange->hasAcceptableInput() &&
                                 lineEditChange->text() != "");
    pushButtonPass->setEnabled(lineEditChange->text() == "");
}


void PlayerWidget::on_lineEditPlay_returnPressed()
{
    QStringList items = lineEditPlay->text().split(' ', QString::SkipEmptyParts);
    ASSERT(items.size() == 2, "Bug found in the validator");

    // Play the word
    int res = m_game->play(qtw(items[1]), qtw(items[0]));
    if (res == 0)
    {
        emit gameUpdated();
    }
    else
    {
        // FIXME: the error is too generic
        QString msg = _q("Cannot play '%1' at position '%2': incorrect or misplaced word (%3)")
            .arg(items[0]).arg(items[1]).arg(res);
        emit notifyProblem(msg);
    }
}


void PlayerWidget::on_lineEditChange_returnPressed()
{
    FreeGame *free = dynamic_cast<FreeGame*>(m_game);
    ASSERT(free != NULL,
           "Trying to pass or change letters while not in free game mode");

    // Pass the turn (and possibly change letters)
    QString letters = lineEditChange->text();
    int res = free->pass(qtw(letters));
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

    QStringList items = input.split(' ', QString::SkipEmptyParts);

    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic.validateLetters(qtw(items[0])))
        return Invalid;

    if (items.size() != 2 || items[0].size() < 2)
        return Intermediate;

    // Check coordinates
    Coord c(qtw(items[1]));
    if (!c.isValid())
        return Intermediate;

    return Acceptable;
}



PlayerTabWidget::PlayerTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
}


void PlayerTabWidget::setGame(Game *iGame)
{
    // Remove all the tabs
    int nbTabs = count();
    for (int i = 0; i < nbTabs; ++i)
    {
        setCurrentWidget(0);
        // Cut all the connections with the page (needed because removeTab()
        // doesn't really destroy the widget)
        disconnect(currentWidget());
        removeTab(0);
    }

    if (iGame != NULL)
    {
        // Training mode: use a dedicated widget
        if (iGame->getMode() == Game::kTRAINING)
        {
            const Player &player = iGame->getPlayer(0);
            TrainingWidget *trWidget = new TrainingWidget;
            trWidget->setGame(iGame);
            QObject::connect(this, SIGNAL(refreshSignal()),
                             trWidget, SLOT(refresh()));
            // Forward the gameUpdated() signal to the outside
            QObject::connect(trWidget, SIGNAL(gameUpdated()),
                             this, SIGNAL(gameUpdated()));
            addTab(trWidget, qfw(player.getName()));
        }
        else
        {
            // Add one tab per player
            for (unsigned int i = 0; i < iGame->getNPlayers(); ++i)
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
            }
        }
    }
}


void PlayerTabWidget::refresh()
{
    emit refreshSignal();
}


