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

#include "elidedlabel.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>

ElidedLabel::ElidedLabel(QWidget* parent) :
    QFrame(parent),
    m_alignment(Qt::AlignLeft),
    m_elidemode(Qt::ElideNone)
{
}

ElidedLabel::~ElidedLabel()
{
}

QString ElidedLabel::text() const
{
    return m_text;
}

void ElidedLabel::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        updateGeometry();
        update();
    }
}

Qt::Alignment ElidedLabel::alignment() const
{
    return m_alignment;
}

void ElidedLabel::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment != alignment) {
        m_alignment = alignment;
        update();
    }
}

Qt::TextElideMode ElidedLabel::elideMode() const
{
    return m_elidemode;
}

void ElidedLabel::setElideMode(Qt::TextElideMode elidemode)
{
    if (m_elidemode != elidemode) {
        m_elidemode = elidemode;
        updateGeometry();
        update();
    }
}

void ElidedLabel::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    QRect r = contentsRect();

    const QString elidedText = fontMetrics().elidedText(m_text, m_elidemode, r.width());
    painter.drawText(r, m_alignment, elidedText);
}
