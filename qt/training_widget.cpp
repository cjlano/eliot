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

#include <QtGui/QStandardItemModel>
#include <QtGui/QValidator>

#include "training_widget.h"
#include "qtcommon.h"
#include "play_word_mediator.h"

#include "dic.h"
#include "bag.h"
#include "public_game.h"
#include "game_exception.h"
#include "player.h"
#include "results.h"

using namespace std;


/// Validator used for the rack line edit
class RackValidator: public QValidator
{
public:
    explicit RackValidator(QObject *parent, const Bag *iBag);
    virtual State validate(QString &input, int &pos) const;

private:
    const Bag *m_bag;
};


TrainingWidget::TrainingWidget(QWidget *parent, CoordModel &iCoordModel, PublicGame *iGame)
    : QWidget(parent), m_game(iGame)
{
    setupUi(this);

    redPalette = lineEditRack->palette();
    redPalette.setColor(QPalette::Text, Qt::red);
    blackPalette = lineEditRack->palette();
    blackPalette.setColor(QPalette::Text, Qt::black);

    // Use the mediator
    m_mediator = new PlayWordMediator(this, *lineEditPlay, *lineEditCoords,
                                      *pushButtonPlay, iCoordModel, m_game);
    QObject::connect(m_mediator, SIGNAL(gameUpdated()),
                     this, SIGNAL(gameUpdated()));
    QObject::connect(m_mediator, SIGNAL(notifyProblem(QString)),
                     this, SIGNAL(notifyProblem(QString)));

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

    if (m_game)
        lineEditRack->setValidator(new RackValidator(this, &m_game->getBag()));

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
        m_game->trainingGetResults().size() == (unsigned int)m_model->rowCount())
    {
        return;
    }

    m_model->removeRows(0, m_model->rowCount());

    if (m_game == NULL)
        return;

    const Results &results = m_game->trainingGetResults();
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
    // Enable the "Play selected" button iff at least one line
    // in the tree view is selected
    pushButtonPlaySelected->setEnabled(!iSelected.indexes().empty());
}


void TrainingWidget::showPreview(const QItemSelection &iSelected,
                                 const QItemSelection &)
{
    m_game->trainingRemoveTestPlay();
    if (!iSelected.indexes().empty())
    {
        // Use the hidden column to get the result number
        const QModelIndex &index =
            m_model->index(iSelected.indexes().first().row(), 5);
        m_game->trainingTestPlay(m_model->data(index).toUInt());
        emit gameUpdated();
    }
}


void TrainingWidget::on_lineEditRack_textEdited(const QString &iText)
{
    // FIXME: first parameter is hardcoded
    m_game->trainingRemoveTestPlay();
    if (!lineEditRack->hasAcceptableInput())
    {
        lineEditRack->setPalette(redPalette);
        return;
    }
    try
    {
        lineEditRack->setPalette(blackPalette);
        const wstring &input = m_game->getDic().convertFromInput(qtw(iText));
        m_game->trainingSetRackManual(false, input);
        pushButtonSearch->setEnabled(m_model->rowCount() == 0 &&
                                     lineEditRack->text() != "");
        emit gameUpdated();
    }
    catch (std::exception &e)
    {
        lineEditRack->setPalette(redPalette);
        emit notifyProblem(_q("Warning: Cannot set the rack to '%1'").arg(iText));
    }
}


void TrainingWidget::on_pushButtonRack_clicked()
{
    m_game->trainingRemoveTestPlay();
    try
    {
        // FIXME: first parameter is hardcoded
        m_game->trainingSetRackRandom(true, PublicGame::kRACK_ALL);
        emit gameUpdated();
    }
    catch (std::exception &e)
    {
        emit notifyProblem(_q(e.what()));
    }
}


void TrainingWidget::on_pushButtonComplement_clicked()
{
    m_game->trainingRemoveTestPlay();
    try
    {
        // FIXME: first parameter is hardcoded
        m_game->trainingSetRackRandom(true, PublicGame::kRACK_NEW);
        emit gameUpdated();
    }
    catch (std::exception &e)
    {
        emit notifyProblem(_q(e.what()));
    }
}


void TrainingWidget::on_pushButtonSearch_clicked()
{
    m_game->trainingRemoveTestPlay();
    emit notifyInfo(_q("Searching with rack '%1'...").arg(lineEditRack->text()));
    m_game->trainingSearch();
    emit notifyInfo(_q("Search done"));
    emit gameUpdated();
}


void TrainingWidget::on_pushButtonPlaySelected_clicked()
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
    m_game->trainingRemoveTestPlay();
    // Use the hidden column to get the result number
    const QModelIndex &index = m_model->index(iIndex.row(), 5);
    m_game->trainingPlayResult(m_model->data(index).toUInt());
    emit gameUpdated();
}


QSize TrainingWidget::sizeHint() const
{
    return QSize(160, 300);
}



RackValidator::RackValidator(QObject *parent, const Bag *iBag)
    : QValidator(parent), m_bag(iBag)
{
}


QValidator::State RackValidator::validate(QString &input, int &) const
{
    // This should never happen, since the control should be disabled in
    // such a case, but checking doesn't hurt...
    if (m_bag == NULL)
        return Invalid;

    input = input.toUpper();

    const Dictionary &dic = m_bag->getDic();

    // The string is invalid if it contains invalid input characters
    const wistring &winput = qtw(input);
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
            m_bag->in(intInput[i]))
        {
            return Invalid;
        }
    }
    return Acceptable;
}


