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

#include "aboutdialog.h"
#include "const.h"
#include "logdialog.h"
#include "mainwindow.h"
#include "rocotomodel.h"
#include "taildialog.h"
#include "updater.h"

#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QRegExpValidator>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    {
        QSettings settings;
        if ( !QFile(settings.fileName()).exists() ) {
            QMessageBox::information(this,
                                     "Information",
                                     "Creating new settings file:\n" + settings.fileName(),
                                     QMessageBox::Ok);

            settings.setValue("tasks_update_interval", TASKS_UPDATE_INTERVAL);
            settings.setValue("cycles_update_interval", CYCLES_UPDATE_INTERVAL);
            settings.sync();
        }
    }

    m_realtime = false;
    m_allowed_update = false;

    ui->setupUi(this);

    QPalette palette;

    palette.setColor(QPalette::Background, SUCCEEDED_COLOR);
    ui->succeededIcon->setPalette(palette);

    palette.setColor(QPalette::Background, RUNNING_COLOR);
    ui->runningIcon->setPalette(palette);

    palette.setColor(QPalette::Background, DEAD_COLOR);
    ui->deadIcon->setPalette(palette);

    palette.setColor(QPalette::Background, QUEUED_COLOR);
    ui->queuedIcon->setPalette(palette);

    palette.setColor(QPalette::Background, SUBMITTING_COLOR);
    ui->submittingIcon->setPalette(palette);

    palette.setColor(QPalette::Background, SUSPENDED_COLOR);
    ui->suspendedIcon->setPalette(palette);

    palette.setColor(QPalette::Background, UNKNOWN_COLOR);
    ui->unknownIcon->setPalette(palette);

    ui->treeView->setModel(&taskmodel);

    ui->cycle_lineEdit->setValidator(new QRegExpValidator(QRegExp("\\d{12}"), this));

    connect(ui->logButton, SIGNAL(clicked()), this, SLOT(showWorkflowLog()));
    connect(ui->treeView, SIGNAL(mouse_press(QMouseEvent *)), this, SLOT(treeview_mousepress(QMouseEvent *)));
    connect(ui->treeView, SIGNAL(key_press(QKeyEvent *)), this, SLOT(treeview_keypress(QKeyEvent *)));
}

MainWindow::~MainWindow()
{
    m_updater_thread.quit();
    m_updater_thread.wait();
    delete ui;
}

void MainWindow::setFiles(const QString &wf, const QString &df)
{
    workflowFile = wf;
    databaseFile = df;
}

