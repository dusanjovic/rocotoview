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

#include "myqtreeview.h"

MyQTreeView::MyQTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void MyQTreeView::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
    Q_EMIT key_press(event);
}

void MyQTreeView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);
    Q_EMIT mouse_press(event);
}

QModelIndex MyQTreeView::getSelectedIndex() const
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty())
        return QModelIndex();

    return indexes.at(0);
}
