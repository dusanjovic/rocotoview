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

#include "aboutdialog.h"

#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->appname->setText(QApplication::applicationName() + " " +
                         QApplication::applicationVersion());
    ui->appinfo->setText(QString("Based on Qt %1 (%2 bit)")
                         .arg(QLatin1String(QT_VERSION_STR),
                              QString::number(QSysInfo::WordSize)));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
