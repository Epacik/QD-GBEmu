//
// Created by epat on 07.07.2021.
//

#ifndef GBEMU_APPLICATION_H
#define GBEMU_APPLICATION_H


#include "global.h"
#include "Windows/MainWindow.h"
#include "Emu/Bus.h"

class Application: public wxApp
{
public:
    virtual bool OnInit();

    Windows::Main * MainWindow;

    std::unique_ptr<Emulator::Bus> EmulatorBus;


};

Application &GetApp();

#endif //GBEMU_APPLICATION_H