bool MainWindow::load()
{
    // Open database file

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseFile);
    db.setConnectOptions("QSQLITE_OPEN_READONLY");
    if (!db.open()) {
        QMessageBox::critical(this,
                              "Error",
                              "Unable to establish a database connection:\n" + databaseFile,
                              QMessageBox::Cancel);
        return false;
    }

    // Open and parse workflow file
    QDomDocument domDocument;

    QFile file(workflowFile);
    bool opened = file.open(QFile::ReadOnly | QFile::Text);
    if (!opened) {
        QMessageBox::critical(this,
                              "Error",
                              "Cannot open workflow file:\n" + workflowFile,
                              QMessageBox::Cancel);
        return false;
    }

    domDocument.setContent(&file);
    file.close();

    QDomDocumentType doctype = domDocument.doctype();
    if (doctype.name() != "workflow")  {
        QMessageBox::critical(this,
                              "Error",
                              "File " + workflowFile + " is not a rocoto workflow file",
                              QMessageBox::Cancel);
        return false;
    }

    QDomElement workflow = domDocument.documentElement();

    if (workflow.tagName() != "workflow") {
        QMessageBox::critical(this,
                              "Error",
                              "Error parsing workflow file:\n" + workflowFile,
                              QMessageBox::Cancel);
        return false;
    }

    m_realtime =  workflow.attributeNode("realtime").value() == "T";
    m_log = workflow.firstChildElement("log").text();

    taskmodel.setupModelData(workflow);

    // Run update thread
    m_updater.setData(&taskmodel, &db, &qcycles, &m_tasks_stat, &m_total_tasks);
    m_updater.moveToThread(&m_updater_thread);
    connect(&m_updater, SIGNAL(tasksUpdated()), this, SLOT(refreshTasks()));
    connect(&m_updater, SIGNAL(cyclesUpdated()), this, SLOT(refreshCycles()));
    connect(&m_updater_thread, SIGNAL(started()), &m_updater, SLOT(start()));
    connect(this, SIGNAL(setSelectedCycleGMT(uint)), &m_updater, SLOT(setSelectedCycleGMT(uint)));
    m_updater_thread.start();

    // Update ui
    ui->treeView->expandAll();
    m_all_expanded = true;
    ui->treeView->resizeColumnToContents(0);
    ui->treeView->resizeColumnToContents(1);

    ui->worlflowLabel->setElideMode(Qt::ElideLeft);
    ui->databaseLabel->setElideMode(Qt::ElideLeft);

    ui->worlflowLabel->setText(workflowFile);
    ui->databaseLabel->setText(databaseFile);

    if (m_realtime) {
        ui->realtimeLabel->setText("REALTIME");
        connect(&m_clock_timer, SIGNAL(timeout()), this, SLOT(updateClock()));
        m_clock_timer.setInterval(1000);
        m_clock_timer.start();
    } else {
        ui->realtimeLabel->setText("RETROSPECTIVE");
    }

    m_allowed_update = true;

    return true;
}

void MainWindow::refreshCycles()
{
    ui->statusBar->showMessage("Refreshing workflow cycles.");

    uint now = QDateTime::currentDateTime().toUTC().toTime_t();

    ui->cycle_comboBox->blockSignals(true);
    ui->cycle_lineEdit->blockSignals(true);

    QString currentText = ui->cycle_comboBox->currentText();

    ui->cycle_comboBox->clear();

    // add new cycles
    for (int i=0; i<qcycles.rowCount(); i++) {
        uint cycle_gmt = qcycles.record(i).value("cycle").toUInt();
        QDateTime cycle;
        cycle.setUtcOffset(0);
        cycle.setTime_t(cycle_gmt);
        if (m_realtime) {
            if ( now - cycle_gmt < 86400+3660 ) {
                ui->cycle_comboBox->addItem(cycle.toString("yyyyMMddhhmm"),cycle_gmt);
            }
        } else {
            ui->cycle_comboBox->addItem(cycle.toString("yyyyMMddhhmm"),cycle_gmt);
        }
    }

    if ( ui->cycle_comboBox->findText(currentText) >=0 ) {
        int index = ui->cycle_comboBox->findText(currentText);
        ui->cycle_comboBox->setCurrentIndex(index);
    } else {
        ui->cycle_comboBox->setCurrentIndex(ui->cycle_comboBox->count()-1);
        ui->cycle_lineEdit->blockSignals(false);
        ui->cycle_comboBox->blockSignals(false);
        ui->cycle_lineEdit->setText(ui->cycle_comboBox->itemText(ui->cycle_comboBox->currentIndex()));
    }

    ui->cycle_lineEdit->blockSignals(false);
    ui->cycle_comboBox->blockSignals(false);

    ui->statusBar->showMessage("Workflow cycles refreshed.", 500);
}

