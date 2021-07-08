//
// Created by epat on 07.07.2021.
//

#include "Application.h"
#include "Windows/MainWindow.h"
bool Application::OnInit()
{

    MainWindow = new Windows::Main( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    MainWindow->Show( true );

    EmulatorBus = std::make_unique<Emulator::Bus>();
    return true;
}

wxIMPLEMENT_APP(Application);