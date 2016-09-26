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

#include "rocotoitem.h"

#include <QDateTime>
#include <QStringList>

RocotoItem::RocotoItem(RocotoItem *parent)
{
    parentItem = parent;
    m_rocoitemtype = Unknown;

    m_cycle_gmt = -1;
    m_cycle_activated = -1;
    m_cycle_expired = -1;
    m_cycle_done = -1;

    m_exit_status = -1;
    m_tries = -1;
    m_nunknowns = 0;
    m_duration = -1.0;

    if (parentItem)
        m_map = parentItem->map();
}

RocotoItem::~RocotoItem()
{
    qDeleteAll(childItems);
}

void RocotoItem::appendChild(RocotoItem *item)
{
    childItems.append(item);
}

RocotoItem *RocotoItem::child(int row)
{
    return childItems.value(row);
}

int RocotoItem::childCount() const
{
    return childItems.count();
}

int RocotoItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<RocotoItem*>(this));

    return 0;
}

int RocotoItem::columnCount() const
{
    return itemData.count();
}

QVariant RocotoItem::data(int column) const
{
    return itemData.value(column);
}

QList<QVariant> RocotoItem::data() const
{
    return itemData;
}

RocotoItem *RocotoItem::parent()
{
    return parentItem;
}

void RocotoItem::setData(const QList<QVariant> &data)
{
    itemData = data;
}

void RocotoItem::load(const QDomElement& element, const int subtask_num)
{
    QDomNamedNodeMap attributes = element.attributes();
    for( uint i = 0; i < (uint)attributes.length(); ++i )
    {
        if ( attributes.item(i).isAttr() ) {
            QDomAttr a = attributes.item(i).toAttr();
            m_attributes[a.name()] = a.value();
        }
    }

    if (parentItem && subtask_num >= 0) {
        QMap<QString, QString> parent_meta_vars = parentItem->meta_vars();
        QMapIterator<QString, QString> i(parent_meta_vars);
        while (i.hasNext()) {
            i.next();
            QString metaname = i.key();
            QStringList metavalues = i.value().split(" ");
            if (subtask_num < metavalues.count()) {
                setMap(metaname, metavalues[subtask_num]);
            }
        }

    }

    if( element.tagName() == "task" ) {
        m_rocoitemtype = Task;

        QDomNodeList nodes = element.childNodes();
        for( uint i = 0; i < (uint)nodes.length(); ++i )
        {
            if( nodes.item( i ).isElement() ) {
                QDomElement e = nodes.item( i ).toElement();
                if (e.tagName() != "envar") {
                    m_attributes[e.tagName()] = e.text();
                }
            }
        }

        QDomNodeList envar_list = element.elementsByTagName("envar");
        for ( uint i = 0; i < (uint)envar_list.length(); i++ ) {
            QDomNode envar = envar_list.item(i);
            m_envars[envar.firstChildElement("name").text()] = envar.firstChildElement("value").text();
        }

    } else if ( element.tagName() == "metatask" ) {
        m_rocoitemtype = Metatask;
    }

    QDomNodeList list = element.childNodes();
    for( int i = 0; i < list.count(); ++i )
    {
        if( list.item( i ).isElement() ) {
            QDomElement e = list.item( i ).toElement();

            if( e.tagName() == "task" ) {
                RocotoItem *task = new RocotoItem(this);
                task->load(e);
                this->appendChild(task);
            } else if (e.tagName() == "metatask") {
                RocotoItem *meta = new RocotoItem(this);
                int firstvarCount = e.firstChildElement("var").text().trimmed().split(" ").count();

                // build a list of var variables;
                QDomNodeList ch = e.childNodes();
                for( int k = 0; k < ch.count(); ++k ) {
                    if( ch.item( k ).isElement() ) {
                        QDomElement var = ch.item( k ).toElement();
                        if( var.tagName() == "var" ) {
                            m_meta_vars[var.attribute("name")] = var.text().trimmed();
                        }
                    }
                }

                // expand metatask
                for (int j=0; j < firstvarCount; j++) {
                    meta->load(e,j);
                }

                this->appendChild(meta);
            }
        }
    }

    updateData();
}

void RocotoItem::clear()
{
    m_cycle = "";
    m_state = "";
    m_native_state = "";
    m_jobid = "";
    m_exit_status = -1;
    m_tries = -1;
    m_nunknowns = 0;
    m_duration = -1.0;
    updateData();
}

