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

#include <QtGui/QTableView>
#include <QtGui/QHeaderView>
#include <QtGui/QStandardItemModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QAction>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtCore/QLocale>

#include "math.h" // For floor()

#include "stats_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "history.h"
#include "turn_data.h"
#include "game_params.h"
#include "settings.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, StatsWidget);


const QColor StatsWidget::WarningBrush(220, 220, 0);
const QColor StatsWidget::PenaltyBrush(220, 120, 0);
const QColor StatsWidget::SoloBrush(0, 200, 0);
const QColor StatsWidget::PassBrush(210, 210, 210);
const QColor StatsWidget::InvalidBrush(255, 0, 0);


/**
 * Flipped version of a model (using the decorator pattern)
 * This implementation does not support tree-models.
 */
class FlippedModel : public QAbstractItemModel
{
    public:
        FlippedModel(QAbstractItemModel *iRefModel) : m_refModel(iRefModel) {}

        virtual int columnCount(const QModelIndex &) const
        {
            // Ignore the parent
            return m_refModel->rowCount();
        }

        virtual int rowCount(const QModelIndex &) const
        {
            // Ignore the parent
            return m_refModel->columnCount();
        }

        virtual QModelIndex index(int row, int column, const QModelIndex &) const
        {
            return createIndex(row, column, (void*)NULL);
        }

        virtual QModelIndex parent(const QModelIndex &) const
        {
            // Ignore the given index
            return QModelIndex();
        }

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        {
            return m_refModel->data(m_refModel->index(index.column(), index.row()), role);
        }

        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        {
            Qt::Orientation newOrient =
                orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal;
            return m_refModel->headerData(section, newOrient, role);
        }

    private:
        /// Wrapped model
        QAbstractItemModel *m_refModel;
};


