//
// Created by epat on 07.07.2021.
//

#ifndef GBEMU_APPLICATION_H
#define GBEMU_APPLICATION_H

#include <wx/app.h>

#include "global.h"
#include "Windows/Windows.h"
#include "Emu/GbBus.h"

class Application: public wxApp
{
public:
    virtual bool OnInit();

    Windows::Main      * MainWindow      = nullptr;
    Windows::Registers * RegistersWindow = nullptr;
    Windows::Memory    * MemoryWindow    = nullptr;

    std::unique_ptr<Emulator::GbBus> EmulatorBus;

    
    
};

Application &GetApp();
#endif //GBEMU_APPLICATION_H
