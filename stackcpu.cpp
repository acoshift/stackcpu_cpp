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

#include "stackcpu.h"

#define MAXSTACK 0xff
#define OPL 0x12

#define E001 "Label redecleared: %s"
#define E002 "\"%s\" is not a valid integer value"
#define E003 "Undeclared label: %s"
#define E004 "Constant value violates subrange bounds: %s"
#define E005 "Stack overflow/underflow"
#define E006 "PC out of bounds"
#define E007 "\"x%s\" is not an opcode"
#define E008 "Out of memory"

const Opcode opcodes[OPL] = {
    {"LIT",     0xff00, 2},
    {"@",       0xff01, 1},
    {"!",       0xff02, 1},
    {"DROP",    0xff03, 1},
    {"DUP",     0xff04, 1},
    {"OVER",    0xff05, 1},
    {"SWAP",    0xff06, 1},
    {"+",       0xff07, 1},
    {"-",       0xff08, 1},
    {"AND",     0xff09, 1},
    {"OR",      0xff0a, 1},
    {"XOR",     0xff0b, 1},
    {"IF",      0xff0c, 2},
    {"CALL",    0xff0d, 2},
    {"EXIT",    0xff0e, 0},
    {"HALT",    0xff0f, 0},
    {">R",      0xff10, 1},
    {"R>",      0xff11, 1},
};

int opGetPci(string ops) {
    for (auto&& op : opcodes) {
        if (op.ops == ops) {
            return op.pci;
        }
    }
    return 0;
}

int opGetPci(int opc) {
    for (auto&& op : opcodes) {
        if (op.opc == opc) {
            return op.pci;
        }
    }
    return 0;
}

int opGetOpc(string ops) {
    for (auto&& op : opcodes) {
        if (op.ops == ops) {
            return op.opc;
        }
    }
    return 0xffff;
}

string opGetOps(int opc) {
    for (auto&& op : opcodes) {
        if (op.opc == opc) {
            return op.ops;
        }
    }
    return "";
}

int opGetCode(int opc) {
    if ((opc & 0xff00) == 0xff00) return opc & 0x00ff;
    return 0xffff;
}

bool tryNumToInt(string num, int *val) {
    char* ok = NULL;
    if (num.substr(0, 1) == "B")
        *val = strtol(num.substr(1, num.length() - 1).c_str(), &ok, 2);
    else if (num.substr(0, 2) == "0X")
        *val = strtol(num.substr(2, num.length() - 2).c_str(), &ok, 16);
    else if (num.substr(0, 1) == "X")
        *val = strtol(num.substr(1, num.length() - 1).c_str(), &ok, 16);
    else
        *val = strtol(num.c_str(), &ok, 10);
    return ok[0] != num[0];
}

string strToUpper(string s) {
    string r;
    for (auto&&c : s) {
        r += toupper(c);
    }
    return r;
}

string intToStr(int i) {
    char buff[255];
    sprintf(buff, "%d", i);
    return buff;
}

StackCPU::StackCPU() {
    lines = new vector<string>();
    ds = new vector<int>();
    rs = new vector<int>();
    lastError = "";
    lastErrorAddr = 0;
    memSize = 32;
    mem = new int[memSize];
    memset(mem, 0, memSize * sizeof(int));
    ftmem = new int[memSize];
    memset(ftmem, 0, memSize * sizeof(int));
    fpc = -1;
    fhalt = true;
}

StackCPU::~StackCPU() {
    delete lines;
    delete ds;
    delete rs;
    delete[] mem;
    delete[] ftmem;
}

int StackCPU::getMem(int addr) {
    if (addr < 0 || addr >= memSize) {
        return 0;
    }
    return mem[addr];
}

void StackCPU::setMem(int addr, int val) {
    if (addr >= 0 && addr < memSize) {
        mem[addr] = val;
    }
}

bool StackCPU::push(vector<int> *stack, int val) {
    if (stack->size() < MAXSTACK) {
        stack->push_back(val);
        return true;
    }
    return false;
}

int StackCPU::pop(vector<int> *stack) {
    if (!stack->empty()) {
        int top = stack->back();
        stack->pop_back();
        return top;
    }
    return 0xffff;
}

