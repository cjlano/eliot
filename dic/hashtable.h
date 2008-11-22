/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#ifndef _HASHTABLE_H
#define _HASHTABLE_H


/// Compute a hash for the data pointed to by iPtr
/**
 * This function is useful to define the HASH_FCN template parameter
 * of HashTable.
 */
unsigned int HashPtr(const void *iPtr, unsigned int iSize);


template<typename KEY, typename VALUE, typename HASH_FCN>
class HashTable
{
    public:
        /// Constructor taking the number of records in the table
        HashTable(unsigned int iSize);

        /// Destructor
        ~HashTable();

        /// Return the number of records in the table
        unsigned int size() const { return m_size; }

        /// Return the value corresponding to the given key, or NULL if not found
        const VALUE *find(const KEY &iKey) const;

        /// Add a new key/value pair (both the key and the value are copied)
        void add(const KEY& iKey, const VALUE &iValue);

    private:
        /// Maximum number of records
        unsigned int m_size;

        /// Definition of a record
        class Node
        {
            public:
                Node(const KEY &iKey, const VALUE &iValue, const Node *iNext);
                ~Node();
                KEY m_key;
                VALUE m_value;
                const Node *m_next;
        };

        /// All the nodes
        const Node **m_nodes;
};

// Include the implementation of the template
#include "hashtable.i"

#endif /* _HASHTABLE_H_ */
