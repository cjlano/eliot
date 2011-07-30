/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2009 Olivier Teulière
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
#include <algorithm>
#include <fstream>
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QToolTip>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QString>

#include "dic_tools_widget.h"
#include "custom_popup.h"
#include "qtcommon.h"
#include "dic.h"
#include "header.h"
#include "listdic.h"
#include "dic_exception.h"

using namespace std;


/// Validator used for the line edits accepting only dictionary characters
class DicRackValidator: public QValidator
{
public:
    explicit DicRackValidator(QObject *parent,
                              const Dictionary *iDic,
                              bool acceptJoker = false);
    virtual State validate(QString &input, int &pos) const;

private:
    const Dictionary *m_dic;
    const bool m_acceptJoker;
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
    darkYellowPalette = labelCheck->palette();
    darkYellowPalette.setColor(QPalette::Foreground, Qt::darkYellow);
    greenPalette = labelCheck->palette();
    greenPalette.setColor(QPalette::Foreground, Qt::darkGreen);

    labelLimitReached->hide();

    // Create connections
    QObject::connect(lineEditCheck, SIGNAL(textChanged(const QString&)), this, SLOT(refreshCheck()));
    QObject::connect(lineEditPlus1, SIGNAL(returnPressed()), this, SLOT(refreshPlus1()));
    QObject::connect(lineEditRegexp, SIGNAL(returnPressed()), this, SLOT(refreshRegexp()));
    QObject::connect(buttonSaveWords, SIGNAL(clicked()), this, SLOT(exportWordsList()));

