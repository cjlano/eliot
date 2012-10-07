/*****************************************************************************
 * Eliot
 * Copyright (C) 2010-2012 Olivier Teulière
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
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>

#include "training_widget.h"
#include "qtcommon.h"
#include "play_word_mediator.h"
#include "custom_popup.h"
#include "validator_factory.h"

#include "dic.h"
#include "bag.h"
#include "public_game.h"
#include "game_exception.h"
#include "player.h"
#include "results.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, TrainingWidget);


static const int HIDDEN_COLUMN = 6;


TrainingWidget::TrainingWidget(QWidget *parent, CoordModel &iCoordModel, PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_autoResizeColumns(true)
{
    setupUi(this);
    treeViewResults->setAlternatingRowColors(true);

    blackPalette = lineEditRack->palette();
    redPalette = lineEditRack->palette();
    redPalette.setColor(QPalette::Text, Qt::red);

    // Use the mediator
    m_mediator = new PlayWordMediator(this, *lineEditPlay, *lineEditCoords,
                                      *lineEditPoints, *pushButtonPlay,
                                      iCoordModel, m_game);
    QObject::connect(m_mediator, SIGNAL(gameUpdated()),
                     this, SIGNAL(gameUpdated()));
    QObject::connect(m_mediator, SIGNAL(notifyProblem(QString)),
                     this, SIGNAL(notifyProblem(QString)));

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    treeViewResults->setModel(m_model);
    m_model->setColumnCount(7);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, "*", Qt::DisplayRole);
    m_model->setHeaderData(4, Qt::Horizontal, "", Qt::DisplayRole);
    m_model->setHeaderData(5, Qt::Horizontal, "", Qt::DisplayRole);
    // Hidden column, used to store internal data
    m_model->setHeaderData(HIDDEN_COLUMN, Qt::Horizontal, "", Qt::DisplayRole);
    treeViewResults->setColumnHidden(HIDDEN_COLUMN, true);

    QObject::connect(lineEditRack, SIGNAL(returnPressed()),
                     this, SLOT(search()));
    QObject::connect(lineEditRack, SIGNAL(textEdited(const QString &)),
                     this, SLOT(onRackEdited(const QString &)));

    QObject::connect(pushButtonRack, SIGNAL(clicked()),
                     this, SLOT(setNewRack()));
    QObject::connect(pushButtonComplement, SIGNAL(clicked()),
                     this, SLOT(completeRack()));
    QObject::connect(pushButtonSearch, SIGNAL(clicked()),
                     this, SLOT(search()));
    QObject::connect(pushButtonPlaySelected, SIGNAL(clicked()),
                     this, SLOT(playSelectedWord()));
    QObject::connect(treeViewResults, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(playSelectedWord()));

    // Add a context menu to the tree header
    QAction *lockSizesAction = new QAction(_q("Lock columns sizes"), this);
    lockSizesAction->setCheckable(true);
    lockSizesAction->setStatusTip(_q("Disable auto-resizing of the columns"));
    treeViewResults->header()->addAction(lockSizesAction);
    treeViewResults->header()->setContextMenuPolicy(Qt::ActionsContextMenu);
    QObject::connect(lockSizesAction, SIGNAL(toggled(bool)),
                     this, SLOT(lockSizesChanged(bool)));

    // Add another context menu for the results
    m_customPopup = new CustomPopup(treeViewResults);
    QObject::connect(m_customPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateMenu(QMenu&, const QPoint&)));
    QObject::connect(m_customPopup, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));

    // Allow very thin columns
    treeViewResults->header()->setMinimumSectionSize(1);

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

    if (m_game)
    {
        QValidator * val = ValidatorFactory::newRackValidator(this, m_game->getBag());
        lineEditRack->setValidator(val);
    }

    // Notify that the rack changed
    QObject::connect(lineEditRack, SIGNAL(textChanged(const QString&)),
                     this, SIGNAL(rackUpdated(const QString&)));

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
        lineEditPlay->clear();
        lineEditCoords->clear();
        lineEditRack->setEnabled(true);
        pushButtonRack->setEnabled(true);
        pushButtonComplement->setEnabled(true);
        pushButtonSearch->setEnabled(m_model->rowCount() == 0 &&
                                     lineEditRack->text() != "");
        // Do not allow entering a move when displaying an old turn
        setEnabled(m_game->isLastTurn());
    }
}


void TrainingWidget::updateModel()
{
    // Consider that there is nothing to do if the number of lines is correct
    // This avoids problems when the game is updated for a test play
    if (m_game != NULL &&
        m_game->trainingGetResults().size() == static_cast<unsigned int>(m_model->rowCount()))
    {
        return;
    }

    // Clear the results
    m_model->removeRows(0, m_model->rowCount());

    // Force the sort column
    treeViewResults->sortByColumn(2);

    if (m_game == NULL)
        return;

    const Results &results = m_game->trainingGetResults();
    // Find the highest score
    int bestScore = -1;
    if (results.size() != 0)
        bestScore = results.get(0).getPoints();
    for (unsigned int i = 0; i < results.size(); ++i)
    {
        const Round &r = results.get(i);
        int rowNum = m_model->rowCount();
        m_model->insertRow(rowNum);
        m_model->setData(m_model->index(rowNum, 0), qfw(r.getWord()));
        m_model->setData(m_model->index(rowNum, 1),
                         qfw(r.getCoord().toString()));
        m_model->setData(m_model->index(rowNum, 2), r.getPoints());
        m_model->setData(m_model->index(rowNum, 3),
                         r.getBonus() ? "*": "");
        if (r.getPoints() == bestScore)
        {
            // Color the line in red if this is the top score
            const QBrush redBrush(Qt::red);
            for (int j = 0; j < HIDDEN_COLUMN; ++j)
            {
                m_model->setData(m_model->index(rowNum, j),
                                 redBrush, Qt::ForegroundRole);
            }
        }
        else
        {
            // Otherwise indicate the difference with the best score
            m_model->setData(m_model->index(rowNum, 4),
                             r.getPoints() - bestScore);
        }

        // Hidden data, used to handle proper sorting in the tree view
        m_model->setData(m_model->index(rowNum, HIDDEN_COLUMN), i);
    }

    // Clear the status bar when there is no search result
    if (m_model->rowCount() == 0)
        emit notifyInfo("");
    else
    {
        // Otherwise, select the first line
        treeViewResults->selectionModel()->select(m_model->index(0, 0),
                                                  QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    }

    if (m_autoResizeColumns)
    {
        treeViewResults->resizeColumnToContents(0);
        treeViewResults->resizeColumnToContents(1);
        treeViewResults->resizeColumnToContents(2);
        treeViewResults->resizeColumnToContents(3);
        treeViewResults->resizeColumnToContents(4);
    }
}


void TrainingWidget::enablePlayButton(const QItemSelection &iSelected,
                                      const QItemSelection &)
{
    // Enable the "Play selected" button iff at least one line
    // in the tree view is selected
    pushButtonPlaySelected->setEnabled(!iSelected.indexes().empty());
}


void TrainingWidget::showPreview(const QItemSelection &iSelected,
                                 const QItemSelection &)
{
    m_game->removeTestRound();
    if (!iSelected.indexes().empty())
    {
        // Use the hidden column to get the result number
        const QModelIndex &index =
            m_model->index(iSelected.indexes().first().row(), HIDDEN_COLUMN);
        unsigned int resNb = m_model->data(index).toUInt();

        const Results &results = m_game->trainingGetResults();
        ASSERT(resNb < results.size(), "Wrong result number");
        m_game->setTestRound(results.get(resNb));
        emit gameUpdated();
    }
}


void TrainingWidget::populateMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewResults->indexAt(iPoint);
    if (!index.isValid())
        return;

    // Find the selected word
    const QModelIndex &wordIndex = m_model->index(index.row(), 0);
    QString selectedWord = m_model->data(wordIndex).toString();

    iMenu.addAction(m_customPopup->getShowDefinitionEntry(selectedWord));
}


void TrainingWidget::lockSizesChanged(bool checked)
{
    m_autoResizeColumns = !checked;
}


void TrainingWidget::onRackEdited(const QString &iText)
{
    m_game->removeTestRound();
    if (!lineEditRack->hasAcceptableInput())
    {
        lineEditRack->setPalette(redPalette);
        return;
    }
    try
    {
        lineEditRack->setPalette(blackPalette);
        const wstring &input = m_game->getDic().convertFromInput(wfq(iText));
        m_game->trainingSetRackManual(false, input);
        pushButtonSearch->setEnabled(m_model->rowCount() == 0 &&
                                     lineEditRack->text() != "");
        emit gameUpdated();
    }
    catch (std::exception &e)
    {
        lineEditRack->setPalette(redPalette);
        emit notifyProblem(_q("Warning: Cannot set the rack to '%1'\n%2").arg(iText).arg(e.what()));
    }
}


void TrainingWidget::setNewRack()
{
    helperSetRack(true);
}


void TrainingWidget::completeRack()
{
    helperSetRack(false);
}


void TrainingWidget::helperSetRack(bool iAll)
{
    m_game->removeTestRound();
    try
    {
        // FIXME: first parameter is hardcoded
        m_game->trainingSetRackRandom(true,
                iAll ? PublicGame::kRACK_NEW : PublicGame::kRACK_NEW);
        emit gameUpdated();
        lineEditRack->setFocus();
    }
    catch (std::exception &e)
    {
        emit notifyProblem(_q(e.what()));
    }
}


void TrainingWidget::search()
{
    m_game->removeTestRound();
    emit notifyInfo(_q("Searching with rack '%1'...").arg(lineEditRack->text()));
    m_game->trainingSearch();
    emit notifyInfo(_q("Search done"));
    emit gameUpdated();
    treeViewResults->setFocus();
}


void TrainingWidget::playSelectedWord()
{
    QModelIndexList indexList = treeViewResults->selectionModel()->selectedIndexes();
    if (indexList.empty())
        return;

    const QModelIndex &selIndex = indexList.front();
    if (!selIndex.isValid())
        return;

    m_game->removeTestRound();
    // Use the hidden column to get the result number
    const QModelIndex &index = m_model->index(selIndex.row(), HIDDEN_COLUMN);
    m_game->trainingPlayResult(m_model->data(index).toUInt());
    emit gameUpdated();
    lineEditRack->setFocus();
}


QSize TrainingWidget::sizeHint() const
{
    return QSize(160, 300);
}

