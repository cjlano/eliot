/*****************************************************************************
 * Eliot
 * Copyright (C) 2010-2011 Olivier Teulière
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

#include <QtGui/QWidget>
#include <QtGui/QMessageBox>

#include "qtcommon.h"
#include <iostream>

using namespace std;


wstring wfq(const QString &q)
{
#ifdef QT_NO_STL
    wchar_t *array = new wchar_t[q.size()];
    int size = q.toWCharArray(array);
    wstring ws(array, size);
    delete[] array;
    return ws;
#else
    return q.toStdWString();
#endif
}


QString qfw(const wstring &wstr)
{
#ifdef QT_NO_STL
    return QString::fromWCharArray(wstr.c_str());
#else
    return QString::fromStdWString(wstr);
#endif
}

static void logFailedTest(const string &testName)
{
    cerr << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
    cerr << "@@@@@@@ Test " + testName + " failed! @@@@@@@" << endl;
    cerr << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;
}


void QtCommon::CheckConversions()
{
    string s = "abcdéöùßĿ";

    // Check identities
    if (s != lfw(wfl(s)))
        logFailedTest("1");
    if (s != lfu(ufl(s)))
        logFailedTest("2");
    if (s != lfq(qfl(s)))
        logFailedTest("3");

    wstring w = wfl(s);
    if (w != wfl(lfw(w)))
        logFailedTest("4");
    if (w != wfu(ufw(w)))
        logFailedTest("5");
    if (w != wfq(qfw(w)))
        logFailedTest("6");

    QString q = qfl(s);
    if (q != qfl(lfq(q)))
        logFailedTest("7");
    if (q != qfu(ufq(q)))
        logFailedTest("8");
    if (q != qfw(wfq(q)))
        logFailedTest("9");

    // Check some cycles
    if (s != lfu(ufw(wfl(s))))
        logFailedTest("10");
    if (s != lfw(wfu(ufl(s))))
        logFailedTest("11");
    if (s != lfq(qfw(wfl(s))))
        logFailedTest("12");
    if (s != lfw(wfq(qfl(s))))
        logFailedTest("13");
    if (s != lfu(ufw(wfq(qfl(s)))))
        logFailedTest("14");
    if (s != lfq(qfw(wfu(ufl(s)))))
        logFailedTest("15");
    if (s != lfu(ufq(qfw(wfl(s)))))
        logFailedTest("16");
    if (s != lfw(wfq(qfu(ufl(s)))))
        logFailedTest("17");
}


void QtCommon::DestroyObject(QWidget *ioObjectToDestroy,
                             QObject *iSource)
{
    if (ioObjectToDestroy == NULL)
        return;
    ioObjectToDestroy->hide();
    ioObjectToDestroy->disconnect();
    if (iSource != NULL)
        iSource->disconnect(ioObjectToDestroy);
    ioObjectToDestroy->deleteLater();
}


bool QtCommon::requestConfirmation(QString iMsg, QString iQuestion, QWidget *iParent)
{
    QMessageBox confirmationBox(QMessageBox::Question, _q("Eliot"), iMsg,
                                QMessageBox::Yes | QMessageBox::No, iParent);
    if (iQuestion != "")
        confirmationBox.setInformativeText(iQuestion);
    else
        confirmationBox.setInformativeText(_q("Do you want to continue?"));
    confirmationBox.setDefaultButton(QMessageBox::Yes);
    confirmationBox.setEscapeButton(QMessageBox::No);
    int res = confirmationBox.exec();
    return res == QMessageBox::Yes;
}