    // Add context menus for the results
    m_customPopupPlus1 = new CustomPopup(treeViewPlus1);
    QObject::connect(m_customPopupPlus1, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateMenuPlus1(QMenu&, const QPoint&)));
    QObject::connect(m_customPopupPlus1, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));

    m_customPopupRegexp = new CustomPopup(treeViewRegexp);
    QObject::connect(m_customPopupRegexp, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateMenuRegexp(QMenu&, const QPoint&)));
    QObject::connect(m_customPopupRegexp, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));

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
    m_dicInfoModel->setColumnCount(6);
    m_dicInfoModel->setHeaderData(0, Qt::Horizontal, _q("Letter"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(1, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(2, Qt::Horizontal, _q("Frequency"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(3, Qt::Horizontal, _q("Vowel?"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(4, Qt::Horizontal, _q("Consonant?"), Qt::DisplayRole);
    m_dicInfoModel->setHeaderData(5, Qt::Horizontal, _q("Alternative inputs"), Qt::DisplayRole);
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
        lineEditPlus1->setValidator(new DicRackValidator(this, m_dic, true));
        lineEditRegexp->setValidator(new RegexpValidator(this, m_dic));
        // Refresh
        refreshCheck();
        refreshPlus1();
        refreshRegexp();
        refreshDicInfo();
    }
}


void DicToolsWidget::setPlus1Rack(const QString &iRack)
{
    lineEditPlus1->setText(iRack);
    refreshPlus1();
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
        if (!rack->hasAcceptableInput())
        {
            labelCheck->setText(_q("Invalid or incomplete letters"));
            labelCheck->setPalette(darkYellowPalette);
            return;
        }

        wstring input = m_dic->convertFromInput(wfq(rack->text()));
        bool res = m_dic->searchWord(input);
        // Convert the input to uppercase
        std::transform(input.begin(), input.end(), input.begin(), towupper);
        const wdstring &dispStr = m_dic->convertToDisplay(input);
        if (res)
        {
            labelCheck->setText(_q("The word '%1' exists").arg(qfw(dispStr)));
            labelCheck->setPalette(greenPalette);
        }
        else
        {
            labelCheck->setText(_q("The word '%1' does not exist").arg(qfw(dispStr)));
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

    const wstring &input = m_dic->convertFromInput(wfq(rack->text().toUpper()));
    const wdstring &disp = m_dic->convertToDisplay(input);
    model->setHeaderData(0, Qt::Horizontal,
                         _q("Rack: %1").arg(qfw(disp)),
                         Qt::DisplayRole);

    if (input != L"")
    {
        map<unsigned int, vector<wstring> > wordList;
        m_dic->search7pl1(input, wordList, true);

        int rowNum = 0;
        map<unsigned int, vector<wstring> >::const_iterator it;
        for (it = wordList.begin(); it != wordList.end(); it++)
        {
            // Create the header line
            model->insertRow(rowNum);
            const QModelIndex &index = model->index(rowNum, 0);
            if (it->first != 0)
                model->setData(index, qfw(m_dic->getHeader().getDisplayStr(it->first)));
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

    const wstring &input = m_dic->convertFromInput(wfq(rack->text().toUpper()));
    const wdstring &disp = m_dic->convertToDisplay(input);
    model->setHeaderData(0, Qt::Horizontal,
                         _q("Regular expression: %1").arg(qfw(disp)),
                         Qt::DisplayRole);

    if (input != L"")
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
            res = m_dic->searchRegExp(input, wordList, lmin, lmax, limit);
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
        lineEditLetters->setText(qfw(m_dic->convertToDisplay(header.getLetters())));
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
                           qfw(it->getDisplayStr()));
            model->setData(model->index(rowNum, 1), it->getPoints());
            model->setData(model->index(rowNum, 2), it->maxNumber());
            model->setData(model->index(rowNum, 3),
                           it->isVowel() ? _q("Yes") : _q("No"));
            model->setData(model->index(rowNum, 4),
                           it->isConsonant() ? _q("Yes") : _q("No"));
            const vector<wistring> &inputVect = it->getInputStr();
            wstring tmp;
            for (unsigned int i = 1; i < inputVect.size(); ++i)
            {
                tmp += inputVect[i] + L"  ";
            }
            model->setData(model->index(rowNum, 5), qfw(tmp));
            // Center the text in the column
            for (int col = 0; col < 5; ++col)
            {
                model->item(rowNum, col)->setTextAlignment(Qt::AlignCenter);
            }

            ++rowNum;
        }
        treeViewDicLetters->resizeColumnToContents(0);
        treeViewDicLetters->resizeColumnToContents(1);
        treeViewDicLetters->resizeColumnToContents(2);
        treeViewDicLetters->resizeColumnToContents(3);
        treeViewDicLetters->resizeColumnToContents(4);
    }
}


void DicToolsWidget::exportWordsList()
{
    if (m_dic == NULL)
        return;
    QString fileName = QFileDialog::getSaveFileName(this, _q("Export words list"));
    if (fileName != "")
    {
        try
        {
            ofstream file(lfq(fileName).c_str());
            ListDic::printWords(file, *m_dic);
            QMessageBox::information(this, _q("Export words list"),
                                     _q("File '%1' successfully saved").arg(fileName));
        }
        catch (std::exception &e)
        {
            QMessageBox::warning(this, _q("Eliot - Error"),
                                 _q("Cannot save the words list: %1").arg(e.what()));
        }
    }
}


void DicToolsWidget::populateMenuPlus1(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewPlus1->indexAt(iPoint);
    if (!index.isValid() || !index.parent().isValid())
        return;

    // Find the selected word
    const QModelIndex &wordIndex = m_plus1Model->index(index.row(), 0, index.parent());
    QString selectedWord = m_plus1Model->data(wordIndex).toString();

    m_customPopupPlus1->addShowDefinitionEntry(iMenu, selectedWord);
}


void DicToolsWidget::populateMenuRegexp(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewRegexp->indexAt(iPoint);
    if (!index.isValid())
        return;

    // Find the selected word
    const QModelIndex &wordIndex = m_regexpModel->index(index.row(), 0);
    QString selectedWord = m_regexpModel->data(wordIndex).toString();

    m_customPopupRegexp->addShowDefinitionEntry(iMenu, selectedWord);
}



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