int StackCPU::peek(vector<int> *stack) {
    if (!stack->empty()) {
        return stack->back();
    }
    return 0xffff;
}

void StackCPU::lineReconstruct() {
    vector<string> l;
    string s, t;
    for (auto&& s : *lines) {
        t = "";
        for (auto&& c : s) {
            if (c == ' ' || c == '\t') {
                if (t != "") l.push_back(strToUpper(t));
                t = "";
                continue;
            }
            if (c == ';') break;
            t += c;
        }
        if (t != "") l.push_back(strToUpper(t));
    }
    lines->clear();
    lines->insert(lines->end(), l.begin(), l.end());
}

bool StackCPU::preprocessing() {
    vector<Opcode> m;
    string s;
    int l;

    // find label and convert number to decimal
    for (size_t i = 0; i < lines->size(); ) {
        s = lines->at(i);
        if (s.substr(0, 1) != ":") {
            int j = opGetPci(s);
            for (int k = 1; k < j; ++k) {
                if (i + k >= lines->size()) break;
                s = lines->at(i + k);
                if (s.substr(0, 1) != ":") {
                    if (tryNumToInt(s, &l)) {
                        if ((abs(l) & 0xff00) != 0) {
                            char buff[255];
                            sprintf(buff, E004, s.c_str());
                            lastError = buff;
                            lastErrorAddr = i + k;
                            return false;
                        }
                        lines->at(i + k) = intToStr(l);
                    } else {
                        char buff[255];
                        sprintf(buff, E002, s.c_str());
                        lastError = buff;
                        lastErrorAddr = i + k;
                        return false;
                    }
                }
            }
            if (j != 0) i += j; else ++i;
            continue;
        } else {
            // check dup
            for (auto&& op : m) {
                if (op.ops == s) {
                    char buff[255];
                    sprintf(buff, E001, s.c_str());
                    lastError = buff;
                    lastErrorAddr = i;
                    return false;
                }
            }
            Opcode t;
            t.ops = s;
            t.opc = i;
            m.push_back(t);
            lines->erase(lines->begin() + i);
        }
    }

    // replace label
    for (auto&& s : *lines) {
        if (s.substr(0, 1) == ":") {
            for (auto&& op : m) {
                if (op.ops == s) {
                    s = intToStr(op.opc);
                    break;
                }
            }
        }
    }
    return true;
}

bool StackCPU::processing() {
    vector<int> d;
    string s;
    int op, l;

    for (size_t i = 0; i < lines->size(); ++i) {
        s = lines->at(i);
        op = opGetOpc(s);
        if (op != 0xffff) {
            d.push_back(op);
        } else {
            if (tryNumToInt(s, &l)) {
                d.push_back(l);
            } else {
                if (s.substr(0, 1) == ":") {
                    char buff[255];
                    sprintf(buff, E003, s.c_str());
                    lastError = buff;
                } else {
                    char buff[255];
                    sprintf(buff, E002, s.c_str());
                    lastError = buff;
                }
                lastErrorAddr = i;
                return false;
            }
        }
    }

    size_t m = (size_t) memSize;
    delete[] ftmem;
    ftmem = new int[m];
    memset(ftmem, 0, m * sizeof(int));
    for (size_t i = 0; i < d.size(); ++i) {
        if (i < m) {
            ftmem[i] = d[i];
        } else {
            lastError = E008;
            lastErrorAddr = i;
            return false;
        }
    }
    return true;
}

