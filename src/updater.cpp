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

#include "const.h"
#include "updater.h"

#include <QDateTime>
#include <QSettings>
#include <QSqlRecord>

Updater::Updater(QObject *parent) :
    QObject(parent)
{
    m_model = NULL;
    m_db = NULL;
    m_qcycles = NULL;
    m_tasks_stat = NULL;
    m_total_tasks = NULL;
}

void Updater::setData(const RocotoModel *model,
                      const QSqlDatabase *db,
                      QSqlQueryModel *qcycles,
                      QMap<QString, int> *tasks_stat,
                      int *total_tasks)
{
    m_model = model;
    m_db = db;
    m_qcycles = qcycles;
    m_tasks_stat = tasks_stat;
    m_total_tasks = total_tasks;

    bool ok;
    QSettings settings;
    int tasks_update_interval = settings.value("tasks_update_interval").toInt(&ok);
    if (!ok || tasks_update_interval < 1) {
        tasks_update_interval = TASKS_UPDATE_INTERVAL;
    }

    int cycles_update_interval = settings.value("cycles_update_interval").toInt(&ok);
    if (!ok || cycles_update_interval < 1) {
        cycles_update_interval = CYCLES_UPDATE_INTERVAL;
    }

    connect(&m_timer_cycles, SIGNAL(timeout()), this, SLOT(update_cycles()));
    m_timer_cycles.start(cycles_update_interval*1000);

    connect(&m_timer_tasks, SIGNAL(timeout()), this, SLOT(update_tasks()));
    m_timer_tasks.start(tasks_update_interval*1000);
}

void Updater::start()
{
    update_cycles();
}

void Updater::setSelectedCycleGMT(const uint cycle_gmt)
{
    m_selected_cycle_gmt = cycle_gmt;
    update_tasks();
}

void Updater::update_cycles()
{
    m_mutex.lock();
    m_qcycles->setQuery(QString("SELECT cycle,activated,expired FROM cycles"),*m_db);
    while (m_qcycles->canFetchMore())
        m_qcycles->fetchMore();
    Q_EMIT cyclesUpdated();
    m_mutex.unlock();
}

void Updater::update_tasks()
{
    m_mutex.lock();
    m_query.setQuery(QString("SELECT taskname,cycle,state,native_state,jobid,exit_status,tries,nunknowns,duration FROM jobs WHERE cycle = '%1'").arg(m_selected_cycle_gmt),*m_db);
    while (m_query.canFetchMore())
        m_query.fetchMore();
    *m_total_tasks = 0;
    m_tasks_stat->clear();
    update_item(QModelIndex());
    Q_EMIT tasksUpdated();
    m_mutex.unlock();
}

void Updater::update_item(const QModelIndex & Top)
{
    QModelIndex item;

    for( int r = 0; r < m_model->rowCount( Top ); r++ ) {
        item = m_model->index( r, 0, Top );
        RocotoItem *task = m_model->itemFromIndex(item);
        task->clear();
        if (task->rocoItemType() == RocotoItem::Task) {
            (*m_total_tasks)++;
            QString taskname = task->name();
            for (int i=0; i<m_query.rowCount(); i++) {
                if (m_query.record(i).value("taskname").toString() == taskname) {
                    QDateTime cycle;
                    cycle.setUtcOffset(0);
                    cycle.setTime_t      (m_query.record(i).value("cycle").toUInt());
                    task->setCycle       (cycle.toString("yyyyMMddhhmm"));
                    task->setState       (m_query.record(i).value("state").toString());
                    task->setNativeState (m_query.record(i).value("native_state").toString());
                    task->setJobID       (m_query.record(i).value("jobid").toString());
                    task->setExitStatus  (m_query.record(i).value("exit_status").toInt());
                    task->setTries       (m_query.record(i).value("tries").toInt());
                    task->setNunknowns   (m_query.record(i).value("nunknowns").toInt());
                    task->setDuration    (m_query.record(i).value("duration").toDouble());

                    if (task->state() == "SUCCEEDED") {
                        (*m_tasks_stat)["SUCCEEDED"]++;
                    }  else if (task->state() == "RUNNING") {
                        (*m_tasks_stat)["RUNNING"]++;
                    } else if (task->state() == "DEAD" ||
                               task->state() == "FAILED" ||
                               task->state() == "ERROR") {
                        (*m_tasks_stat)["DEAD"]++;
                    } else if (task->state() == "QUEUED") {
                        (*m_tasks_stat)["QUEUED"]++;
                    } else if (task->state() == "SUBMITTING") {
                        (*m_tasks_stat)["SUBMITTING"]++;
                    } else if (task->state() == "SUSPENDED" ||
                               task->state() == "EXPIRED") {
                        (*m_tasks_stat)["SUSPENDED"]++;
                    } else if (task->state() == "UNKNOWN" ||
                               task->state() == "UNAVAILABLE") {
                        (*m_tasks_stat)["UNKNOWN"]++;
                    } else {
                        (*m_tasks_stat)["EMPTY"]++;
                    }

                    QSqlQueryModel qcycle;
                    qcycle.setQuery(QString("SELECT cycle,activated,expired,done FROM cycles WHERE cycle = '%1'").arg(task->cycle_gmt()));
                    if ( qcycle.record(0).value("cycle").toInt() == task->cycle_gmt()) {
                        task->setCycleActivated(qcycle.record(0).value("activated").toInt());
                        task->setCycleExpired(qcycle.record(0).value("expired").toInt());
                        task->setCycleDone(qcycle.record(0).value("done").toInt());
                    }
                    task->updateData();
                    break;
                }
            }
        }
        update_item( item );
    }
}
