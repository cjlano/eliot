/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#include <algorithm>
#include <functional>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "hints_dialog.h"
#include "qtcommon.h"

#include "hints.h"
#include "debug.h"


using namespace std;

INIT_LOGGER(qt, HintsDialog);


struct CostComparator : public binary_function<const AbstractHint*, const AbstractHint*, bool>
{
    bool operator()(const AbstractHint *iHint1, const AbstractHint *iHint2) const
    {
        return iHint1->getCost() < iHint2->getCost();
    }
};


HintWidget::HintWidget(const AbstractHint &iHint,
                       bool iShowCosts, QWidget *parent)
    : QWidget(parent), m_hint(iHint)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    QString labelText = qfl(m_hint.getName());
    if (iShowCosts)
        labelText += " (" + _q("cost: %1").arg(m_hint.getCost()) + ")";
    QLabel *label = new QLabel(labelText);
    label->setToolTip(qfl(m_hint.getDescription()));
    layout->addWidget(label);

    layout->addStretch();

    QPushButton *button = new QPushButton(_q("Show"));
    button->setToolTip(qfl(m_hint.getDescription()));
    QObject::connect(button, SIGNAL(clicked()),
                     this, SLOT(buttonClicked()));
    layout->addWidget(button);

    setContentsMargins(0, 0, 0, 0);
}


void HintWidget::buttonClicked()
{
    emit hintRequested(m_hint);
}



HintsDialog::HintsDialog(QWidget *parent, bool iShowCosts)
    : QDialog(parent), m_move(NULL), m_showCosts(iShowCosts)
{
    initializeHints();

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    if (m_showCosts)
    {
        QLabel *label = new QLabel(_q("Each hint has a corresponding cost, seen as a time penalty."));
        label->setWordWrap(true);
        vLayout->addWidget(label);
    }
    Q_FOREACH(const AbstractHint *hint, m_allHints)
    {
        HintWidget *hintWidget = new HintWidget(*hint, m_showCosts);
        QObject::connect(hintWidget, SIGNAL(hintRequested(const AbstractHint&)),
                         this, SLOT(showHint(const AbstractHint&)));
        vLayout->addWidget(hintWidget);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    vLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setWindowTitle(_q("Hints"));
}


HintsDialog::~HintsDialog()
{
    Q_FOREACH(const AbstractHint *hint, m_allHints)
    {
        delete hint;
    }
}


void HintsDialog::initializeHints()
{
    m_allHints.push_back(new ScoreHint);
    m_allHints.push_back(new OrientationHint);
    m_allHints.push_back(new PositionHint);
    m_allHints.push_back(new LengthHint);
    m_allHints.push_back(new BoardLettersHint);
    m_allHints.push_back(new WordLettersHint);
    m_allHints.push_back(new FirstLetterHint);

    // Sort them by increasing cost
    CostComparator costComp;
    std::sort(m_allHints.begin(), m_allHints.end(), costComp);
}


void HintsDialog::setMove(const Move &iMove)
{
    m_move = &iMove;
}


void HintsDialog::showHint(const AbstractHint &iHint)
{
    ASSERT(m_move != 0, "No move defined");
    // Show the hint in a message box
    const string & result = iHint.giveHint(*m_move);
    QMessageBox::information(this, _q("Hint"), qfl(result));

    // Notify of the hint usage (most likely to "pay" the cost of the hint)
    emit hintUsed(iHint);
}