StatsWidget::StatsWidget(QWidget *parent, const PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_autoResizeColumns(true)
{
    // Layout
    setLayout(new QVBoxLayout);

    // Create the table
    m_table = new QTableView(this);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->verticalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setMinimumSectionSize(15);
    m_table->verticalHeader()->setMinimumSectionSize(15);
    //m_table->setSortingEnabled(true);
    layout()->addWidget(m_table);

    m_model = new QStandardItemModel();
    m_flippedModel = new FlippedModel(m_model);

    m_table->setModel(m_model);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add a context menu to the tree header
    QAction *lockSizesAction = new QAction(_q("Lock columns sizes"), this);
    lockSizesAction->setCheckable(true);
    lockSizesAction->setStatusTip(_q("Disable auto-resizing of the columns"));
    m_table->horizontalHeader()->addAction(lockSizesAction);
    m_table->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    QObject::connect(lockSizesAction, SIGNAL(toggled(bool)),
                     this, SLOT(lockSizesChanged(bool)));

    m_table->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Add a context menu option to flip the table
    QAction *flipAction = new QAction(_q("Flip table"), this);
    flipAction->setStatusTip(_q("Flip the table so that rows and columns are exchanged.\n"
                                "This allows sorting the players by ranking, for example."));
    m_table->addAction(flipAction);
    QObject::connect(flipAction, SIGNAL(triggered()),
                     this, SLOT(flipTable()));

    QAction *printPreviewAction = new QAction(_q("Print preview..."), this);
    printPreviewAction->setStatusTip(_q("Print the table."));
    m_table->addAction(printPreviewAction);
    QObject::connect(printPreviewAction, SIGNAL(triggered()),
                     this, SLOT(onPrintPreview()));

    QAction *printAction = new QAction(_q("Print..."), this);
    printAction->setStatusTip(_q("Print the table."));
    m_table->addAction(printAction);
    QObject::connect(printAction, SIGNAL(triggered()),
                     this, SLOT(onPrint()));

    refresh();
}


void StatsWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void StatsWidget::refresh()
{
    m_model->removeRows(0, m_model->rowCount());

    unsigned histSize = m_game == NULL ? 0 : m_game->getHistory().getSize();
    unsigned nbPlayers = m_game == NULL ? 0 : m_game->getNbPlayers();

    setModelSize(nbPlayers + 1, histSize + 10);

    // Some fields are displayed only in some cases
    const bool isTraining = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kTRAINING;
    const bool isArbit = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kARBITRATION;
    const bool isFreeGame = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kFREEGAME;
    const bool isTopping = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kTOPPING;
    const bool canHaveSolos = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kDUPLICATE &&
        Settings::Instance().getInt("duplicate.solo-players") <= (int)m_game->getNbPlayers();

    // Define columns (or rows, depending on the orientation)
    int col = 0;
    setSectionHidden(col, !isArbit);
    setModelHeader(col++, _q("Table"), false);

    for (unsigned i = 1; i <= histSize; ++i)
    {
        QString turnString = QString("#%1").arg(i);
        // Show the move played for this turn, if it is valid.
        // We don't show it when the table is flipped, because it looks ugly.
        const Move &move = m_game->getHistory().getTurn(i - 1).getMove();
        if (move.isValid() && !isFlipped())
        {
            turnString += QString(" (%1 - %2)")
                .arg(qfw(move.getRound().getWord()))
                .arg(qfw(move.getRound().getCoord().toString()));
        }
        setModelHeader(col++, turnString, false);
    }

    setSectionHidden(col, !isArbit && !canHaveSolos && !isFreeGame);
    setModelHeader(col++, _q("Sub-total"), false);
    setSectionHidden(col, !isFreeGame);
    setModelHeader(col++, _q("End game points"), false);
    setSectionHidden(col, !isArbit && !canHaveSolos);
    setModelHeader(col++, _q("Solo points"), false);
    setSectionHidden(col, !isArbit && !isTopping);
    setModelHeader(col++, _q("Penalties"), false);
    setSectionHidden(col, !isArbit);
    setModelHeader(col++, _q("Warnings"), false);

    setModelHeader(col++, _q("Total"), false);
    setSectionHidden(col, isTopping);
    setModelHeader(col++, _q("Diff"), false);
    setSectionHidden(col, isTopping);
    setModelHeader(col++, _q("Game %"), false);
    setSectionHidden(col, isTraining || isTopping);
    setModelHeader(col++, _q("Ranking"), false);

    // Define the header for the Game pseudo-player
    setModelHeader(0, _q("Game"), true);

    if (m_game == NULL)
        return;

    QLocale locale;
    const History &gHistory = m_game->getHistory();

    // Game data
    int gameTotal = 0;
    {
        const int row = 0;
        int col = 0;
        // Skip the table number
        ++col;
        int score = 0;
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            setModelTurnData(getIndex(row, col++), gHistory.getTurn(j), gHistory.getTurn(j));
            score += gHistory.getTurn(j).getMove().getScore();
        }
        setModelText(getIndex(row, col++), score, true);
        // Skip the events columns
        col += 4;
        setModelText(getIndex(row, col++), score, true);
        // Skip the diff column
        ++col;
        setModelText(getIndex(row, col++), locale.toString((double)100, 'f', 1) + "%", true);
        // Skip the ranking column
        ++col;

        gameTotal = score;
    }

    // Players data
    for (unsigned i = 0; i < nbPlayers; ++i)
    {
        const Player &player = m_game->getPlayer(i);
        int col = 0;
        setModelHeader(i + 1, qfw(player.getName()), true);

        // Table number
        setModelText(getIndex(i + 1, col++), player.getTableNb());

        // Normal turns
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            const History &pHistory = player.getHistory();
            setModelTurnData(getIndex(i + 1, col++),
                             pHistory.getTurn(j), gHistory.getTurn(j));
        }

        // Sub-total
        const int subTotal = player.getMovePoints();
        setModelText(getIndex(i + 1, col++), subTotal, subTotal >= gameTotal);

        // Events columns
        for (int j = 0; j <= 3; ++j)
        {
            setModelEventData(getIndex(i + 1, col++), j, player);
        }

        // Final score
        const int totalScore = player.getTotalScore();
        setModelText(getIndex(i + 1, col++), totalScore, totalScore >= gameTotal);

        // Diff with game total
        setModelText(getIndex(i + 1, col++), totalScore - gameTotal);
        // Global score percentage
        setModelText(getIndex(i + 1, col++),
                     locale.toString(100. * totalScore / gameTotal, 'f', 1) + "%",
                     totalScore >= gameTotal);

        // Ranking
        // FIXME: quadratic complexity, we can probably do better
        int rank = 1;
        for (unsigned j = 0; j < nbPlayers; ++j)
        {
            if (i == j)
                continue;
            if (player.getTotalScore() < m_game->getPlayer(j).getTotalScore())
                ++rank;
        }
        setModelText(getIndex(i + 1, col++), rank, rank == 1);
    }

    // Resize
    m_table->resizeRowsToContents();
    if (m_autoResizeColumns)
        m_table->resizeColumnsToContents();
}


