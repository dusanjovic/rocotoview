//
//  rocotoview - Rocoto workflow viewer
//
//  Copyright (C) 2014-2016 by Dusan Jovic
//
//  This file is part of rocotoview.
//
//  rocotoview is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  rocotoview is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with rocotoview.  If not, see <http://www.gnu.org/licenses/>.
//

#include "taildialog.h"

#include "ui_taildialog.h"

TailDialog::TailDialog(const QString &filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TailDialog),
    procTail(this)
{
    ui->setupUi(this);

    QStringList arglist;
    arglist << "-n 1000" << "-f" << filename;
    procTail.start("tail", arglist, QIODevice::ReadWrite);
    connect(&procTail, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readFromStdoutForTail()) );
}

TailDialog::~TailDialog()
{
    if( procTail.state() == QProcess::Running ) {
        procTail.terminate();
        procTail.waitForFinished();
    }
    delete ui;
}

void TailDialog::readFromStdoutForTail()
{
    while ( procTail.canReadLine() ) {
        QByteArray a = procTail.readLine();
        if (a.at(a.size()-1) == '\n')
            a[a.size()-1] = ' ';
        ui->plainTextEdit->appendPlainText(a);
    }
}