void MainWindow::refreshTasks()
{
    ui->statusBar->showMessage("Refreshing workflow tasks.");

    QModelIndex topLeft = taskmodel.index(0, 0);
    QModelIndex bottomRight = taskmodel.index(taskmodel.rowCount() - 1, taskmodel.columnCount() - 1);
    taskmodel.changedData(topLeft,bottomRight);

    ui->treeView->resizeColumnToContents(1);

    ui->totaltasksLabel->setText(QString::number(m_total_tasks));

    ui->succeededLabel->setText(QString::number(0));
    ui->runningLabel->setText(QString::number(0));
    ui->deadLabel->setText(QString::number(0));
    ui->queuedLabel->setText(QString::number(0));
    ui->submittingLabel->setText(QString::number(0));
    ui->suspendedLabel->setText(QString::number(0));
    ui->unknownLabel->setText(QString::number(0));

    QMapIterator<QString, int> i(m_tasks_stat);
    while (i.hasNext()) {
        i.next();
        const QString state = i.key();
        const int num_tasks = i.value();

        if (state == "SUCCEEDED") {
            ui->succeededLabel->setText(QString::number(num_tasks));
        } else if (state == "RUNNING") {
            ui->runningLabel->setText(QString::number(num_tasks));
        } else if (state == "DEAD") {
            ui->deadLabel->setText(QString::number(num_tasks));
        } else if (state == "QUEUED") {
            ui->queuedLabel->setText(QString::number(num_tasks));
        } else if (state == "SUBMITTING") {
            ui->submittingLabel->setText(QString::number(num_tasks));
        } else if (state == "SUSPENDED") {
            ui->suspendedLabel->setText(QString::number(num_tasks));
        } else if (state == "UNKNOWN") {
            ui->unknownLabel->setText(QString::number(num_tasks));
        }
    }

    ui->statusBar->showMessage("Workflow tasks refreshed.", 500);
}

void MainWindow::treeview_keypress(QKeyEvent *event)
{
    QModelIndex index = ui->treeView->getSelectedIndex();
    if (!index.isValid())
        return;

    if (event->key() == Qt::Key_Z ) {
        if (ui->treeView->isExpanded(ui->treeView->getSelectedIndex()))
            ui->treeView->collapse(ui->treeView->getSelectedIndex());
        else
            ui->treeView->expand(ui->treeView->getSelectedIndex());
    }
}

void MainWindow::treeview_mousepress(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QModelIndex selectedIndex = ui->treeView->getSelectedIndex();
        RocotoItem *item = taskmodel.itemFromIndex(selectedIndex);
        if (item->rocoItemType() == RocotoItem::Task) {
            QMenu menu(this);
            menu.addAction(ui->actionShow_Log);
            menu.addSeparator();
            menu.addAction(ui->actionRocotocheck);
            if (item->state() == "DEAD") {
                menu.addAction(ui->actionRocotoboot);
            }
            menu.exec(event->globalPos());
        }
    }
}

void MainWindow::on_actionShow_Log_triggered()
{
    QModelIndex selectedIndex = ui->treeView->getSelectedIndex();
    RocotoItem *item = taskmodel.itemFromIndex(selectedIndex);
    if (item->rocoItemType() == RocotoItem::Task) {
        QString logFilename = item->join();
        if (QFile::exists(logFilename)) {
            QFile file(logFilename);
            file.open(QIODevice::ReadOnly);
            QTextStream stream(&file);
            QString content = stream.readAll();
            file.close();

            LogDialog *log_dialog = new LogDialog(this);
            log_dialog->setWindowTitle(logFilename);
            log_dialog->setText(content);
            log_dialog->show();
        } else {
            QMessageBox::warning(this,
                                 "Warning",
                                 QString("File does not exist\n%1").arg(logFilename),
                                 QMessageBox::Cancel);
        }
    }
}

