/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: cross.cpp,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#include "cross.h"


Cross::Cross()
{
    // the default behaviour is to match everything
    m_any = true;
}


void Cross::clear()
{
    m_tilesSet.clear();
    m_any = false;
}


bool Cross::check(const Tile& iTile) const
{
    if (m_any || (iTile.isJoker() && !m_tilesSet.empty()))
        return true;
    set<Tile>::const_iterator it = m_tilesSet.find(iTile);
    return it != m_tilesSet.end();
}
