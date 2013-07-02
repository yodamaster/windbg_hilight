#include "Platform.h"

int Platform::Maximum(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

bool Platform::IsDBCSLeadByte(int codePage, char ch) {
    // Byte ranges found in Wikipedia articles with relevant search strings in each case
    unsigned char uch = static_cast<unsigned char>(ch);
    switch (codePage) {
    case 932:
        // Shift_jis
        return ((uch >= 0x81) && (uch <= 0x9F)) ||
            ((uch >= 0xE0) && (uch <= 0xEF));
    case 936:
        // GBK
        return (uch >= 0x81) && (uch <= 0xFE);
    case 949:
        // Korean Wansung KS C-5601-1987
        return (uch >= 0x81) && (uch <= 0xFE);
    case 950:
        // Big5
        return (uch >= 0x81) && (uch <= 0xFE);
    case 1361:
        // Korean Johab KS C-5601-1992
        return
            ((uch >= 0x84) && (uch <= 0xD3)) ||
            ((uch >= 0xD8) && (uch <= 0xDE)) ||
            ((uch >= 0xE0) && (uch <= 0xF9));
    }
    return false;
}