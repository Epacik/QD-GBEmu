//
// Created by epat on 10.07.2021.
//

#include <bitset>
#include <ios>
#include <sstream>
#include <locale>

#include "Tools.h"


namespace Tools{
    namespace StringConverters{
        //converts uint16_t to wxString
        wxString GetBinaryString(uint16_t val) {
            auto bitStr = std::bitset<16>(val).to_string();
            wxString result(bitStr);
            return result;
        }

        wxString GetBinaryString(uint8_t val) {
            auto bitStr = std::bitset<8>(val).to_string();
            wxString result(bitStr);
            return result;
        }


        wxString GetHexString(uint16_t val) {
            std::stringstream stream;
            stream << std::hex << val;
            wxString result(stream.str());
            return result;
        }

        wxString GetHexString(uint8_t val) {
            std::stringstream stream;
            stream << std::hex << val;
            wxString result(stream.str());
            return result;
        }
    }
}