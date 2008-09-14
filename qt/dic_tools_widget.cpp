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

#include "config.h"

#include <map>
#include <vector>
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QToolTip>
#include <QtCore/QString>

#include "dic_tools_widget.h"
#include "qtcommon.h"
#include "dic.h"
#include "header.h"
#include "dic_exception.h"

using namespace std;


/// Validator used for the line edits accepting only dictionary characters
class DicRackValidator: public QValidator
{
public:
    explicit DicRackValidator(QObject *parent,
                              const Dictionary *iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary *m_dic;
};


/// Validator used for the regexp line edit
class RegexpValidator: public QValidator
{
public:
    explicit RegexpValidator(QObject *parent,
                             const Dictionary *iDic);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary *m_dic;
};


DicToolsWidget::DicToolsWidget(QWidget *parent)
    : QWidget(parent), m_dic(NULL)
{
    setupUi(this);

    redPalette = labelCheck->palette();
    redPalette.setColor(QPalette::Foreground, Qt::red);
    greenPalette = labelCheck->palette();
    greenPalette.setColor(QPalette::Foreground, Qt::darkGreen);

    labelLimitReached->hide();

    // Create connections
    QObject::connect(lineEditCheck, SIGNAL(textChanged(const QString&)), this, SLOT(refreshCheck()));
    QObject::connect(lineEditPlus1, SIGNAL(returnPressed()), this, SLOT(refreshPlus1()));
    QObject::connect(lineEditRegexp, SIGNAL(returnPressed()), this, SLOT(refreshRegexp()));

    // Create models
    m_plus1Model = new QStandardItemModel(this);
    treeViewPlus1->setModel(m_plus1Model);
    m_plus1Model->setColumnCount(1);
    m_plus1Model->setHeaderData(0, Qt::Horizontal, _q("Rack:"), Qt::DisplayRole);

    m_regexpModel = new QStandardItemModel(this);
    treeViewRegexp->setModel(m_regexpModel);
    m_regexpModel->setColumnCount(1);
    m_regexpModel->setHeaderData(0, Qt::Horizontal, _q("Rack:"), Qt::DisplayRole);

    m_dicInfoModel = new QStandardItemModel(this);
    treeViewDicLetters->setModel(m_dicInfoModel);
    m_dicInfoModel->setColumnCount(5);
    m_dicInfoModel->setHeaderData(0, Qt::Horizontal, _q("Letter"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(1, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(2, Qt::Horizontal, _q("Frequency"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(3, Qt::Horizontal, _q("Vowel?"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(4, Qt::Horizontal, _q("Consonant?"), Qt::DisplayRole);
}


void DicToolsWidget::setDic(const Dictionary *iDic)
{
    if (m_dic != iDic)
    {
        m_dic = iDic;
        // Reset the letters
        lineEditCheck->clear();
        lineEditPlus1->clear();
        lineEditRegexp->clear();
        // Create new validators
        lineEditCheck->setValidator(new DicRackValidator(this, m_dic));
        lineEditPlus1->setValidator(new DicRackValidator(this, m_dic));
        lineEditRegexp->setValidator(new RegexpValidator(this, m_dic));
        // Refresh
        refreshCheck();
        refreshPlus1();
        refreshRegexp();
        refreshDicInfo();
    }
}


void DicToolsWidget::refreshCheck()
{
    QLineEdit *rack = lineEditCheck;
    if (m_dic == NULL)
    {
        labelCheck->setText(_q("Please select a dictionary"));
        return;
    }

    if (rack->text() == "")
        labelCheck->setText("");
    else
    {
        bool res = m_dic->searchWord(qtw(rack->text()));
        if (res)
        {
            labelCheck->setText(_q("The word '%1' exists").arg(rack->text().toUpper()));
            labelCheck->setPalette(greenPalette);
        }
        else
        {
            labelCheck->setText(_q("The word '%1' does not exist").arg(rack->text().toUpper()));
            labelCheck->setPalette(redPalette);
        }
    }
}


void DicToolsWidget::refreshPlus1()
{
    QStandardItemModel *model = m_plus1Model;
    QTreeView *treeView = treeViewPlus1;
    QLineEdit *rack = lineEditPlus1;

    model->removeRows(0, model->rowCount());
    if (m_dic == NULL)
    {
        model->setHeaderData(0, Qt::Horizontal,
                               _q("Please select a dictionary"),
                               Qt::DisplayRole);
        return;
    }
    else
    {
        model->setHeaderData(0, Qt::Horizontal,
                               _q("Rack: %1").arg(rack->text().toUpper()),
                               Qt::DisplayRole);
    }

    if (rack->text() != "")
    {
        map<wchar_t, vector<wstring> > wordList;
        m_dic->search7pl1(qtw(rack->text()), wordList, false);

        int rowNum = 0;
        map<wchar_t, vector<wstring> >::const_iterator it;
        for (it = wordList.begin(); it != wordList.end(); it++)
        {
            // Create the header line
            model->insertRow(rowNum);
            const QModelIndex &index = model->index(rowNum, 0);
            if (it->first)
                model->setData(index, qfw(wstring(1, it->first)));
            else
                model->setData(index, _q("Anagrams"));
            treeView->setExpanded(index, true);
            ++rowNum;

            // Create the result lines
            model->insertColumn(0, index);
            model->insertRows(0, it->second.size(), index);
            const vector<wstring> &results = it->second;
            for (unsigned i = 0; i < results.size(); ++i)
            {
                model->setData(model->index(i, 0, index),
                                 qfw(results[i]));
            }
        }
    }
}


void DicToolsWidget::refreshRegexp()
{
    QStandardItemModel *model = m_regexpModel;
    QLineEdit *rack = lineEditRegexp;

    model->removeRows(0, model->rowCount());
    if (m_dic == NULL)
    {
        model->setHeaderData(0, Qt::Horizontal,
                               _q("Please select a dictionary"),
                               Qt::DisplayRole);
        return;
    }
    else
    {
        model->setHeaderData(0, Qt::Horizontal,
                               _q("Regular expression: %1").arg(rack->text().toUpper()),
                               Qt::DisplayRole);
    }

    if (rack->text() != "")
    {
        unsigned lmin = spinBoxMinLength->value();
        unsigned lmax = spinBoxMaxLength->value();
        // FIXME: this value should not be hardcoded,
        // or a warning should appear when it is reached
        unsigned limit = 1000;
        vector<wstring> wordList;
        bool res = true;
        int rowNum = 0;
        try
        {
            res = m_dic->searchRegExp(qtw(rack->text()), wordList,
                                      lmin, lmax, limit);
        }
        catch (InvalidRegexpException &e)
        {
            model->insertRow(rowNum);
            model->setData(model->index(rowNum, 0),
                           _q("Invalid regular expression: %1").arg(qfl(e.what())));
            model->setData(model->index(rowNum, 0),
                           QBrush(Qt::red), Qt::ForegroundRole);
        }

        vector<wstring>::const_iterator it;
        for (it = wordList.begin(); it != wordList.end(); it++)
        {
            model->insertRow(rowNum);
            model->setData(model->index(rowNum, 0), qfw(*it));
            ++rowNum;
        }
        if (res)
            labelLimitReached->hide();
        else
            labelLimitReached->show();
    }
}


void DicToolsWidget::refreshDicInfo()
{
    if (m_dic == NULL)
    {
        lineEditName->setText("");
        lineEditLetters->setText("");
        spinBoxWords->setValue(0);
        m_dicInfoModel->clear();
    }
    else
    {
        const Header &header = m_dic->getHeader();
        lineEditName->setText(qfw(header.getName()));
        lineEditLetters->setText(qfw(header.getLetters()));
        spinBoxWords->setValue(header.getNbWords());

        QStandardItemModel *model = m_dicInfoModel;
        model->removeRows(0, model->rowCount());
        const vector<Tile> &allTiles = m_dic->getAllTiles();
        vector<Tile>::const_iterator it;
        int rowNum = 0;
        for (it = allTiles.begin(); it != allTiles.end(); ++it)
        {
            model->insertRow(rowNum);
            model->setData(model->index(rowNum, 0),
                           qfw(wstring(1, it->toChar())));
            model->setData(model->index(rowNum, 1), it->getPoints());
            model->setData(model->index(rowNum, 2), it->maxNumber());
            model->setData(model->index(rowNum, 3), it->isVowel());
            model->setData(model->index(rowNum, 4), it->isConsonant());
            ++rowNum;
        }
        treeViewDicLetters->resizeColumnToContents(0);
        treeViewDicLetters->resizeColumnToContents(1);
        treeViewDicLetters->resizeColumnToContents(2);
        treeViewDicLetters->resizeColumnToContents(3);
        treeViewDicLetters->resizeColumnToContents(4);
    }
}



DicRackValidator::DicRackValidator(QObject *parent,
                                   const Dictionary *iDic)
    : QValidator(parent), m_dic(iDic)
{
}


QValidator::State DicRackValidator::validate(QString &input, int &) const
{
    if (m_dic == NULL)
        return Invalid;

    if (input == "")
        return Intermediate;

    //input = input.toUpper();

    // The string is invalid if it contains characters not present
    // in the dictionary or if it contains a '?'
    if (!m_dic->validateLetters(qtw(input)) || input.contains('?'))
        return Invalid;
    return Acceptable;
}



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

    // Strip regular expression characters
    QString copy(input);
    QString authorizedChars = ".[]()*+?:^";
    for (int i = 0; i < authorizedChars.size(); ++i)
    {
        copy.remove(authorizedChars[i]);
    }

    // The string is invalid if it contains characters not present
    // in the dictionary
    if (!m_dic->validateLetters(qtw(copy)))
        return Invalid;
    return Acceptable;
}

