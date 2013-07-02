#include "stdafx.h"

#include <assert.h>
#include <string.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "LexAccessor.h"
#include "Accessor.h"
#include "LexerModule.h"

#include "Platform.h"
#include "..\SciLexer\src\Catalogue.h"
#include "lex.h"
#include <vector>
#include "Config.h"

int SCI_METHOD CText::Version() const
{
    return 0;
}

void SCI_METHOD CText::SetErrorStatus( int status )
{

}

int SCI_METHOD CText::Length() const
{
    //return m_cbCount;
    return str_.size();
}

void SCI_METHOD CText::GetCharRange( char *buffer, int position, int lengthRetrieve ) const
{
    //memcpy(buffer, (unsigned char*)m_lpString + position, lengthRetrieve);
    memcpy(buffer, str_.c_str() + position, lengthRetrieve);
}

char SCI_METHOD CText::StyleAt( int position ) const
{
    return 'a';
}

int SCI_METHOD CText::LineFromPosition( int position ) const
{
    if (0 == position)
    {
        return 0;
    }
    do
    {
        // TMD，windbg只有0X0D
        if('\r' == *(str_.c_str()+ position - 1))
        //if (L'\r' == *(m_lpString + position - 1))
        {
            return position;
            break;
        }
        --position;
    }
    while(position != 0);
    
    return 0;
}

int SCI_METHOD CText::LineStart( int line ) const
{
    do
    {
        // TMD，windbg只有0X0D
        if(L'\r' == *(str_.c_str() + line))
        //if (L'\r' == *(m_lpString + line))
        {
            return line;
            break;
        }
        ++line;
    }
    while((UINT)line < str_.size()/*m_cbCount*/);

    return line;
}

int SCI_METHOD CText::GetLevel( int line ) const
{
    return 0;
}

int SCI_METHOD CText::SetLevel( int line, int level )
{
    return 0;
}

int SCI_METHOD CText::GetLineState( int line ) const
{
    return 0;
}

int SCI_METHOD CText::SetLineState( int line, int state )
{
    return 0;
}

void SCI_METHOD CText::StartStyling( int position, char mask )
{

}

bool SCI_METHOD CText::SetStyleFor( int length, char style )
{
    return true;
}

bool SCI_METHOD CText::SetStyles( int length, const char *styles )
{
    const char* pForward = styles;
    const char* pPrev = styles;

    for (; (pForward - styles) <= length; ++pForward)
    {
        if ((*pForward != *pPrev)
            || ((pForward - styles) == length))
        {
            LPCWSTR lpString = m_hd.lpString + (pPrev - styles);
            UINT cbCount = pForward - pPrev;

            COLORREF old = SetTextColor(m_hd.hdc, CConfig_Single.get_color((unsigned int)*pPrev));
            m_hd.ExtTextOutW_Org(m_hd.hdc, m_hd.X, m_hd.Y, m_hd.fuOptions, m_hd.lprc
                , lpString
                , cbCount
                , m_hd.lpDx);
            SIZE s;
            GetTextExtentPoint32W(m_hd.hdc, lpString, cbCount, &s);
            m_hd.X += s.cx; // 经测试，每次只输出一行
            SetTextColor(m_hd.hdc, old);

            pPrev = pForward;
        }
    }

    return true;
}

void SCI_METHOD CText::DecorationSetCurrentIndicator( int indicator )
{

}

void SCI_METHOD CText::DecorationFillRange( int position, int value, int fillLength )
{

}

void SCI_METHOD CText::ChangeLexerState( int start, int end )
{

}

// 经测试是unicode，即utf-16le
int SCI_METHOD CText::CodePage() const
{
    // ExtTextOutW是unicode
    return 0; // LexAccessor构造函数
}

bool SCI_METHOD CText::IsDBCSLeadByte( char ch ) const
{
    return Platform::IsDBCSLeadByte(CodePage(), ch);
}

const char * SCI_METHOD CText::BufferPointer()
{
    return NULL;
}

int SCI_METHOD CText::GetLineIndentation( int line )
{
    return 0;
}

static std::string wstring2string(const std::wstring & rwString, UINT codepage)
{
    int len = WideCharToMultiByte(codepage, 0, rwString.c_str(), -1, NULL, 0, NULL, NULL);
    if(len > 0)
    {		
        std::vector<char> vw(len);
        WideCharToMultiByte(codepage, 0, rwString.c_str(), -1, &vw[0], len, NULL, NULL);
        return &vw[0];
    }
    else
        return "";
}

CText::CText(const hook_data* hd)
//: m_lpString(lpString)
//, m_cbCount(cbCount)
{
    m_hd = *hd;
    std::wstring wstr(m_hd.lpString, m_hd.lpString + m_hd.cbCount);
    str_ = wstring2string(wstr, CP_ACP);
}
