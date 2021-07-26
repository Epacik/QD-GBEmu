///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul  9 2021)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/dataview.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class Registers
///////////////////////////////////////////////////////////////////////////////
class Registers : public wxFrame
{
	private:

	protected:
		wxStaticText* AccumulatorValue;
		wxStaticText* FlagsValue;
		wxStaticText* BValue;
		wxStaticText* CValue;
		wxStaticText* DValue;
		wxStaticText* EValue;
		wxStaticText* HValue;
		wxStaticText* LValue;
		wxStaticText* SPValue;
		wxStaticText* PCValue;

	public:

		Registers( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Registers"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 200,347 ), long style = wxCAPTION|wxCLOSE_BOX|wxMINIMIZE_BOX|wxTAB_TRAVERSAL );

		~Registers();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Memory
///////////////////////////////////////////////////////////////////////////////
class Memory : public wxFrame
{
	private:

	protected:
		wxDataViewListCtrl* m_dataViewListCtrl1;

	public:

		Memory( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Memory();

};

