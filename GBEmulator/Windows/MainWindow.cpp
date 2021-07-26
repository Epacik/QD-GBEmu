//
// Created by epat on 07.07.2021.
//

#include "MainWindow.h"
#include "Registers.h"
#include "../Application.h"
#include "../global.h"


namespace Windows {

    enum class MenuItems {
        Hello = 1,
        Registers = 1 << 1,
        Memory = 1 << 2,
    };

    wxBEGIN_EVENT_TABLE(Main, wxFrame)
        EVT_MENU((int)MenuItems::Hello,     Main::OnHello)
        EVT_MENU((int)MenuItems::Registers, Main::OnOpenRegistersWindow)
        EVT_MENU((int)MenuItems::Memory, Main::OnOpenRegistersWindow)
        EVT_MENU(wxID_EXIT,                       Main::OnExit)
        EVT_MENU(wxID_ABOUT,                      Main::OnAbout)
    wxEND_EVENT_TABLE()

    Main::Main(const wxString &title, const wxPoint &pos, const wxSize &size, Application *app)
            : wxFrame(nullptr, wxID_ANY, title, pos, size) {

        this->App = app;
        CreateMenuBar();

        CreateStatusBar();
        SetStatusText("Welcome to wxWidgets!");
    }

    void Main::OnExit(wxCommandEvent &event) {
        Close(true);
    }

    void Main::OnAbout(wxCommandEvent &event) {
        wxMessageBox("This is a wxWidgets' Hello world sample",
                     "About Hello World", wxOK | wxICON_INFORMATION);
    }

    void Main::OnHello(wxCommandEvent &event) {
        wxLogMessage("Hello world from wxWidgets!");
    }

    void Main::OnOpenRegistersWindow(wxCommandEvent &event) {
        // wxLogMessage("Opening registers window");

        if(App->RegistersWindow == nullptr){
            auto regWindow = new Windows::Registers(wxPoint(50, 50));
            regWindow->Show(true);
            return;
        }

        App->RegistersWindow->SetFocus();
        App->RegistersWindow->Show(true);

    }

    void Main::OnOpenMemoryWindow(wxCommandEvent &event) {
        // wxLogMessage("Opening registers window");

        if(App->MemoryWindow == nullptr){
            auto regWindow = new Windows::Registers(wxPoint(50, 50));
            regWindow->Show(true);
            return;
        }

        App->MemoryWindow->SetFocus();
        App->MemoryWindow->Show(true);

    }

    void Main::CreateMenuBar() {
        // File
        auto fileMenu = new wxMenu;
        fileMenu->Append((int)MenuItems::Hello, "&Hello...\tCtrl-H",
                         "Help string shown in status bar for this menu item");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT);

         
        // Windows
        auto debugMenu = new wxMenu;
        debugMenu->Append((int)MenuItems::Registers, "Registers", "Displays current state of registers");
        debugMenu->Append((int)MenuItems::Registers, "Registers", "Displays current state of registers");

        // Help
        auto helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT);

        //MenuBar
        auto menuBar = new wxMenuBar;

        menuBar->Append(fileMenu,  "File");
        menuBar->Append(debugMenu, "Debug");
        menuBar->Append(helpMenu,  "Help");

        SetMenuBar(menuBar);
        //CreateStatusBar();

    }
}