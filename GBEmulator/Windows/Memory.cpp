//
// Created by epat on 23.07.2021.
//

#include "Memory.h"
#include "../Tools.h"
namespace Windows{
    Memory::Memory( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
    {
        GetApp().MemoryWindow = this;
        this->SetSizeHints( wxDefaultSize, wxDefaultSize );

        wxBoxSizer* bSizer3;
        bSizer3 = new wxBoxSizer( wxVERTICAL );

        MemoryView = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
        bSizer3->Add( MemoryView, 1, wxALL|wxEXPAND, 5 );
        MemoryView->AppendTextColumn( "Text" );

        this->SetSizer( bSizer3 );
        this->Layout();

        this->Centre( wxBOTH );
    }

    Memory::~Memory()
    {
        GetApp().MemoryWindow = nullptr;
    }

    void Memory::AddValue(uint8_t d) {
        wxVector<wxVariant> data;
        data.push_back( wxVariant(Tools::StringConverters::GetHexString(d)) );
        MemoryView->AppendItem(data);
    }
}