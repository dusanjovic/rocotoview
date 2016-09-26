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

#ifndef UPDATER_H
#define UPDATER_H

#include "rocotomodel.h"

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QTimer>

class Updater : public QObject
{
    Q_OBJECT

public:
    explicit Updater(QObject *parent = 0);

    void setData(const RocotoModel *model,
                 const QSqlDatabase *db,
                 QSqlQueryModel *qcycles,
                 QMap<QString, int> *tasks_stat,
                 int *total_tasks);

public Q_SLOTS:
    void start();
    void setSelectedCycleGMT(const uint cycle_gmt);

private Q_SLOTS:
    void update_cycles();
    void update_tasks();

Q_SIGNALS:
    void tasksUpdated( void ) const;
    void cyclesUpdated( void ) const;

private:
    const RocotoModel *m_model;
    const QSqlDatabase *m_db;
    QSqlQueryModel *m_qcycles;
    QMap<QString, int> *m_tasks_stat;
    int *m_total_tasks;

    uint m_selected_cycle_gmt;

    QTimer m_timer_cycles;
    QTimer m_timer_tasks;

    QSqlQueryModel m_query;

    void update_item(const QModelIndex &Top);

    QMutex m_mutex;

};

#endif // UPDATER_H
