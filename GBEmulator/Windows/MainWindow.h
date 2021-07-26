//
// Created by epat on 07.07.2021.
//

#ifndef GBEMU_MAINWINDOW_H
#define GBEMU_MAINWINDOW_H

#include <wx/app.h>
#include "../Application.h"
#include "../global.h"
#include "../Tools.h"

class Application;

namespace Windows {

    class Main : public wxFrame {
    public:
        Main(const wxString &title, const wxPoint &pos, const wxSize &size, Application *app);

    private:
        void OnHello(wxCommandEvent &event);

        void OnExit(wxCommandEvent &event);

        void OnAbout(wxCommandEvent &event);

        void OnOpenRegistersWindow(wxCommandEvent &event);

        void OnOpenMemoryWindow(wxCommandEvent &event);

        void CreateMenuBar();

        Application * App;

    wxDECLARE_EVENT_TABLE();
    };

}
#endif //GBEMU_MAINWINDOW_H
