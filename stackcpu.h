/****************************************************************************
**
** Copyright (C) 2013 Moon Rhythm
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

#ifndef STACKCPU_H
#define STACKCPU_H

#include <string>
#include <cstring>
#include <vector>

using namespace std;

class StackCPU
{
public:
    StackCPU();
    ~StackCPU();
    bool compile();
    void clearStack();
    bool run();
    bool stepInto();
    bool stepOver();
    void setLines(vector<string> l);
    vector<string> *getLines() const;
    string error() const;
    int errorAddr() const;
    int pc() const;
    bool halt() const;
    vector<int> dataStack() const;
    vector<int> returnStack() const;
    int getMemSize() const;
    void setMemSize(int val);
    int memory(int i) const;

private:
    vector<string> *lines;
    string lastError;
    int lastErrorAddr;
    int *mem, *ftmem;
    vector<int> *ds, *rs;
    int fpc;
    bool fhalt;
    int memSize;

    int getMem(int addr);
    void setMem(int addr, int val);
    bool push(vector<int> *stack, int val);
    int pop(vector<int> *stack);
    int peek(vector<int> *stack);
    void lineReconstruct();
    bool preprocessing();
    bool processing();
    bool step();
};

struct Opcode
{
    string ops;
    int opc;
    int pci;
};

int opGetPci(string ops);
int opGetPci(int opc);
int opGetOpc(string ops);
string opGetOps(int opc);
int opGetCode(int opc);

#endif // STACKCPU_H
