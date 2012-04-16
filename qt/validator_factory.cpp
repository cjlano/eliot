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

#include <QtGui/QLineEdit>
#include <QtGui/QValidator>

#include "validator_factory.h"
#include "qtcommon.h"

#include "dic.h"
#include "bag.h"
#include "coord.h"
#include "history.h"
#include "pldrack.h"

// TODO: There is probably a good potential for code factorization in this file


INIT_LOGGER(qt, ValidatorFactory);

// {{{ ChangeValidator: Validator used for the "change letters" line edit
class ChangeValidator: public QValidator
{
public:
    explicit ChangeValidator(QObject *parent,
                             const QLineEdit &iLineEdit,
                             const Dictionary &iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const QLineEdit &m_lineEdit;
    const Dictionary &m_dic;
};


ChangeValidator::ChangeValidator(QObject *parent,
                                 const QLineEdit &iLineEdit,
                                 const Dictionary &iDic)
    : QValidator(parent), m_lineEdit(iLineEdit), m_dic(iDic)
{
}


QValidator::State ChangeValidator::validate(QString &input, int &) const
{
    // The string is invalid if it contains invalid input characters
    const wistring &winput = wfq(input);
    if (!m_dic.validateInputChars(winput))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = m_dic.convertFromInput(winput);
    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic.validateLetters(intInput))
        return Intermediate;

    const wstring &rack = m_dic.convertFromInput(wfq(m_lineEdit.text()));
    if (intInput.size() > rack.size())
        return Intermediate;
    // The letters to change must be in the rack
    // We convert back to QString objects, because their count() method is
    // very practical...
    QString qrack = qfw(rack);
    QString qinput = qfw(intInput);
    for (int i = 0; i < qinput.size(); ++i)
    {
        if (qinput.count(qinput[i], Qt::CaseInsensitive) >
            qrack.count(qinput[i], Qt::CaseInsensitive))
        {
            return Intermediate;
        }
    }
    return Acceptable;
}


QValidator *ValidatorFactory::newChangeValidator(QObject *parent,
                                                 const QLineEdit &iRackLineEdit,
                                                 const Dictionary &iDic)
{
    return new ChangeValidator(parent, iRackLineEdit, iDic);
}
// }}}


// {{{ RackValidator: Validator used for the rack line edit
class RackValidator: public QValidator
{
public:
    RackValidator(QObject *parent, const Bag &iBag,
                  const History *iHistory, bool checkDuplicate,
                  int iMaxLetters);
    virtual State validate(QString &input, int &pos) const;

private:
    const Bag &m_bag;
    const History *m_history;
    bool m_checkDuplicate;
    int m_maxLetters;
};


RackValidator::RackValidator(QObject *parent, const Bag &iBag,
                             const History *iHistory, bool checkDuplicate,
                             int iMaxLetters)
    : QValidator(parent), m_bag(iBag),
    m_history(iHistory), m_checkDuplicate(checkDuplicate),
    m_maxLetters(iMaxLetters)
{
}


QValidator::State RackValidator::validate(QString &input, int &) const
{
    if (input == "")
        return Intermediate;

    input = input.toUpper();

    const Dictionary &dic = m_bag.getDic();

    // The string is invalid if it contains invalid input characters
    const wistring &winput = wfq(input);
    if (!dic.validateInputChars(winput))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = dic.convertFromInput(winput);
    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!dic.validateLetters(intInput))
        return Intermediate;

    QString qinput = qfw(intInput);
    // The letters must be in the bag
    for (int i = 0; i < qinput.size(); ++i)
    {
        if ((unsigned int)qinput.count(qinput[i], Qt::CaseInsensitive) >
            m_bag.in(intInput[i]))
        {
            return Invalid;
        }
    }

    // Make sure we don't have too many letters...
    if (m_maxLetters > 0 && intInput.size() > (unsigned)m_maxLetters)
        return Intermediate;
    // ... or too few
    if (m_maxLetters > 0 && intInput.size() < (unsigned)m_maxLetters &&
        m_bag.getNbTiles() >= (unsigned)m_maxLetters)
    {
        return Intermediate;
    }

    // Check that the rack has 2 consonants and 2 vocals
    if (m_checkDuplicate)
    {
        PlayedRack pld;
        pld.setManual(intInput);

        int min;
        if (m_bag.getNbVowels() > 1 && m_bag.getNbConsonants() > 1
            && m_history->getSize() < 15)
            min = 2;
        else
            min = 1;
        if (!pld.checkRack(min, min))
            return Intermediate;
    }

    return Acceptable;
}


