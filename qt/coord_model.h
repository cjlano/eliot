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

#ifndef COORD_MODEL_H_
#define COORD_MODEL_H_

#include <QObject>
#include "coord.h"


class CoordModel: public QObject
{
    Q_OBJECT;

public:
    void setCoord(const Coord &iCoord);
    void clear();

    const Coord &getCoord() const { return m_currCoord; }
    const Coord &getPrevCoord() const { return m_prevCoord; }

signals:
    void coordChanged(const Coord &iNewCoord);

private:
    Coord m_currCoord;
    Coord m_prevCoord;
};

#endif

