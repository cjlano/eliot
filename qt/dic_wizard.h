/*****************************************************************************
 * Eliot
 * Copyright (C) 2010 Olivier Teulière
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

#ifndef DIC_WIZARD_H_
#define DIC_WIZARD_H_

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>
#include <QtGui/QStyledItemDelegate>

#include "ui/dic_wizard_info_page.ui.h"
#include "ui/dic_wizard_letters_def_page.ui.h"
#include "ui/dic_wizard_conclusion_page.ui.h"
#include "logging.h"

class QStandardItemModel;
class QItemSelection;
class QStyleOptionViewItem;
class QModelIndex;
class Dictionary;

class DicWizard: public QWizard
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    DicWizard(QWidget *parent, const Dictionary *iCurrDic);
    virtual void accept();

private:
    int m_lettersPageId;

signals:
    void loadDictionary(QString dawgFile);
    void notifyInfo(QString msg);
    void notifyProblem(QString msg);
};


class WizardInfoPage: public QWizardPage, private Ui::WizardInfoPage
{
    Q_OBJECT;
    DEFINE_LOGGER();
public:
    explicit WizardInfoPage(QWidget *parent = 0);
    virtual bool isComplete() const;
    virtual bool validatePage();

signals:
    void notifyProblem(QString msg);

private slots:
    void onBrowseGenDicClicked();
    void onBrowseWordListClicked();
};


class WizardLettersDefPage: public QWizardPage, private Ui::WizardLettersDefPage
{
    Q_OBJECT;
    DEFINE_LOGGER();
public:
    explicit WizardLettersDefPage(const Dictionary *iCurrDic, QWidget *parent = 0);
    const QStandardItemModel * getModel() const { return m_model; }

signals:
    void notifyProblem(QString msg);

private:
    QStandardItemModel *m_model;
    const Dictionary *m_currDic;

private slots:
    void loadLettersFromWordList();
    void loadValuesFromDic();
};


class WizardConclusionPage: public QWizardPage, private Ui::WizardConclusionPage
{
    Q_OBJECT;
    DEFINE_LOGGER();
public:
    explicit WizardConclusionPage(QWidget *parent = 0);
    virtual void initializePage();
};


class LettersDelegate: public QStyledItemDelegate
{
    Q_OBJECT;

public:
    explicit LettersDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent) {}

    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
};

#endif

