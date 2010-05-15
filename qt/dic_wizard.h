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

#include "ui/dic_wizard_info_page.ui.h"
#include "ui/dic_wizard_letters_def_page.ui.h"

class QStandardItemModel;
class QItemSelection;

class DicWizard: public QWizard
{
    Q_OBJECT;

public:
    DicWizard(QWidget *parent);

private:
    QWizardPage *createLettersDefPage() const;
};


class WizardInfoPage: public QWizardPage, private Ui::WizardInfoPage
{
    Q_OBJECT
public:
    explicit WizardInfoPage(QWidget *parent = 0);
    virtual bool isComplete() const;
    virtual bool validatePage();

private slots:
    void onBrowseGenDicClicked();
    void onBrowseWordListClicked();
};


class WizardLettersDefPage: public QWizardPage, private Ui::WizardLettersDefPage
{
    Q_OBJECT
public:
    explicit WizardLettersDefPage(QWidget *parent = 0);
    virtual bool isComplete() const;

private:
    QStandardItemModel *m_model;

private slots:
    void loadLettersFromWordList();
    void enableRemoveButton(const QItemSelection &);
    void removeLetter();
};


#endif

