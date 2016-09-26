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
#include "rocotomodel.h"

#include <QColor>
#include <QFont>

RocotoModel::RocotoModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    rootItem = new RocotoItem();
}

RocotoModel::~RocotoModel()
{
    delete rootItem;
}

int RocotoModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<RocotoItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant RocotoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    RocotoItem *item = static_cast<RocotoItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    } else if (role == Qt::BackgroundRole && index.column() == 2) {
        QString state = item->data(index.column()).toString();
        if (state == "SUCCEEDED") {
            return SUCCEEDED_COLOR;
        } else if (state == "RUNNING") {
            return RUNNING_COLOR;
        } else if (state == "DEAD" || state == "FAILED" || state == "ERROR") {
            return DEAD_COLOR;
        } else if (state == "QUEUED") {
            return QUEUED_COLOR;
        } else if (state == "SUBMITTING") {
            return SUBMITTING_COLOR;
        } else if (state == "SUSPENDED" || state == "EXPIRED") {
            return SUSPENDED_COLOR;
        } else if (state == "UNKNOWN" || state == "UNAVAILABLE") {
            return UNKNOWN_COLOR;
        } else {
            if (item->childCount() == 0) {
                return QColor(240,240,240);
            } else {
                return QColor("white");
            }
        }
    } else if (role == Qt::FontRole && index.column() == 0 && item->childCount() != 0) {
        QFont italicFont;
        italicFont.setItalic(true);
        return italicFont;
    } else if (role == Qt::ForegroundRole && index.column() == 0 && item->childCount() != 0) {
        return QColor(Qt::gray);
    } else if (role == Qt::TextAlignmentRole && index.column() > 0) {
        return Qt::AlignRight;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags RocotoModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant RocotoModel::headerData(int section,
                                 Qt::Orientation orientation,
                                 int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex RocotoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    RocotoItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<RocotoItem*>(parent.internalPointer());

    RocotoItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex RocotoModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    RocotoItem *childItem = static_cast<RocotoItem*>(index.internalPointer());
    RocotoItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int RocotoModel::rowCount(const QModelIndex &parent) const
{
    RocotoItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<RocotoItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void RocotoModel::setupModelData(const QDomElement &doc)
{
    rootItem->load(doc);
}

void RocotoModel::changedData(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_EMIT dataChanged(topLeft, bottomRight);
}

RocotoItem *RocotoModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<RocotoItem *>(index.internalPointer());
    } else {
        return rootItem;
    }
}
