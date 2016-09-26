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

#ifndef TAILDIALOG_H
#define TAILDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class TailDialog;
}

class TailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TailDialog(const QString &filename, QWidget *parent = 0);
    ~TailDialog();

private Q_SLOTS:
    void readFromStdoutForTail();

private:
    Ui::TailDialog *ui;
    QProcess procTail;
};

#endif // TAILDIALOG_H
