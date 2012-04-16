/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#ifndef VALIDATOR_FACTORY_H_
#define VALIDATOR_FACTORY_H_

#include <QObject>
#include "logging.h"

class QValidator;
class QLineEdit;
class Dictionary;
class Bag;
class History;


class ValidatorFactory: public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    /**
     * Create a validator suitable for changing letters.
     * The given QLineEdit contains the current rack.
     */
    static QValidator *newChangeValidator(QObject *parent,
                                          const QLineEdit &iRackLineEdit,
                                          const Dictionary &iDic);

    /**
     * Create a validator suitable for setting rack letters.
     */
    static QValidator *newRackValidator(QObject *parent,
                                        const Bag &iBag,
                                        bool checkDuplicate = false,
                                        const History *iHistory = 0,
                                        int iMaxLetters = 0);

    /**
     * Create a validator suitable for setting rack letters
     * in the dictionary window.
     */
    static QValidator *newDicRackValidator(QObject *parent,
                                           const Dictionary *iDic,
                                           bool acceptJoker = false);

    /**
     * Create a validator suitable for entering a regular expression
     * in the dictionary window.
     */
    static QValidator *newRegexpValidator(QObject *parent,
                                          const Dictionary *iDic);

    /**
     * Create a validator suitable for playing a word.
     */
    static QValidator *newPlayWordValidator(QObject *parent,
                                            const Dictionary &iDic);

    /**
     * Create a validator suitable for entering board coordinates.
     */
    static QValidator *newCoordsValidator(QObject *parent);
};

#endif