void MainWindow::on_actionRocotoboot_triggered()
{
    QModelIndex selectedIndex = ui->treeView->getSelectedIndex();
    RocotoItem *item = taskmodel.itemFromIndex(selectedIndex);
    if (item->rocoItemType() == RocotoItem::Task) {
        const QString command = "rocotoboot";
        QStringList args;
        args << "-w" << workflowFile
             << "-d" << databaseFile
             << "-c" << item->cycle()
             << "-t" << item->name();

        QProcess process;
        process.start(command,args,QIODevice::ReadOnly);

        process.waitForFinished(-1);

        const QString StdOut = QString(process.readAllStandardOutput());
        const QString StdError = QString(process.readAllStandardError());

        LogDialog *log_dialog = new LogDialog(this);
        log_dialog->setWindowTitle(command);
        log_dialog->setText("command was: " + command + " " + args.join(" "));
        log_dialog->setText("------------------------------");
        log_dialog->setText(StdOut);
        log_dialog->setText(StdError);
        log_dialog->show();
    }
}

void MainWindow::on_actionRocotocheck_triggered()
{
    RocotoItem *task = taskmodel.itemFromIndex(ui->treeView->getSelectedIndex());

    if (task->rocoItemType() != RocotoItem::Task)
        return;

    QString s;
    s += "Task: " + task->name() + "\n";

    QMapIterator<QString, QString> attr(task->attributes());
    while (attr.hasNext()) {
        attr.next();
        QString value = attr.value();
        s += "  " + attr.key() + ": " + task->expand(value) + "\n";
    }

    s += "  environment\n";
    QMapIterator<QString, QString> envar(task->envars());
    while (envar.hasNext()) {
        envar.next();
        QString value = envar.value();
        s += "    " + envar.key() + " ==> " + task->expand(value) + "\n";
    }
    s += "\n";

    QString cycle_state;
    if (task->cycle_done() > 0) {
        cycle_state = "done";
    } else if (task->cycle_expired() > 0) {
        cycle_state = "expired";
    } else if (task->cycle_activated() > 0) {
        cycle_state = "active";
    } else {
        cycle_state = "inactive";
    }

    QDateTime cycle_activated;
    cycle_activated.setUtcOffset(0);
    cycle_activated.setTime_t(task->cycle_activated());
    QString cycle_activated_str = (task->cycle_activated() > 0) ? cycle_activated.toString() : "-";

    QDateTime cycle_done;
    cycle_done.setUtcOffset(0);
    cycle_done.setTime_t(task->cycle_done());
    QString cycle_done_str = (task->cycle_done() > 0) ? cycle_done.toString() : "-";

    QDateTime cycle_expired;
    cycle_expired.setUtcOffset(0);
    cycle_expired.setTime_t(task->cycle_expired());
    QString cycle_expired_str = (task->cycle_expired() > 0) ? cycle_expired.toString() : "-";

    s += "Cycle: " + task->cycle() + "\n";
    s += "  State: " + cycle_state + "\n";
    s += "  Activated: " + cycle_activated_str + "\n";
    s += "  Completed: " + cycle_done_str + "\n";
    s += "  Expired: " + cycle_expired_str + "\n";
    s += "\n";

    QString job_str;
    if (task->jobid().isEmpty()) {
        job_str = "This task has not been submitted for this cycle";
        s += "Job: " + job_str+ "\n";
    } else {
        job_str = task->jobid();
        QString job_state_str;
        job_state_str = task->state() + " (" + task->native_state() + ")";
        QString job_exit_status_str = (task->exit_status() >= 0) ? QString::number(task->exit_status()) : "-";
        QString job_tries_str = QString::number(task->tries());
        QString job_nunknown_str = QString::number(task->nunknowns());

        s += "Job: " + job_str+ "\n";
        s += "  State: " + job_state_str + "\n";
        s += "  Exit Status: " + job_exit_status_str + "\n";
        s += "  Tries: " + job_tries_str + "\n";
        s += "  Unknown count: " + job_nunknown_str + "\n";
    }

    LogDialog *log_dialog = new LogDialog(this);
    log_dialog->setWindowTitle("rocotocheck -w ... -d ... -c " + task->cycle() + " -t " + task->name());
    log_dialog->setText(s);
    log_dialog->show();
}