QModelIndex StatsWidget::getIndex(int row, int col) const
{
    return m_model->index(col, row);
}


void StatsWidget::setSectionHidden(int index, bool iHide)
{
    if (isFlipped())
        m_table->setColumnHidden(index, iHide);
    else
        m_table->setRowHidden(index, iHide);
}


void StatsWidget::setModelSize(int rowCount, int colCount)
{
    m_model->setRowCount(colCount);
    m_model->setColumnCount(rowCount);
}


void StatsWidget::setModelHeader(int index, const QString &iText, bool iPlayerNames)
{
    Qt::Orientation orientation;
    if (iPlayerNames)
        orientation = Qt::Horizontal;
    else
        orientation = Qt::Vertical;
    m_model->setHeaderData(index, orientation, iText);
    m_model->setHeaderData(index, orientation, Qt::AlignCenter, Qt::TextAlignmentRole);
}


void StatsWidget::setModelText(const QModelIndex &iIndex,
                               const QVariant &iData, bool useBoldFont)
{
    m_model->setData(iIndex, iData);
    m_model->setData(iIndex, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (useBoldFont)
    {
        QFont boldFont = font();
        boldFont.setBold(true);
        m_model->setData(iIndex, boldFont, Qt::FontRole);
    }
}


void StatsWidget::setModelTurnData(const QModelIndex &iIndex,
                                   const TurnData &iTurn, const TurnData &iGameTurn)
{
    // Set the text (score for the turn)
    int score = iTurn.getMove().getScore();
    if (score != 0)
    {
        bool shouldUseBold = score >= iGameTurn.getMove().getScore()
            && (m_game == 0 || m_game->getMode() != PublicGame::kTOPPING);
        setModelText(iIndex, QVariant(score), shouldUseBold);
    }

    // Set the background color
    if (iTurn.getSoloPoints() != 0)
        m_model->setData(iIndex, SoloBrush, Qt::BackgroundRole);
    else if (iTurn.getPenaltyPoints() != 0)
        m_model->setData(iIndex, PenaltyBrush, Qt::BackgroundRole);
    else if (iTurn.getWarningsNb() != 0)
        m_model->setData(iIndex, WarningBrush, Qt::BackgroundRole);
    else if (iTurn.getMove().isNull())
        m_model->setData(iIndex, PassBrush, Qt::BackgroundRole);

    // Set the foreground color
    if (iTurn.getMove().isInvalid())
        m_model->setData(iIndex, InvalidBrush, Qt::ForegroundRole);

    // Set the tooltip
    const QString &tooltip = getTooltip(iTurn, iGameTurn);
    m_model->setData(iIndex, tooltip, Qt::ToolTipRole);
}


void StatsWidget::setModelEventData(const QModelIndex &iIndex,
                                    int iEvent, const Player &iPlayer)
{
    QVariant text;
    if (iEvent == 0 && iPlayer.getEndGamePoints() != 0)
        text = iPlayer.getEndGamePoints();
    else if (iEvent == 1 && iPlayer.getSoloPoints() != 0)
        text = iPlayer.getSoloPoints();
    else if (iEvent == 2 && iPlayer.getPenaltyPoints() != 0)
        text = iPlayer.getPenaltyPoints();
    else if (iEvent == 3 && iPlayer.getWarningsNb() != 0)
        text = iPlayer.getWarningsNb();
    setModelText(iIndex, text);
}


QString StatsWidget::getTooltip(const TurnData &iTurn, const TurnData &iGameTurn) const
{
    QString tooltip = _q("Rack: %1").arg(qfw(iTurn.getPlayedRack().toString()));
    const Move &move = iTurn.getMove();
    if (move.isValid())
    {
        tooltip += "\n" + _q("Word: %1").arg(qfw(move.getRound().getWord()));
        tooltip += "\n" + _q("Ref: %1").arg(qfw(move.getRound().getCoord().toString()));
    }
    else if (move.isInvalid())
    {
        tooltip += "\n" + _q("Invalid move (%1 - %2)")
            .arg(qfw(move.getBadWord()))
            .arg(qfw(move.getBadCoord()));
    }
    else if (move.isChangeLetters())
    {
        tooltip += "\n" + _q("Changed letters: %1").arg(qfw(move.getChangedLetters()));
    }
    else if (move.isPass())
    {
        tooltip += "\n" + _q("Passed turn");
    }
    else
    {
        tooltip += "\n" + _q("No move");
    }

    // Points
    int score = move.getScore();
    int gameScore = iGameTurn.getMove().getScore();
    if (move.isNull())
        tooltip += _q("Points: %1").arg(score);
    else
    {
        QString scoreString = _q("Points: %1 (%2)").arg(score);
        if (score == gameScore)
            tooltip += "\n" + scoreString.arg(_q("max"));
        else
            tooltip += "\n" + scoreString.arg(score - gameScore);
    }

    if (iTurn.getSoloPoints())
    {
        tooltip += "\n" + _q("Solo: %1").arg(iTurn.getSoloPoints());
    }
    if (iTurn.getWarningsNb())
    {
        tooltip += "\n" + _q("Warnings: %1").arg(iTurn.getWarningsNb());
    }
    if (iTurn.getPenaltyPoints())
    {
        tooltip += "\n" + _q("Penalties: %1").arg(iTurn.getPenaltyPoints());
    }
    return tooltip;
}


void StatsWidget::lockSizesChanged(bool checked)
{
    m_autoResizeColumns = !checked;
}


bool StatsWidget::isFlipped() const
{
    return m_table->model() != m_model;
}


void StatsWidget::flipTable()
{
    bool flipped = isFlipped();
    m_table->setSortingEnabled(!flipped);
    if (flipped)
        m_table->setModel(m_model);
    else
    {
        QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
        proxy->setDynamicSortFilter(true);
        proxy->setSourceModel(m_flippedModel);
        m_table->setModel(proxy);
        // Sort by ranking (last column)
        const int col = m_flippedModel->columnCount() - 1;
        m_table->sortByColumn(col);
        m_table->horizontalHeader()->setSortIndicator(col, Qt::AscendingOrder);
    }
    refresh();
}


void StatsWidget::onPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted)
        return;

    LOG_INFO("Printing statistics");
    print(&printer);
}


