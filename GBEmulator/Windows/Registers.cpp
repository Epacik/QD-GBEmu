//
// Created by epat on 08.07.2021.
//

#include "Registers.h"
#include "../global.h"
#include "../Application.h"
#include "../Tools.h"
#include "../Emu/CpuRegisters.h"

namespace Windows{
    bool init = true;
    Registers::Registers(const wxPoint &pos)
            : wxFrame(nullptr, wxID_ANY, "Registers", pos, wxSize(200, 400), ToolWindowStyle) {

        Initialize();
    }

    

    Registers::~Registers() {
        GetApp().RegistersWindow = nullptr;
    }


    // values used to limit updates
    std::chrono::time_point<std::chrono::system_clock> UpdatedOn = std::chrono::system_clock::now();
    std::chrono::milliseconds UpdateDelta(100);

    // Get values of registers and displays them in a window
    void Registers::UpdateValues() {
        if (init)
            return;

        // limit updates so they won't use a half of a CPU
        // updates should occur at most once every 100ms or so
        auto now = std::chrono::system_clock::now();
        if(now - UpdatedOn < UpdateDelta){
            return;
        }
        UpdatedOn = now;

        //some binary to string converters
        using namespace Tools::StringConverters;
        

        //Accumulator
        AccumulatorValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.Accumulator));

        FlagsValue->SetLabel(GetFlagsString
        (GetApp().EmulatorBus->Cpu->Registers.FlagsState));

        BValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.B));

        CValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.C));

        DValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.D));

        EValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.E));

        HValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.H));

        LValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.L));

        SPValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.SP) + " [0x" + GetHexString(GetApp().EmulatorBus->Cpu->Registers.SP) + "]");

        PCValue->SetLabel(GetBinaryString(GetApp().EmulatorBus->Cpu->Registers.PC) + " [0x" + GetHexString(GetApp().EmulatorBus->Cpu->Registers.PC) + "]");
    }


    wxString Registers::GetFlagsString(uint8_t val) {
        std::string str;

        str += (val & 1 << 7) > 0 ? "Z" : "-";
        str += (val & 1 << 6) > 0 ? "N" : "-";
        str += (val & 1 << 5) > 0 ? "H" : "-";
        str += (val & 1 << 4) > 0 ? "C" : "-";

        wxString result(str);
        return result;
    }


    void Registers::Initialize()
    {
        app = &GetApp();
        GetApp().RegistersWindow = this;
        this->SetBackgroundColour(wxColour(*wxWHITE));

        this->SetSizeHints(wxDefaultSize, wxDefaultSize);

        wxBoxSizer* MainSizer;
        MainSizer = new wxBoxSizer(wxVERTICAL);

        wxGridSizer* gSizer1;
        gSizer1 = new wxGridSizer(0, 2, 5, 5);

        wxStaticBoxSizer* AFBC_Grid;
        AFBC_Grid = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Accumulator")), wxVERTICAL);

        AccumulatorValue = new wxStaticText(AFBC_Grid->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        AccumulatorValue->Wrap(-1);
        AFBC_Grid->Add(AccumulatorValue, 0, wxALL, 5);


        gSizer1->Add(AFBC_Grid, 1, wxEXPAND, 5);

        wxStaticBoxSizer* Flags;
        Flags = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Flags")), wxVERTICAL);

        FlagsValue = new wxStaticText(Flags->GetStaticBox(), wxID_ANY, wxT("Z-HC----"), wxDefaultPosition, wxDefaultSize, 0);
        FlagsValue->Wrap(-1);
        Flags->Add(FlagsValue, 0, wxALL, 5);


        gSizer1->Add(Flags, 1, wxEXPAND, 5);

        wxStaticBoxSizer* B;
        B = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("B")), wxVERTICAL);

        BValue = new wxStaticText(B->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        BValue->Wrap(-1);
        B->Add(BValue, 0, wxALL, 5);


        gSizer1->Add(B, 1, wxEXPAND, 5);

        wxStaticBoxSizer* C;
        C = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("C")), wxVERTICAL);

        CValue = new wxStaticText(C->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        CValue->Wrap(-1);
        C->Add(CValue, 0, wxALL, 5);


        gSizer1->Add(C, 1, wxEXPAND, 5);


        MainSizer->Add(gSizer1, 1, wxEXPAND, 5);

        wxGridSizer* DEHL_Grid;
        DEHL_Grid = new wxGridSizer(0, 2, 5, 5);

        wxStaticBoxSizer* D;
        D = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("D")), wxVERTICAL);

        DValue = new wxStaticText(D->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        DValue->Wrap(-1);
        D->Add(DValue, 0, wxALL, 5);


        DEHL_Grid->Add(D, 1, wxEXPAND, 5);

        wxStaticBoxSizer* E;
        E = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("E")), wxVERTICAL);

        EValue = new wxStaticText(E->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        EValue->Wrap(-1);
        E->Add(EValue, 0, wxALL, 5);


        DEHL_Grid->Add(E, 1, wxEXPAND, 5);

        wxStaticBoxSizer* H;
        H = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("H")), wxVERTICAL);

        HValue = new wxStaticText(H->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        HValue->Wrap(-1);
        H->Add(HValue, 0, wxALL, 5);


        DEHL_Grid->Add(H, 1, wxEXPAND, 5);

        wxStaticBoxSizer* L;
        L = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("L")), wxVERTICAL);

        LValue = new wxStaticText(L->GetStaticBox(), wxID_ANY, wxT("10101010"), wxDefaultPosition, wxDefaultSize, 0);
        LValue->Wrap(-1);
        L->Add(LValue, 0, wxALL, 5);


        DEHL_Grid->Add(L, 1, wxEXPAND, 5);


        MainSizer->Add(DEHL_Grid, 1, wxEXPAND, 5);

        wxBoxSizer* SPPC_Box;
        SPPC_Box = new wxBoxSizer(wxVERTICAL);

        wxStaticBoxSizer* SP;
        SP = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Stack Pointer")), wxVERTICAL);

        SPValue = new wxStaticText(SP->GetStaticBox(), wxID_ANY, wxT("1010101010101010"), wxDefaultPosition, wxDefaultSize, 0);
        SPValue->Wrap(-1);
        SP->Add(SPValue, 0, wxALL, 5);


        SPPC_Box->Add(SP, 1, wxEXPAND, 5);

        wxStaticBoxSizer* PC;
        PC = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Program Counter")), wxVERTICAL);

        PCValue = new wxStaticText(PC->GetStaticBox(), wxID_ANY, wxT("1010101010101010"), wxDefaultPosition, wxDefaultSize, 0);
        PCValue->Wrap(-1);
        PC->Add(PCValue, 0, wxALL, 5);


        SPPC_Box->Add(PC, 1, wxEXPAND, 5);


        MainSizer->Add(SPPC_Box, 1, wxEXPAND, 5);


        this->SetSizer(MainSizer);
        this->Layout();

        this->Centre(wxBOTH);

        SetFocus();

        //UpdateValues();
        init = false;
    }
}