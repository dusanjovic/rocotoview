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

#ifndef CONST_H
#define CONST_H

#include <QColor>

const int CYCLES_UPDATE_INTERVAL = 5*60; //  5 min
const int TASKS_UPDATE_INTERVAL  =   15; // 15 sec

const QColor SUCCEEDED_COLOR  = QColor(255,225, 53);
const QColor RUNNING_COLOR    = QColor(  5,250,  5);
const QColor DEAD_COLOR       = QColor(250,  5,  5);
const QColor QUEUED_COLOR     = QColor(153,205,255);
const QColor SUBMITTING_COLOR = QColor(  0,250,250);
const QColor SUSPENDED_COLOR  = QColor(250,153,  0);
const QColor UNKNOWN_COLOR    = QColor(255,192,203);

#endif // CONST_H
