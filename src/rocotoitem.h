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

#ifndef ROCOTOITEM_H
#define ROCOTOITEM_H

#include <QList>
#include <QVariant>

#include <QDomDocument>

class RocotoItem
{
public:
    enum RocoItemType { Unknown, Task, Metatask, Log };
    explicit RocotoItem(RocotoItem *parent = 0);
    ~RocotoItem ();

    void appendChild(RocotoItem *child);

    RocotoItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    QList<QVariant> data() const;
    int row() const;
    RocotoItem *parent();

    void setData(const QList<QVariant> &data);
    void clear();
    void updateData();

    void load(const QDomElement& element, const int subtask_num = -1);

    void setMap(const QString& metaname, const QString& metavalue);

    QMap<QString, QString> map() const;
    QMap<QString, QString> meta_vars() const;
    QMap<QString, QString> envars() const;
    QMap<QString, QString> attributes() const;

    void setName(const QString &name);
    void setRocoItemType(const RocoItemType rocoitemtype);
    void setCycle(const QString& cycle);
    void setState(const QString& state);
    void setNativeState(const QString& native_state);
    void setJobID(const QString& jobid);
    void setExitStatus(const int exit_status);
    void setTries(const int tries);
    void setNunknowns(const int nunknowns);
    void setDuration(const double duration);

    void setCycleActivated(const int activated);
    void setCycleExpired(const int expired);
    void setCycleDone(const int done);

    QString expand(QString &str);

    RocoItemType rocoItemType() const;
    QString name();
    QString cycle() const;
    int cycle_gmt() const;
    QString state() const;
    QString native_state() const;
    QString jobid() const;
    int exit_status() const;
    int tries() const;
    int nunknowns() const;
    double duration() const;

    QString join();

    int cycle_activated() const;
    int cycle_expired() const;
    int cycle_done() const;

private:

    QList<RocotoItem*> childItems;
    QList<QVariant> itemData;
    RocotoItem *parentItem;

    QMap<QString, QString> m_map;
    QMap<QString, QString> m_meta_vars;

    QString m_jobid;
    int m_cycle_gmt;
    QString m_cycle;
    QString m_state;
    QString m_native_state;
    int m_exit_status;
    int m_tries;
    int m_nunknowns;
    double m_duration;

    RocoItemType m_rocoitemtype;
    QMap<QString, QString> m_envars;
    QMap<QString, QString> m_attributes;

    int m_cycle_activated;
    int m_cycle_expired;
    int m_cycle_done;
};

#endif // ROCOTOITEM_H
