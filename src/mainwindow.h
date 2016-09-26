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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "updater.h"
#include "rocotomodel.h"

#include <QMainWindow>
#include <QSqlDatabase>
#include <QMouseEvent>
#include <QLabel>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setFiles(const QString & wf, const QString & df);
    bool load();

Q_SIGNALS:
    void setSelectedCycleGMT( const uint cycle_gmt ) const;

protected Q_SLOTS:
    void updateClock();
    void refreshCycles();
    void refreshTasks();
    void treeview_mousepress(QMouseEvent *event);
    void treeview_keypress(QKeyEvent *event);
    void showWorkflowLog();

    void on_actionShow_Log_triggered();
    void on_actionRocotoboot_triggered();
    void on_actionRocotocheck_triggered();

    void on_cycle_lineEdit_textChanged(const QString &cycle);
    void on_actionAbout_rocoview_triggered();
    void on_actionToggle_Fold_triggered();

private:
    Ui::MainWindow *ui;

    QSqlDatabase db;
    RocotoModel taskmodel;
    QSqlQueryModel qcycles;

    QString workflowFile;
    QString databaseFile;

    bool m_realtime;
    bool m_allowed_update;

    QString m_log;

    QMap<QString, int> m_tasks_stat;
    int m_total_tasks;
    bool m_all_expanded;

    QTimer m_clock_timer;

    QThread m_updater_thread;
    Updater m_updater;
};

#endif // MAINWINDOW_H
