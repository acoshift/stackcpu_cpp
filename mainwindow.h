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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnReset_clicked();
    void on_btnFont_clicked();
    void on_btnCompile_clicked();
    void on_btnRun_clicked();
    void on_btnInto_clicked();
    void on_btnOver_clicked();
    void on_lstMem_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
    void raiseRuntimeError();
    void raiseHaltMessage();
    void reloadStack();
    void reloadMemory();
};

#endif // MAINWINDOW_H