void RocotoItem::updateData()
{
    QList<QVariant> taskData;

    if (!parentItem) {
        taskData << "Task" << "Cycle" << "State" << "JobID" << "Exit status" << "Tries" << "Duration";
    } else {
        if (m_rocoitemtype == Task) {

            taskData << name();

            if (!m_cycle.isEmpty()) {
                taskData << m_cycle;
            } else {
                taskData << "-";
            }

            if (!m_state.isEmpty()) {
                taskData << m_state;
            } else {
                taskData << "-";
            }

            if (!m_jobid.isEmpty()) {
                taskData << m_jobid;
            } else {
                taskData << "-";
            }

            if ( m_exit_status >= 0 ) {
                taskData << m_exit_status;
            } else {
                taskData << "-";
            }

            if ( m_tries >= 0 ) {
                taskData << m_tries;
            } else {
                taskData << "-";
            }

            if ( m_duration >= 0.0 ) {
                taskData << QString::number(m_duration, 'f', 1);
            } else {
                taskData << "-";
            }

        } else {
            taskData << name() << "" << "" << "" << "" << "" << "";
        }
    }

    setData(taskData);
}

QString RocotoItem::expand(QString &str)
{
    QMapIterator<QString, QString> i(m_map);
    while (i.hasNext()) {
        i.next();
        QString key = "#" + i.key() + "#";
        QString value = i.value();
        str.replace(key,value);
    }
    if ( !m_cycle.isEmpty() ) {
        QDateTime c;
        c.setUtcOffset(0);
        c.setTime_t(m_cycle_gmt);
        str.replace("@Y",c.toString("yyyy"));
        str.replace("@y",c.toString("yy"));
        str.replace("@m",c.toString("MM"));
        str.replace("@d",c.toString("dd"));
        str.replace("@H",c.toString("hh"));
        str.replace("@M",c.toString("mm"));

        str.replace("@j",QString::number(c.date().dayOfYear()));
    }

    return str;
}

void RocotoItem::setMap(const QString& metaname, const QString& metavalue)
{
    m_map[metaname] = metavalue;
}

QMap<QString, QString> RocotoItem::map() const
{
    return m_map;
}

QMap<QString, QString> RocotoItem::meta_vars() const
{
    return m_meta_vars;
}

QMap<QString, QString> RocotoItem::envars() const
{
    return m_envars;
}

QMap<QString, QString> RocotoItem::attributes() const
{
    return m_attributes;
}

void RocotoItem::setRocoItemType(const RocoItemType rocoitemtype){
    m_rocoitemtype = rocoitemtype;
}

RocotoItem::RocoItemType RocotoItem::rocoItemType() const
{
    return m_rocoitemtype;
}

void RocotoItem::setCycle(const QString& cycle)
{
    m_cycle = cycle;
    QDateTime c;
    c.setUtcOffset(0);
    int year = m_cycle.mid(0,4).toInt();
    int month = m_cycle.mid(4,2).toInt();
    int day = m_cycle.mid(6,2).toInt();
    int hour = m_cycle.mid(8,2).toInt();
    int min = m_cycle.mid(10,2).toInt();
    c.setDate(QDate(year,month,day));
    c.setTime(QTime(hour,min,0,0));
    m_cycle_gmt = c.toTime_t();
}

void RocotoItem::setState(const QString& state)
{
    m_state = state;
}

void RocotoItem::setNativeState(const QString& native_state)
{
    m_native_state = native_state;
}

void RocotoItem::setJobID(const QString& jobid)
{
    m_jobid = jobid;
}

void RocotoItem::setExitStatus(const int exit_status)
{
    m_exit_status = exit_status;
}

void RocotoItem::setTries(const int tries)
{
    m_tries = tries;
}

void RocotoItem::setNunknowns(const int nunknowns)
{
    m_nunknowns = nunknowns;
}

void RocotoItem::setDuration(const double duration)
{
    m_duration = duration;
}

void RocotoItem::setCycleActivated(const int activated)
{
    m_cycle_activated = activated;
}

void RocotoItem::setCycleExpired(const int expired)
{
    m_cycle_expired = expired;
}

void RocotoItem::setCycleDone(const int done)
{
    m_cycle_done = done;
}

void RocotoItem::setName(const QString &name)
{
    m_attributes["name"] = name;
}

QString RocotoItem::name()
{
    QString n = m_attributes["name"];
    expand(n);
    return n;
}

QString RocotoItem::join()
{
    QString j = m_attributes["join"];
    expand(j);
    return j;
}

QString RocotoItem::cycle() const
{
    return m_cycle;
}

int RocotoItem::cycle_gmt() const
{
    return m_cycle_gmt;
}

QString RocotoItem::state() const
{
    return m_state;
}

QString RocotoItem::native_state() const
{
    return m_native_state;
}

QString RocotoItem::jobid() const
{
    return m_jobid;
}

int RocotoItem::exit_status() const
{
    return m_exit_status;
}

int RocotoItem::tries() const
{
    return m_tries;
}

int RocotoItem::nunknowns() const
{
    return m_nunknowns;
}

double RocotoItem::duration() const
{
    return m_duration;
}

int RocotoItem::cycle_activated() const
{
    return m_cycle_activated;
}

int RocotoItem::cycle_expired() const
{
    return m_cycle_expired;
}

int RocotoItem::cycle_done() const
{
    return m_cycle_done;
}
