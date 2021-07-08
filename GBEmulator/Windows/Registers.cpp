//
// Created by epat on 08.07.2021.
//

#include "Registers.h"
#include "../global.h"

namespace Windows{
    wxBEGIN_EVENT_TABLE(Registers, wxFrame)
    wxEND_EVENT_TABLE()

    Registers::Registers(const wxPoint &pos)
            : wxFrame(nullptr, wxID_ANY, "Registers", pos, wxSize(200, 200)) {
        EnableMaximizeButton(false);
//        SetMinSize(wxSize(199.999999999, 199.999999999));
//        SetMaxSize(wxSize(200, 200));


        // Create a top-level panel to hold all the contents of the frame
        wxPanel* panel = new wxPanel(this, wxID_ANY);

        // Create the wxStaticText control
        wxStaticText* staticText = new wxStaticText(panel, wxID_ANY, "Static Text");

        // Set up the sizer for the panel
        wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);
        panelSizer->Add(staticText, 1, wxEXPAND);
        panel->SetSizer(panelSizer);

        // Set up the sizer for the frame and resize the frame
        // according to its contents
        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
        topSizer->SetMinSize(215, 50);
        topSizer->Add(panel, 1, wxEXPAND);
        SetSizerAndFit(topSizer);


        SetFocus();
    }
}