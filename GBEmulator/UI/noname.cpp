///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul  9 2021)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "noname.h"

///////////////////////////////////////////////////////////////////////////

Registers::Registers( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 5, 5 );

	wxStaticBoxSizer* AFBC_Grid;
	AFBC_Grid = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Accumulator") ), wxVERTICAL );

	AccumulatorValue = new wxStaticText( AFBC_Grid->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	AccumulatorValue->Wrap( -1 );
	AFBC_Grid->Add( AccumulatorValue, 0, wxALL, 5 );


	gSizer1->Add( AFBC_Grid, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* Flags;
	Flags = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Flags") ), wxVERTICAL );

	FlagsValue = new wxStaticText( Flags->GetStaticBox(), wxID_ANY, wxT("Z-HC----"), wxDefaultPosition, wxDefaultSize, 0 );
	FlagsValue->Wrap( -1 );
	Flags->Add( FlagsValue, 0, wxALL, 5 );


	gSizer1->Add( Flags, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* B;
	B = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("B") ), wxVERTICAL );

	BValue = new wxStaticText( B->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	BValue->Wrap( -1 );
	B->Add( BValue, 0, wxALL, 5 );


	gSizer1->Add( B, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* C;
	C = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("C") ), wxVERTICAL );

	CValue = new wxStaticText( C->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	CValue->Wrap( -1 );
	C->Add( CValue, 0, wxALL, 5 );


	gSizer1->Add( C, 1, wxEXPAND, 5 );


	MainSizer->Add( gSizer1, 1, wxEXPAND, 5 );

	wxGridSizer* DEHL_Grid;
	DEHL_Grid = new wxGridSizer( 0, 2, 0, 0 );

	wxStaticBoxSizer* D;
	D = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("D") ), wxVERTICAL );

	DValue = new wxStaticText( D->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	DValue->Wrap( -1 );
	D->Add( DValue, 0, wxALL, 5 );


	DEHL_Grid->Add( D, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* E;
	E = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("E") ), wxVERTICAL );

	EValue = new wxStaticText( E->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	EValue->Wrap( -1 );
	E->Add( EValue, 0, wxALL, 5 );


	DEHL_Grid->Add( E, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* H;
	H = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("H") ), wxVERTICAL );

	HValue = new wxStaticText( H->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	HValue->Wrap( -1 );
	H->Add( HValue, 0, wxALL, 5 );


	DEHL_Grid->Add( H, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* L;
	L = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("L") ), wxVERTICAL );

	LValue = new wxStaticText( L->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0 );
	LValue->Wrap( -1 );
	L->Add( LValue, 0, wxALL, 5 );


	DEHL_Grid->Add( L, 1, wxEXPAND, 5 );


	MainSizer->Add( DEHL_Grid, 1, wxEXPAND, 5 );

	wxBoxSizer* SPPC_Box;
	SPPC_Box = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* SP;
	SP = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Stack Pointer") ), wxVERTICAL );

	SPValue = new wxStaticText( SP->GetStaticBox(), wxID_ANY, wxT("1010101010101010"), wxDefaultPosition, wxDefaultSize, 0 );
	SPValue->Wrap( -1 );
	SP->Add( SPValue, 0, wxALL, 5 );


	SPPC_Box->Add( SP, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* PC;
	PC = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Program Counter") ), wxVERTICAL );

	PCValue = new wxStaticText( PC->GetStaticBox(), wxID_ANY, wxT("1010101010101010"), wxDefaultPosition, wxDefaultSize, 0 );
	PCValue->Wrap( -1 );
	PC->Add( PCValue, 0, wxALL, 5 );


	SPPC_Box->Add( PC, 1, wxEXPAND, 5 );


	MainSizer->Add( SPPC_Box, 1, wxEXPAND, 5 );


	this->SetSizer( MainSizer );
	this->Layout();

	this->Centre( wxBOTH );
}

Registers::~Registers()
{
}

Memory::Memory( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_dataViewListCtrl1 = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_dataViewListCtrl1, 1, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer3 );
	this->Layout();

	this->Centre( wxBOTH );
}

Memory::~Memory()
{
}
