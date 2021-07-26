//
// Created by epat on 07.07.2021.
//

#include "Application.h"
#include "Windows/MainWindow.h"
#include "Tools.h"

bool Application::OnInit()
{
    MainWindow = new Windows::Main("Hello World",
                                   wxPoint(50, 50),
                                   wxSize(450, 340),
                                   static_cast<Application *>(this));
    MainWindow->Show( true );

    EmulatorBus = std::make_unique<Emulator::GbBus>();
    EmulatorBus->SetOnRefreshUI(std::make_shared<std::function<void()>>([this](){
        if(RegistersWindow != nullptr){
            CallAfter([this]{
                RegistersWindow->UpdateValues();
            });
        }

    }));
    return true;
}

wxIMPLEMENT_APP(Application);

Application &GetApp(){
    return wxGetApp();
}
