//
// Created by epat on 07.07.2021.
//

#include "MainWindow.h"
#include "Registers.h"
#include "../Application.h"
#include "../global.h"


namespace Windows {
    /// <summary>
    /// Opcje w menu
    /// </summary>
    enum class MenuItems {
        Hello = 1,
        Registers = 1 << 1,
        Memory = 1 << 2,
    };

    /// <summary>
    /// Rejestracja eventów dla okna 
    /// </summary>
    wxBEGIN_EVENT_TABLE(Main, wxFrame)
        EVT_MENU((int)MenuItems::Hello,           Main::OnHello)
        EVT_MENU((int)MenuItems::Registers,       Main::OnOpenRegistersWindow)
        EVT_MENU((int)MenuItems::Memory,          Main::OnOpenMemoryWindow)
        EVT_MENU(wxID_EXIT,                       Main::OnExit)
        EVT_MENU(wxID_ABOUT,                      Main::OnAbout)

        EVT_CLOSE(Main::OnClose)
    wxEND_EVENT_TABLE()

    Main::Main(const wxString &title, const wxPoint &pos, const wxSize &size, Application *app)
            : wxFrame(nullptr, wxID_ANY, title, pos, size) {
        this->SetBackgroundColour(wxColour(*wxWHITE));
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

    /// <summary>
    /// Jeœli okno z widokiem rejestrów nie zosta³o otwarte otwiera nowe, jeœli zosta³o, pokazuje je u¿ytkownikowi
    /// </summary>
    /// <param name="event"></param>
    void Main::OnOpenRegistersWindow(wxCommandEvent &event) {
        // wxLogMessage("Opening registers window");

        if(App->RegistersWindow == nullptr){
            auto registersWindow = new Windows::Registers(wxPoint(50, 50));
            registersWindow->Show(true);
            return;
        }

        FocusWindow(App->RegistersWindow);
    }

    

    /// <summary>
    /// Jeœli okno z widokiem pamiêci nie zosta³o otwarte otwiera nowe, jeœli zosta³o, pokazuje je u¿ytkownikowi
    /// </summary>
    /// <param name="event"></param>
    void Main::OnOpenMemoryWindow(wxCommandEvent &event) {
        // wxLogMessage("Opening registers window");

        if(App->MemoryWindow == nullptr){
            auto memoryWindow = new Windows::Memory(wxPoint(50, 50));
            memoryWindow->Show(true);
            return;
        }

        FocusWindow(App->MemoryWindow);

    }

    void Windows::Main::FocusWindow(wxFrame* window)
    {
        window->SetFocus();
        window->Show(true);
    }

    /// <summary>
    /// Tworzenie paska menu i wype³nienie go opcjami
    /// </summary>
    void Main::CreateMenuBar() {
        // File
        auto fileMenu = new wxMenu;
        fileMenu->Append((int)MenuItems::Hello, "&Hello...\tCtrl-H",
                         "Help string shown in status bar for this menu item");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT);

         
        // Windows
        auto windowsMenu = new wxMenu;
        windowsMenu->Append((int)MenuItems::Registers, "Registers", "Displays current state of registers");
        windowsMenu->Append((int)MenuItems::Memory,    "Memory", "Displays current state of memory around ProgramCounter");

        // Help
        auto helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT);

        //MenuBar
        auto menuBar = new wxMenuBar;

        menuBar->Append(fileMenu,  "File");
        menuBar->Append(windowsMenu, "Windows");
        menuBar->Append(helpMenu,  "Help");

        SetMenuBar(menuBar);
        //CreateStatusBar();

    }


    /// <summary>
    /// Event wywo³ywany po klikniêciu przycisku zamkniêcia okna
    /// </summary>
    /// <param name="event"></param>
    void Main::OnClose(wxCloseEvent& event)
    {
        if (App->MemoryWindow != nullptr)
            App->MemoryWindow->Close();

        if (App->RegistersWindow != nullptr)
            App->RegistersWindow->Close();

        Destroy();
    }
}