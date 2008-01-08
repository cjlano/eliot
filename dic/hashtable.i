/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#include <cstdlib>

#include "hashtable.h"


template<typename KEY, typename VALUE, typename HASH_FCN>
HashTable<KEY, VALUE, HASH_FCN>::HashTable(unsigned int iSize)
    : m_size(iSize)
{
    m_nodes = new const Node*[m_size];
    for (unsigned int i = 0; i < m_size; ++i)
    {
        m_nodes[i] = NULL;
    }
}


template<typename KEY, typename VALUE, typename HASH_FCN>
HashTable<KEY, VALUE, HASH_FCN>::~HashTable()
{
    for (unsigned int i = 0; i < m_size; ++i)
        delete m_nodes[i];
    delete[] m_nodes;
}


template<typename KEY, typename VALUE, typename HASH_FCN>
HashTable<KEY, VALUE, HASH_FCN>::Node::Node(const KEY &iKey, const VALUE &iValue, const Node *iNext)
    : m_key(iKey), m_value(iValue), m_next(iNext)
{
}


template<typename KEY, typename VALUE, typename HASH_FCN>
HashTable<KEY, VALUE, HASH_FCN>::Node::~Node()
{
    delete m_next;
}


template<typename KEY, typename VALUE, typename HASH_FCN>
const VALUE *HashTable<KEY, VALUE, HASH_FCN>::find(const KEY &iKey) const
{
    HASH_FCN aHashFunc;
    unsigned int h_key = aHashFunc(iKey) % m_size;
    for (const Node *entry = m_nodes[h_key]; entry; entry = entry->m_next)
    {
        // Note: we need to be able to call == on a type KEY
        if (entry->m_key == iKey)
        {
            return &entry->m_value;
        }
    }
    return NULL;
}


template<typename KEY, typename VALUE, typename HASH_FCN>
void HashTable<KEY, VALUE, HASH_FCN>::add(const KEY &iKey, const VALUE &iValue)
{
    HASH_FCN aHashFunc;
    unsigned int h_key = aHashFunc(iKey) % m_size;
    const Node *entry = new Node(iKey, iValue, m_nodes[h_key]);
    m_nodes[h_key] = entry;
}

// vim: ft=cpp
