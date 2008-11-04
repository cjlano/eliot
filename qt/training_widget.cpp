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

#include <QtGui/QStandardItemModel>
#include <QtGui/QValidator>

#include "training_widget.h"
#include "qtcommon.h"
#include "dic.h"
#include "game.h"
#include "training.h"
#include "player.h"
#include "results.h"

using namespace std;


/// Validator used for the rack line edit
class RackValidator: public QValidator
{
public:
    explicit RackValidator(QObject *parent);
    virtual State validate(QString &input, int &pos) const;

    void setBag(const Bag *iBag) { m_bag = iBag; }

private:
    const Bag *m_bag;
};


TrainingWidget::TrainingWidget(QWidget *parent)
    : QWidget(parent), m_game(NULL)
{
    setupUi(this);

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    treeViewResults->setModel(m_model);
    m_model->setColumnCount(6);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, "*", Qt::DisplayRole);
    m_model->setHeaderData(4, Qt::Horizontal, "", Qt::DisplayRole);
    // Hidden column, used to store internal data
    m_model->setHeaderData(5, Qt::Horizontal, "", Qt::DisplayRole);
    treeViewResults->setColumnHidden(5, true);

    // Enable the Play button only when there is a selection in the tree
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this,
                     SLOT(enablePlayButton(const QItemSelection&, const QItemSelection&)));
    // Display a preview of the selected word on the board
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this,
                     SLOT(showPreview(const QItemSelection&, const QItemSelection&)));

    m_validator = new RackValidator(this);
    lineEditRack->setValidator(m_validator);

    refresh();
}


void TrainingWidget::setGame(Game *iGame)
{
    m_game = dynamic_cast<Training*>(iGame);
    if (m_game != NULL)
        m_validator->setBag(&m_game->getBag());
    else
        m_validator->setBag(NULL);
    refresh();
}


void TrainingWidget::refresh()
{
    updateModel();
    if (m_game == NULL)
    {
        lineEditRack->setText("");
        lineEditRack->setEnabled(false);
        pushButtonRack->setEnabled(false);
        pushButtonComplement->setEnabled(false);
        pushButtonSearch->setEnabled(false);
        pushButtonPlay->setEnabled(false);
    }
    else
    {
        wstring rack = m_game->getPlayer(0).getCurrentRack().toString(PlayedRack::RACK_SIMPLE);
        // Update the rack only if it is needed, to avoid losing cursor position
        if (qfw(rack) != lineEditRack->text())
            lineEditRack->setText(qfw(rack));
        lineEditRack->setEnabled(true);
        pushButtonRack->setEnabled(true);
        pushButtonComplement->setEnabled(true);
        pushButtonSearch->setEnabled(m_model->rowCount() == 0 &&
                                     lineEditRack->text() != "");
    }
}


void TrainingWidget::updateModel()
{
    // Consider that there is nothing to do if the number of lines is correct
    // This avoids problems when the game is updated for a test play
    if (m_game != NULL &&
        m_game->getResults().size() == (unsigned int)m_model->rowCount())
    {
        return;
    }

    m_model->removeRows(0, m_model->rowCount());

    if (m_game == NULL)
        return;

    for (unsigned int i = 0; i < m_game->getResults().size(); ++i)
    {
        const Round &r = m_game->getResults().get(i);
        int rowNum = m_model->rowCount();
        m_model->insertRow(rowNum);
        m_model->setData(m_model->index(rowNum, 0), qfw(r.getWord()));
        m_model->setData(m_model->index(rowNum, 1),
                         qfw(r.getCoord().toString()));
        m_model->setData(m_model->index(rowNum, 2), r.getPoints());
        m_model->setData(m_model->index(rowNum, 3),
                         r.getBonus() ? "*": "");
        // Hidden data, used to handle proper sorting in the tree view
        m_model->setData(m_model->index(rowNum, 5), i);
    }

    // Clear the status bar when there is no search result
    if (m_model->rowCount() == 0)
        emit notifyInfo("");

    treeViewResults->resizeColumnToContents(0);
    treeViewResults->resizeColumnToContents(1);
    treeViewResults->resizeColumnToContents(2);
    treeViewResults->resizeColumnToContents(3);
}


void TrainingWidget::enablePlayButton(const QItemSelection &iSelected,
                                      const QItemSelection &)
{
    // Enable the "Play" button iff at least one line in the tree view
    // is selected
    pushButtonPlay->setEnabled(!iSelected.indexes().empty());
}


void TrainingWidget::showPreview(const QItemSelection &iSelected,
                                 const QItemSelection &)
{
    m_game->removeTestPlay();
    if (!iSelected.indexes().empty())
    {
        // Use the hidden column to get the result number
        const QModelIndex &index =
            m_model->index(iSelected.indexes().first().row(), 5);
        m_game->testPlay(m_model->data(index).toUInt());
        emit gameUpdated();
    }
}


void TrainingWidget::on_lineEditRack_textEdited(const QString &iText)
{
    // FIXME: first parameter is hardcoded
    int res = m_game->setRackManual(false, qtw(iText));
    if (res == 0)
    {
        pushButtonSearch->setEnabled(m_model->rowCount() == 0 &&
                                     lineEditRack->text() != "");
        emit gameUpdated();
    }
    else
        emit notifyProblem(_q("Warning: Cannot set the rack to '%1'").arg(iText));
}


void TrainingWidget::on_pushButtonRack_clicked()
{
    // FIXME: first parameter is hardcoded
    m_game->removeTestPlay();
    m_game->setRackRandom(true, Game::RACK_ALL);
    emit gameUpdated();
}


void TrainingWidget::on_pushButtonComplement_clicked()
{
    // FIXME: first parameter is hardcoded
    m_game->removeTestPlay();
    m_game->setRackRandom(true, Game::RACK_NEW);
    emit gameUpdated();
}


void TrainingWidget::on_pushButtonSearch_clicked()
{
    m_game->removeTestPlay();
    emit notifyInfo(_q("Searching with rack '%1'...").arg(lineEditRack->text()));
    m_game->search();
    emit notifyInfo(_q("Search done"));
    emit gameUpdated();
}


void TrainingWidget::on_pushButtonPlay_clicked()
{
    QModelIndexList indexList = treeViewResults->selectionModel()->selectedIndexes();
    if (indexList.empty())
        return;
    // Forward the work to another slot
    on_treeViewResults_doubleClicked(indexList.front());
}


void TrainingWidget::on_treeViewResults_doubleClicked(const QModelIndex &iIndex)
{
    if (!iIndex.isValid())
        return;
    m_game->removeTestPlay();
    // Use the hidden column to get the result number
    const QModelIndex &index = m_model->index(iIndex.row(), 5);
    m_game->playResult(m_model->data(index).toUInt());
    emit gameUpdated();
}


QSize TrainingWidget::sizeHint() const
{
    return QSize(160, 300);
}



RackValidator::RackValidator(QObject *parent)
    : QValidator(parent)
{
}


QValidator::State RackValidator::validate(QString &input, int &) const
{
    // This should never happen, since the control should be disabled in
    // such a case, but checking doesn't hurt...
    if (m_bag == NULL)
        return Invalid;

    input = input.toUpper();

    if (!m_bag->getDic().validateLetters(qtw(input)))
        return Invalid;

    // The letters must be in the bag
    for (int i = 0; i < input.size(); ++i)
    {
        if ((unsigned int)input.count(input[i], Qt::CaseInsensitive) >
            m_bag->in(qtw(input.mid(i, 1))[0]))
        {
            return Invalid;
        }
    }
    return Acceptable;
}