bool StackCPU::step() {
    bool ret = false;
    bool nop = false;
    int addr, tmp1, tmp2;
    int s = mem[fpc];

    switch (s) {
    case 0xff00:
        ret = push(ds, getMem(fpc + 1));
        break;
    case 0xff01:
        addr = pop(ds);
        ret = (addr != 0xffff) && push(ds, getMem(addr));
        break;
    case 0xff02:
        addr = pop(ds);
        ret = addr != 0xffff;
        setMem(addr, pop(ds));
        break;
    case 0xff03:
        ret = pop(ds) != 0xffff;
        break;
    case 0xff04:
        tmp1 = peek(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1);
        break;
    case 0xff05:
        tmp1 = pop(ds);
        tmp2 = peek(ds);
        push(ds, tmp1);
        ret = (tmp2 != 0xffff) && push(ds, tmp2);
        break;
    case 0xff06:
        tmp1 = pop(ds);
        tmp2 = pop(ds);
        push(ds, tmp1);
        ret = (tmp2 != 0xffff) && push(ds, tmp2);
        break;
    case 0xff07:
        tmp2 = pop(ds);
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1 + tmp2);
        break;
    case 0xff08:
        tmp2 = pop(ds);
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1 - tmp2);
        break;
    case 0xff09:
        tmp2 = pop(ds);
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1 & tmp2);
        break;
    case 0xff0a:
        tmp2 = pop(ds);
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1 | tmp2);
        break;
    case 0xff0b:
        tmp2 = pop(ds);
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(ds, tmp1 ^ tmp2);
        break;
    case 0xff0c:
        tmp1 = pop(ds);
        ret = tmp1 != 0xffff;
        if (tmp1 == 0) {
            fpc = getMem(fpc + 1);
            s = 0xf;
        }
        break;
    case 0xff0d:
        ret = push(rs, fpc + 2);
        fpc = getMem(fpc + 1);
        s = 0xf;
        break;
    case 0xff0e:
        fpc = pop(rs);
        ret = fpc != 0xffff;
        break;
    case 0xff0f:
        fhalt = true;
        ret = true;
        break;
    case 0xff10:
        tmp1 = pop(ds);
        ret = (tmp1 != 0xffff) && push(rs, tmp1);
        break;
    case 0xff11:
        tmp1 = pop(rs);
        ret = (tmp1 != 0xffff) && push(ds, tmp1);
        break;
    default:
       nop = true;
    }
    if (ret) {
        fpc += opGetPci(s);
    } else {
        if (!nop) {
            lastError = E005;
        } else {
            char buff[255];
            sprintf(buff, "%x", static_cast<unsigned char>(s));
            string a = buff;
            if (a.length() == 1) a = string(1, '0').append(a);
            sprintf(buff, E007, strToUpper(a).c_str());
            lastError = buff;
        }
        lastErrorAddr = fpc;
    }
    if (fpc >= memSize) {
        ret = false;
        lastError = E006;
        lastErrorAddr = fpc;
    }
    return ret;
}

bool StackCPU::compile() {
    lineReconstruct();
    if (!preprocessing()) return false;
    if (!processing()) return false;
    return true;
}

void StackCPU::clearStack() {
    ds->clear();
    rs->clear();
    fpc = 0;
    fhalt = false;
    delete[] mem;
    mem = new int[memSize];
    memcpy(mem, ftmem, memSize * sizeof(int));
}

bool StackCPU::run() {
    while (!fhalt) {
        if (!step()) return false;
    }
    return true;
}

bool StackCPU::stepInto() {
    return step();
}

bool StackCPU::stepOver() {
    int t = mem[fpc] == 0xff0d ? 1 : 0;
    if (!step()) return false;
    while (!fhalt && t > 0) {
        if (mem[fpc] == 0xff0d) ++t;
        else if (mem[fpc] == 0xff0e) --t;
        if (!step()) return false;
    }
    return true;
}

void StackCPU::setLines(vector<string> l) {
    lines->clear();
    lines->insert(lines->end(), l.begin(), l.end());
}

vector<string> *StackCPU::getLines() const {
    return lines;
}

string StackCPU::error() const {
    return lastError;
}

int StackCPU::errorAddr() const {
    return lastErrorAddr;
}

int StackCPU::pc() const {
    return fpc;
}

bool StackCPU::halt() const {
    return fhalt;
}

vector<int> StackCPU::dataStack() const {
    vector<int> lst;
    for (int &x : (*ds)) lst.push_back(x);
    return lst;
}

vector<int> StackCPU::returnStack() const {
    vector<int> lst;
    for (int &x : (*rs)) lst.push_back(x);
    return lst;
}

int StackCPU::getMemSize() const {
    return memSize;
}

void StackCPU::setMemSize(int val) {
    memSize = val;
}

int StackCPU::memory(int i) const {
    return mem[i];
}
