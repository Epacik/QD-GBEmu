//
// Created by epat on 08.07.2021.
//

#ifndef GBEMU_REGISTERS_H
#define GBEMU_REGISTERS_H

#include "../Application.h"
#include "../Emu/GbCpu.h"

#include "../global.h"
namespace Windows {
    class Registers : public wxFrame {
    public:
        Registers(const wxPoint &pos);

    private:

    wxDECLARE_EVENT_TABLE();
    };

}


#endif //GBEMU_REGISTERS_H
