//
// Created by epat on 08.07.2021.
//

#ifndef GBEMU_REGISTERS_H
#define GBEMU_REGISTERS_H

#include "../Application.h"
#include "../Emu/GbCpu.h"

#include "../Tools.h"
#include "../global.h"

namespace Windows {
    class Registers : public wxFrame {
    public:
        Registers(const wxPoint &pos);
        ~Registers();


        void UpdateValues();

    private:

        wxString GetFlagsString(uint8_t val);

        Application* app;

    protected:
        wxStaticText* AccumulatorValue;
        wxStaticText* FlagsValue;
        wxStaticText* BValue;
        wxStaticText* CValue;
        wxStaticText* DValue;
        wxStaticText* EValue;
        wxStaticText* HValue;
        wxStaticText* LValue;
        wxStaticText* SPValue;
        wxStaticText* PCValue;


       //wxDECLARE_EVENT_TABLE();
    };

}


#endif //GBEMU_REGISTERS_H
