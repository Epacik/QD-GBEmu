//
// Created by epat on 10.07.2021.
//

#ifndef GBEMU_TOOLS_H
#define GBEMU_TOOLS_H


#include <wx/string.h>
#include "Application.h"

class Application;

namespace Tools{

    // Converters Data -> wxString
    namespace StringConverters {

        //Converts uint16_t to wxString binary value
        wxString GetBinaryString(uint16_t val);

        //Converts uint8_t to wxString binary value
        wxString GetBinaryString(uint8_t val);

        //Converts uint16_t to wxString hex value
        wxString GetHexString(uint16_t val);

        //Converts uint8_t to wxString hex value
        wxString GetHexString(uint8_t val);
    }
}


#endif //GBEMU_TOOLS_H