QValidator *ValidatorFactory::newRackValidator(QObject *parent,
                                               const Bag &iBag,
                                               bool checkDuplicate,
                                               const History *iHistory,
                                               int iMaxLetters)
{
    return new RackValidator(parent, iBag, iHistory, checkDuplicate, iMaxLetters);
}
// }}}


// {{{ DicRackValidator: Validator used for the line edits accepting only dictionary characters
class DicRackValidator: public QValidator
{
public:
    explicit DicRackValidator(QObject *parent,
                              const Dictionary *iDic,
                              bool acceptJoker);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary *m_dic;
    const bool m_acceptJoker;
};


DicRackValidator::DicRackValidator(QObject *parent,
                                   const Dictionary *iDic,
                                   bool acceptJoker)
    : QValidator(parent), m_dic(iDic), m_acceptJoker(acceptJoker)
{
}


QValidator::State DicRackValidator::validate(QString &input, int &) const
{
    if (m_dic == NULL)
        return Invalid;

    if (input == "")
        return Intermediate;

    // The string is invalid if it contains invalid input characters
    const wistring &winput = wfq(input);
    if (!m_dic->validateInputChars(winput))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = m_dic->convertFromInput(winput);
    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic->validateLetters(intInput))
        return Intermediate;

    // A '?' may not be acceptable
    if (!m_acceptJoker && input.contains('?'))
        return Invalid;
    // Do not accept more than 2 jokers
    if (input.count('?') > 2)
        return Invalid;
    return Acceptable;
}


QValidator *ValidatorFactory::newDicRackValidator(QObject *parent,
                                                  const Dictionary *iDic,
                                                  bool acceptJoker)
{
    return new DicRackValidator(parent, iDic, acceptJoker);
}
// }}}


// {{{ Validator used for the regexp line edit
class RegexpValidator: public QValidator
{
public:
    explicit RegexpValidator(QObject *parent,
                             const Dictionary *iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary *m_dic;
};


RegexpValidator::RegexpValidator(QObject *parent,
                                 const Dictionary *iDic)
    : QValidator(parent), m_dic(iDic)
{
}


QValidator::State RegexpValidator::validate(QString &input, int &) const
{
    if (m_dic == NULL)
        return Invalid;

    if (input == "")
        return Intermediate;

    wstring authorizedChars = L".[]()*+?:^";

    // The string is invalid if it contains invalid input characters
    const wistring &winput = wfq(input);
    if (!m_dic->validateInputChars(winput, authorizedChars))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = m_dic->convertFromInput(winput);
    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic->validateLetters(intInput, authorizedChars))
        return Intermediate;

    return Acceptable;
}


QValidator *ValidatorFactory::newRegexpValidator(QObject *parent,
                                                 const Dictionary *iDic)
{
    return new RegexpValidator(parent, iDic);
}
// }}}


// {{{ Validator used for the "play word" line edit
class PlayWordValidator: public QValidator
{
public:
    explicit PlayWordValidator(QObject *parent,
                               const Dictionary &iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary &m_dic;
};


PlayWordValidator::PlayWordValidator(QObject *parent,
                                     const Dictionary &iDic)
    : QValidator(parent), m_dic(iDic)
{
}


QValidator::State PlayWordValidator::validate(QString &input, int &) const
{
    if (input == "")
        return Intermediate;

    const wistring &winput = wfq(input);
    // The string is invalid if it contains invalid input characters
    if (!m_dic.validateInputChars(winput, L"()") || input.contains('?'))
        return Invalid;

    // Convert the string to internal letters
    const wstring &intInput = m_dic.convertFromInput(winput);
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
            !m_dic.validateLetters(wfq(QString(qintInput[pos + 1]))))
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


QValidator *ValidatorFactory::newPlayWordValidator(QObject *parent,
                                                   const Dictionary &iDic)
{
    return new PlayWordValidator(parent, iDic);
}
// }}}


// {{{ CoordsValidator: Validator used to enter coordinates
class CoordsValidator: public QValidator
{
public:
    explicit CoordsValidator(QObject *parent);
    virtual State validate(QString &input, int &pos) const;
};


CoordsValidator::CoordsValidator(QObject *parent)
    : QValidator(parent)
{
}


QValidator::State CoordsValidator::validate(QString &input, int &) const
{
    // Only authorize characters part of a valid coordinate
    wstring copy = wfq(input.toUpper());
    wstring authorized = L"ABCDEFGHIJKLMNO1234567890";
    if (copy.find_first_not_of(authorized) != wstring::npos)
        return Invalid;

    // Check coordinates
    Coord c(wfq(input));
    if (!c.isValid())
        return Intermediate;

    return Acceptable;
}


QValidator *ValidatorFactory::newCoordsValidator(QObject *parent)
{
    return new CoordsValidator(parent);
}
// }}}

// vim:fdm=marker fdl=0
