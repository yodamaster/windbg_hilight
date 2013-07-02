// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\SciLexer.lib")
#else
#pragma comment(lib, "..\\Release\\SciLexer.lib")
#endif

#include <assert.h>
#include <string.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "LexAccessor.h"
#include "Accessor.h"
#include "LexerModule.h"

#include "..\SciLexer\src\Catalogue.h"

#define TEST_STR "text:1000 mov eax, ecx; mov ecx, edx;"
class Doc : public IDocument
{
public:
    virtual int SCI_METHOD Version() const
    {
        return 0;
    }
    virtual void SCI_METHOD SetErrorStatus(int status)
    {

    }
    virtual int SCI_METHOD Length() const
    {
        return strlen(TEST_STR);
    }
    // ！！获取文本
    virtual void SCI_METHOD GetCharRange(char *buffer, int position, int lengthRetrieve) const
    {
        strcpy(buffer, 
            TEST_STR
            );
    }
    virtual char SCI_METHOD StyleAt(int position) const
    {
        return 'a';
    }
    // 当前行的起始位置
    virtual int SCI_METHOD LineFromPosition(int position) const
    {
        return 0;
    }
    // 下一行的位置
    virtual int SCI_METHOD LineStart(int line) const 
    {
        return line + 10;
    }
    virtual int SCI_METHOD GetLevel(int line) const
    {
        return 0;

    }
    virtual int SCI_METHOD SetLevel(int line, int level)
    {
        return 0;

    }
    virtual int SCI_METHOD GetLineState(int line) const
    {
        return 0;
    }
    virtual int SCI_METHOD SetLineState(int line, int state)
    {
        return 0;

    }
    // 开始渲染
    virtual void SCI_METHOD StartStyling(int position, char mask)
    {

    }
    virtual bool SCI_METHOD SetStyleFor(int length, char style) 
    {
        return true;
    }
    // ！！处理完毕，styles每字节对应一个文字，值就是应该显示的颜色
    virtual bool SCI_METHOD SetStyles(int length, const char *styles) 
    {
        return true;
    }
    virtual void SCI_METHOD DecorationSetCurrentIndicator(int indicator)
    {
    }
    virtual void SCI_METHOD DecorationFillRange(int position, int value, int fillLength) 
    {

    }
    virtual void SCI_METHOD ChangeLexerState(int start, int end)
    {

    }
    virtual int SCI_METHOD CodePage() const 
    {
        // ExtTextOutW
        return 0; // LexAccessor构造函数
    }

    // 字节是否是开头的多字节字,gbk
    virtual bool SCI_METHOD IsDBCSLeadByte(char ch) const 
    {
        return true;
    }
    virtual const char * SCI_METHOD BufferPointer() 
    {
        return NULL;
    }
    virtual int SCI_METHOD GetLineIndentation(int line)
    {
        return 0;
    }
};
int _tmain(int argc, _TCHAR* argv[])
{
    const LexerModule *lex = Catalogue::Find("asm");
    if (lex)
    {
        ILexer* p = lex->Create();
        class Doc c;
        p->Lex(0, 20, 0, &c);

    }
	return 0;
}

