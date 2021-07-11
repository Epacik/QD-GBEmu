//
// Created by epat on 10.07.2021.
//

#ifndef GBEMU_TOOLS_H
#define GBEMU_TOOLS_H


#include <wx/string.h>

namespace Tools{
    namespace BitString {
        wxString GetBinaryString(uint16_t val);

        wxString GetBinaryString(uint8_t val);

        wxString GetHexString(uint16_t val);

        wxString GetHexString(uint8_t val);
    }
}


#endif //GBEMU_TOOLS_H
