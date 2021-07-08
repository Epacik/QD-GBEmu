//
// Created by epat on 07.07.2021.
//

#ifndef GBEMU_MAINWINDOW_H
#define GBEMU_MAINWINDOW_H

#include "../global.h"
namespace Windows {



    class Main : public wxFrame {
    public:
        Main(const wxString &title, const wxPoint &pos, const wxSize &size);

    private:
        void OnHello(wxCommandEvent &event);

        void OnExit(wxCommandEvent &event);

        void OnAbout(wxCommandEvent &event);

        void OnOpenRegistersWindow(wxCommandEvent &event);


        void CreateMenuBar();

    wxDECLARE_EVENT_TABLE();
    };

}
#endif //GBEMU_MAINWINDOW_H
