##
##  rocotoview - Rocoto workflow viewer
##
##  Copyright (C) 2014-2016 by Dusan Jovic
##
##  This file is part of rocotoview.
##
##  rocotoview is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  rocotoview is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with rocotoview.  If not, see <http:##www.gnu.org/licenses/>.
##


QT += core gui xml sql

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += -Wno-long-long
}

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic -ansi

CONFIG += no_keywords

TARGET = ../bin/rocotoview

TEMPLATE = app

UI_DIR       = .ui
MOC_DIR      = .moc
OBJECTS_DIR  = .obj

SOURCES += main.cpp\
           mainwindow.cpp \
           myqtreeview.cpp \
           rocotoitem.cpp \
           rocotomodel.cpp \
           logdialog.cpp \
           elidedlabel.cpp \
           aboutdialog.cpp \
           updater.cpp \
           taildialog.cpp

HEADERS += mainwindow.h \
           myqtreeview.h \
           rocotoitem.h \
           rocotomodel.h \
           logdialog.h \
           elidedlabel.h \
           aboutdialog.h \
           const.h \
           updater.h \
           taildialog.h

FORMS   += mainwindow.ui \
           logdialog.ui \
           aboutdialog.ui \
           taildialog.ui

