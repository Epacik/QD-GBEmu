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

#include "../global.h"

namespace Windows{
    class Memory : public wxFrame
    {
    private:

    protected:
        wxDataViewListCtrl* MemoryView;

    public:

        Memory(const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = ToolWindowStyle | wxTAB_TRAVERSAL );

        void Initialize();

        ~Memory();

        void AddValue(uint8_t value);

    };
}

#endif //GBEMU_MEMORY_H
