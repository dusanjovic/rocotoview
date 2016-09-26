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

#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QFrame>

class ElidedLabel : public QFrame
{
    Q_OBJECT

public:
    explicit ElidedLabel(QWidget* parent = 0);
    virtual ~ElidedLabel();

    QString text() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode elidemode);

public Q_SLOTS:
    void setText(const QString& text);

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    QString m_text;
    Qt::Alignment m_alignment;
    Qt::TextElideMode m_elidemode;
};

#endif // ELIDEDLABEL_H