void MainWindow::on_cycle_lineEdit_textChanged(const QString &cycle)
{
    if (!m_allowed_update)
        return;

    if (cycle.length() != 12)
        return;

    QRegExp digits("\\d{12}");
    if (digits.exactMatch(cycle)) {
        QDateTime c;
        c.setUtcOffset(0);
        bool ok;
        bool parse_ok = true;
        int year  = cycle.mid( 0,4).toInt(&ok); parse_ok = parse_ok && ok;
        int month = cycle.mid( 4,2).toInt(&ok); parse_ok = parse_ok && ok;
        int day   = cycle.mid( 6,2).toInt(&ok); parse_ok = parse_ok && ok;
        int hour  = cycle.mid( 8,2).toInt(&ok); parse_ok = parse_ok && ok;
        int min   = cycle.mid(10,2).toInt(&ok); parse_ok = parse_ok && ok;

        if (year < 1970) parse_ok = false;
        if (month < 1 || month > 12) parse_ok = false;
        if (day < 1 || day > 31) parse_ok = false;
        if (hour < 0 || hour > 23) parse_ok = false;
        if (min < 0 || min > 59) parse_ok = false;

        if (!parse_ok) {
            QMessageBox::warning(this,
                                 "Warning",
                                 QString("%1 is not valid date/time").arg(cycle),
                                 QMessageBox::Cancel);
            return;
        }
        c.setDate(QDate(year,month,day));
        c.setTime(QTime(hour,min,0,0));
        if ( c.isValid() && c >= QDateTime(QDate(1970,1,1),QTime(0,0,0),Qt::UTC) ) {
            uint cycle_gmt = c.toTime_t();
            Q_EMIT setSelectedCycleGMT(cycle_gmt);
            ui->logButton->setEnabled(true);
        } else {
            QMessageBox::warning(this,
                                 "Warning",
                                 QString("%1 is not valid date/time").arg(cycle),
                                 QMessageBox::Cancel);
            return;
        }
    } else {
        QMessageBox::warning(this,
                             "Warning",
                             QString("%1 is not valid date/time").arg(cycle),
                             QMessageBox::Cancel);
        return;
    }
}

void MainWindow::updateClock()
{
    ui->nowLabel->setText(QDateTime::currentDateTime().toUTC().toString("yyyy/MM/dd hh:mm ") + "UTC");
}

void MainWindow::showWorkflowLog()
{
    QString cycle = ui->cycle_lineEdit->text();
    if (cycle.isEmpty()) {
        QMessageBox::warning(this,
                             "Warning",
                             "Cycle is not available",
                             QMessageBox::Cancel);
        return;
    }

    QString filename = m_log;

    filename.replace("@Y",cycle.mid( 0,4));
    filename.replace("@y",cycle.mid( 2,2));
    filename.replace("@m",cycle.mid( 4,2));
    filename.replace("@d",cycle.mid( 6,2));
    filename.replace("@H",cycle.mid( 8,2));
    filename.replace("@M",cycle.mid(10,2));

    if (!QFile::exists(filename)) {
        QMessageBox::warning(this,
                             "Warning",
                             "File does not exist:\n" + filename,
                             QMessageBox::Cancel);
        return;
    }

    QFileInfo f(filename);
    if (!f.isReadable()) {
        QMessageBox::warning(this,
                             "Warning",
                             "File is not readable:\n" + filename,
                             QMessageBox::Cancel);
        return;
    }

    TailDialog *tail_dialog = new TailDialog(filename,this);
    tail_dialog->setWindowTitle(filename);
    tail_dialog->show();
}

void MainWindow::on_actionAbout_rocoview_triggered()
{
    AboutDialog *about_dialog = new AboutDialog(this);
    about_dialog->show();
}

void MainWindow::on_actionToggle_Fold_triggered()
{
    if (m_all_expanded)
        ui->treeView->collapseAll();
    else
        ui->treeView->expandAll();

    m_all_expanded = !m_all_expanded;
}
