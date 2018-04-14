/****************************************************************************
**
** Copyright (C) 2013 Thanatat Tamtan
**
** This file is part of Stack CPU.
**
** Stack CPU is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation version 3.
**
** Stack CPU is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Stack CPU.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stackcpu.h"
#include <QMessageBox>
#include <QFontDialog>
#include <QDebug>

#define APPTITLE "Stack CPU"
#define HEXFORMAT "x%1 (%2)"
#define MEMFORMAT "x%1: %2"
#define COMPILEER "Compile error at address x%1: %2"
#define RUNTIMEER "Runtime error at address x%1: %2"

int strToBlock(QString s)
{
    int i = s.indexOf(' ');
    return s.leftRef(i).toString().toInt();
}

StackCPU *stackcpu;
int len;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    stackcpu = new StackCPU();
    len = 2;
}

MainWindow::~MainWindow()
{
    delete stackcpu;
    delete ui;
}

void MainWindow::raiseRuntimeError()
{
    QMessageBox::critical(this, APPTITLE,
                          QString(RUNTIMEER).arg(QString::number(stackcpu->errorAddr(), 16).toUpper(), len, QChar('0'))\
                          .arg(QString::fromStdString(stackcpu->error())));
}

void MainWindow::raiseHaltMessage()
{
    QMessageBox::information(this, APPTITLE, tr("Halt"));
}

void MainWindow::reloadStack()
{
    ui->lstDS->clear();
    ui->lstRS->clear();

    vector<int> tmp = stackcpu->dataStack();
    foreach(unsigned char t, tmp)
    {
        ui->lstDS->addItem(QString(HEXFORMAT).arg(QString::number(t, 16).toUpper(), 2, QChar('0'))\
                           .arg(static_cast<signed char>(t)));
    }

    tmp = stackcpu->returnStack();
    foreach(unsigned char t, tmp)
    {
        ui->lstRS->addItem(QString(HEXFORMAT).arg(QString::number(t, 16).toUpper(), 2, QChar('0'))\
                           .arg(static_cast<signed char>(t)));
    }

    int pc = stackcpu->pc();

    ui->lblPC->setText(QString(HEXFORMAT).arg(QString::number(pc, 16).toUpper(), len, QChar('0'))\
                       .arg(pc));

    ui->lstDS->scrollToBottom();
    ui->lstRS->scrollToBottom();

    QBrush brush = ui->lstMem->palette().brush(QPalette::Active, QPalette::Base);

    for (int i = 0; i < ui->lstMem->count(); ++i)
        ui->lstMem->item(i)->setBackground(brush);

    brush = ui->lstMem->palette().brush(QPalette::Active, QPalette::Highlight);

    if (pc >= 0 && pc < ui->lstMem->count())
    {
        ui->lstMem->item(pc)->setSelected(true);
        ui->lstMem->item(pc)->setBackground(brush);
    }
}

void MainWindow::reloadMemory()
{
    QString t;
    int l = stackcpu->getMemSize();

    // resize list
    while (ui->lstMem->count() < l) ui->lstMem->addItem("");

    len = QString::number(l - 1, 16).length();

    for (int i = 0; i < l; ++i)
    {
        int m = stackcpu->memory(i);
        int p = opGetCode(m);
        t = QString::fromStdString(opGetOps(m));
        if (p == 0xffff || t == "")
        {
            unsigned char r = static_cast<unsigned char>(m);
            t = QString(HEXFORMAT).arg(QString::number(r, 16).toUpper(), 2, QChar('0')).arg(static_cast<signed char>(r));
        }
        ui->lstMem->item(i)->setText(QString(MEMFORMAT).arg(QString::number(i, 16).toUpper(), len, QChar('0')).arg(t));
    }
}

void MainWindow::on_btnReset_clicked()
{
    stackcpu->clearStack();
    reloadStack();
    reloadMemory();
}

void MainWindow::on_btnFont_clicked()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, ui->edtCode->font(), this);
    if (ok)
    {
        ui->edtCode->setFont(f);
        ui->lstMem->setFont(f);
        ui->lstDS->setFont(f);
        ui->lstRS->setFont(f);
    }
}

void MainWindow::on_btnCompile_clicked()
{
    ui->lstMem->clear();
    stackcpu->setMemSize(strToBlock(ui->cmbMemSize->currentText()));
    vector<string> lst;
    QStringList slst = ui->edtCode->toPlainText().split("\n");
    for (int i = 0; i < slst.count(); ++i)
    {
        string tmp = "";
        QString st = slst[i];
        for (int j = 0; j < st.length(); ++j)
            tmp += st[j].toLatin1();
        lst.push_back(tmp);
    }
    stackcpu->setLines(lst);
    if (stackcpu->compile())
    {
        stackcpu->clearStack();
        reloadMemory();
        reloadStack();
    }
    else
        QMessageBox::critical(this, APPTITLE, QString(COMPILEER).arg(QString::number(stackcpu->errorAddr(), 16).toUpper(), len, QChar('0'))\
                              .arg(QString::fromStdString(stackcpu->error())));
}

void MainWindow::on_btnRun_clicked()
{
    bool r = stackcpu->run();
    reloadStack();
    reloadMemory();
    if (!r) raiseRuntimeError();
}

void MainWindow::on_btnInto_clicked()
{
    if (!stackcpu->halt())
    {
        bool r = stackcpu->stepInto();
        reloadStack();
        reloadMemory();
        if (!r) raiseRuntimeError();
    }
    if (stackcpu->halt()) raiseHaltMessage();
}

void MainWindow::on_btnOver_clicked()
{
    if (!stackcpu->halt())
    {
        bool r = stackcpu->stepOver();
        reloadStack();
        reloadMemory();
        if (!r) raiseRuntimeError();
    }
    if (stackcpu->halt()) raiseHaltMessage();
}

void MainWindow::on_lstMem_itemSelectionChanged()
{
    int pc = stackcpu->pc();
    if (pc >= 0 && pc < ui->lstMem->count())
        ui->lstMem->setCurrentRow(pc);
}
