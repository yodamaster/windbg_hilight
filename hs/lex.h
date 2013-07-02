#pragma once

#include <windows.h>
#include "ILexer.h"
#include <string>

typedef BOOL (WINAPI* EXTTEXTOUTW)(  HDC hdc,          // handle to DC
                                   int X,            // x-coordinate of reference point
                                   int Y,            // y-coordinate of reference point
                                   UINT fuOptions,   // text-output options
                                   CONST RECT* lprc, // optional dimensions  
                                   LPCWSTR lpString, // string
                                   UINT cbCount,     // number of characters in string
                                   CONST INT* lpDx   // array of spacing values
                                   );

// 该类只是参考类，该类并不支持unicode(utf-16)，其他utf8和gbk之类也许支持很好，但没测试过
class CText : public IDocument
{
public:
    struct hook_data
    {
        HDC hdc; 
        int X;
        int Y;
        UINT fuOptions;
        CONST RECT* lprc;
        LPCWSTR lpString;
        UINT cbCount;
        CONST INT* lpDx;
        EXTTEXTOUTW ExtTextOutW_Org;
    };

    CText(const hook_data* hd);
    virtual int SCI_METHOD Version() const;
    virtual void SCI_METHOD SetErrorStatus(int status);
    virtual int SCI_METHOD Length() const;
    // ！！获取文本
    virtual void SCI_METHOD GetCharRange(char *buffer, int position, int lengthRetrieve) const;
    virtual char SCI_METHOD StyleAt(int position) const;
    // 当前行的起始位置
    virtual int SCI_METHOD LineFromPosition(int position) const;
    // 下一行的位置
    virtual int SCI_METHOD LineStart(int line) const;
    virtual int SCI_METHOD GetLevel(int line) const;
    virtual int SCI_METHOD SetLevel(int line, int level);
    virtual int SCI_METHOD GetLineState(int line) const;
    virtual int SCI_METHOD SetLineState(int line, int state);
    // 开始渲染
    virtual void SCI_METHOD StartStyling(int position, char mask);
    virtual bool SCI_METHOD SetStyleFor(int length, char style);
    // ！！处理完毕，styles每字节对应一个文字，值就是应该显示的颜色
    virtual bool SCI_METHOD SetStyles(int length, const char *styles);
    virtual void SCI_METHOD DecorationSetCurrentIndicator(int indicator);
    virtual void SCI_METHOD DecorationFillRange(int position, int value, int fillLength);
    virtual void SCI_METHOD ChangeLexerState(int start, int end);
    virtual int SCI_METHOD CodePage() const;
    // 字节是否是开头的多字节字,gbk
    virtual bool SCI_METHOD IsDBCSLeadByte(char ch) const;
    virtual const char * SCI_METHOD BufferPointer();
    virtual int SCI_METHOD GetLineIndentation(int line);
private:
    std::string str_;
    //LPCWSTR m_lpString;
    //UINT m_cbCount;
    hook_data m_hd;
};