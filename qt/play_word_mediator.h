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

#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include <QtGui/QValidator>


class QLineEdit;
class QPushButton;
class PublicGame;
class Coord;
class CoordModel;
class Dictionary;

/**
 * Mediator handling the connections between the controls used to play a word.
 * The controls are:
 *  - a QLineEdit for the played word
 *  - a QLineEdit for the coordinates
 *  - a Play button to really play the word
 *
 * All the logic between these controls is handled by the mediator, as long as
 * it is alive (same lifetime as the controls themselves).
 * Note: this class is not a graphical widget, because it would cause problems
 * to place the controls exactly where wanted in the QGridLayout of the
 * PlayerWidget class...
 */
class PlayWordMediator: public QObject
{
    Q_OBJECT;

public:
    PlayWordMediator(QObject *parent, QLineEdit &iEditWord,
                     QLineEdit &iEditCoord, QPushButton &iButtonPlay,
                     CoordModel &iCoordModel, PublicGame *iGame);

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);

private slots:
    void lineEditPlay_textChanged();
    void lineEditPlay_returnPressed();
    void lineEditCoord_textChanged(const QString &iText);
    void lineEditCoord_returnPressed() { lineEditPlay_returnPressed(); }
    void pushButtonPlay_clicked() { lineEditPlay_returnPressed(); }
    void updateCoord(const Coord &, const Coord &iNewCoord);

private:
    PublicGame *m_game;
    QLineEdit &m_lineEditPlay;
    QLineEdit &m_lineEditCoord;
    QPushButton &m_pushButtonPlay;
    CoordModel &m_coordModel;
};

#endif

