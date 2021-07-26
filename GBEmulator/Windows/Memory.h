//
// Created by epat on 23.07.2021.
//

#ifndef GBEMU_MEMORY_H
#define GBEMU_MEMORY_H


#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/dataview.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/frame.h>

namespace Windows{
    class Memory : public wxFrame
    {
    private:

    protected:
        wxDataViewListCtrl* MemoryView;

    public:

        Memory( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

        ~Memory();

        void AddValue(uint8_t value);

    };
}

#endif //GBEMU_MEMORY_H
