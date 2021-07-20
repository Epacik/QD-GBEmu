//
// Created by epat on 14.07.2021.
//

#ifndef GBEMU_INSTRUCTION_H
#define GBEMU_INSTRUCTION_H


#include <wx/string.h>
#include <functional>

namespace Emulator{
    struct Opcode {
    public:
        wxString Name = "";
        std::function<void()> Exec;
        std::function<wxString()> FormatText = [this]{ return Name; };
    };
}


#endif //GBEMU_INSTRUCTION_H