void StatsWidget::onPrintPreview()
{
    QPrintPreviewDialog previewDialog;
    QObject::connect(&previewDialog, SIGNAL(paintRequested(QPrinter *)),
                     this, SLOT(print(QPrinter*)));
    previewDialog.exec();
}


void StatsWidget::print(QPrinter *printer)
{
    // Dimensions of the page
    const int pageWidth = printer->pageRect().width();
    const int pageHeight = printer->pageRect().height();
    // Dimensions of the table (without the useless space after
    // the last row and last column)
    const int width = 2 + m_table->verticalHeader()->width() + m_table->horizontalHeader()->length();
    const int height = 2 + m_table->horizontalHeader()->height() + m_table->verticalHeader()->length();

    QPainter painter(printer);
    // Try to use as much space as possible on the page, by turning it if needed
    bool rotated = false;
    if ((pageWidth > pageHeight && height > width) ||
        (pageWidth < pageHeight && height < width))
    {
        painter.translate(QPoint(pageWidth, 0));
        painter.rotate(90);
        rotated = true;
    }

    const double scaleX = (rotated ? pageHeight : pageWidth) / (double) width;
    const double scaleY = (rotated ? pageWidth : pageHeight) / (double) height;
    double scale = std::min(scaleX, scaleY);
    // Only scale if there is too much to print on the page
    if (scale > 1)
        scale = floor(scale);
    painter.scale(scale, scale);

    // Little trick, needed to have the full table printed.
    // If we don't do that, only the viewport is printed, which is not what we want
    // when some contents is not displayed (i.e. when the scrollbars are shown).
    m_table->resize(width, height);
    // Disable scrollbars, because for some reason they get printed if the viewport
    // was too small to show everything
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Actually paint the widget
    m_table->render(&painter, QPoint(0, 0), QRect(0, 0, width, height));
    // Restore scrollbars
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


