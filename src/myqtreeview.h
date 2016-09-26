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

#ifndef MYQTREEVIEW_H
#define MYQTREEVIEW_H

#include <QTreeView>

class MyQTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit MyQTreeView(QWidget *parent = 0);
    QModelIndex getSelectedIndex() const;

Q_SIGNALS:
    void key_press(QKeyEvent *event);
    void mouse_press(QMouseEvent *event);

protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
};

#endif // MYQTREEVIEW_H
