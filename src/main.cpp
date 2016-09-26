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

#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

void printHelp()
{
    qDebug() << "Usage:  rocotoview  [-h] [-v] -w workflow_document -d database_file";
    qDebug() << "";
    qDebug() << "Options:";
    qDebug() << "  -h, --help             Displays this help.";
    qDebug() << "  -v, --version          Displays version information.";
    qDebug() << "  -w, --workflow <file>  Rocoto workflow definition file (xml).";
    qDebug() << "  -d, --database <file>  Rocoto database file.";
    exit(0);
}

void printVersion()
{
    qDebug() << QApplication::applicationName().toStdString().c_str()
             << QApplication::applicationVersion().toStdString().c_str();
    exit(0);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("dusanjovic");
    QApplication::setApplicationName("rocotoview");
    QApplication::setApplicationVersion("1.0");

    QString workflowFile;
    QString databaseFile;

    QStringList arguments = app.arguments();

    if (arguments.size() > 1) {
        int i=1;
        while (i < arguments.size()) {
            if (arguments[i] == "-h" || arguments[i] == "--help") {
                printHelp();
            } else if (arguments[i] == "-v" || arguments[i] == "--version") {
                printVersion();
            } else if (arguments[i] == "-w") {
                if (i+1 < arguments.size()) {
                    workflowFile = arguments[i+1];
                } else {
                    printHelp();
                }
                i++;
            } else if (arguments[i] == "-d") {
                if (i+1 < arguments.size()) {
                    databaseFile = arguments[i+1];
                } else {
                    printHelp();
                }
                i++;
            } else {
                printHelp();
            }
            i++;
        }
    } else {
        printHelp();
    }

    if (workflowFile.isEmpty() || databaseFile.isEmpty())
        printHelp();

    MainWindow w;

    w.setFiles(workflowFile, databaseFile);
    if (!w.load()) {
        app.quit();
        return 1;
    }

    w.show();

    return app.exec();
}
