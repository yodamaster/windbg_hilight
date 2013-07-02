#include "stdafx.h"
#include <windows.h>
#include "detours.h"
#include "engextcpp.hpp" // for g_ExtDllMain

#include <assert.h>
#include <string.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "LexAccessor.h"
#include "Accessor.h"
#include "LexerModule.h"

#include "..\SciLexer\src\Catalogue.h"
#include "lex.h"
#include "Config.h"
#include "LexAsm.h"
#include "LexDbgCmd.h"

#ifdef _WIN64
#pragma comment(lib, "Detours_x64.lib")
#else
#pragma comment(lib, "Detours_x86.lib")
#endif

//typedef BOOL (WINAPI *PEXT_DLL_MAIN)
//(HANDLE Instance, ULONG Reason, PVOID Reserved);

EXTTEXTOUTW ExtTextOutW_Org = ExtTextOutW;

BOOL WINAPI ExtTextOutW_Hook(  HDC hdc,          // handle to DC
                int X,            // x-coordinate of reference point
                int Y,            // y-coordinate of reference point
                UINT fuOptions,   // text-output options
                CONST RECT* lprc, // optional dimensions  
                LPCWSTR lpString, // string
                UINT cbCount,     // number of characters in string
                CONST INT* lpDx   // array of spacing values
                )
{
    if (cbCount <= 4 || fuOptions != 4)
    {
        return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
    }
    if (cbCount && (*lpString < 0x20 || *lpString > 0x7f))
    {
        return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
    }
    if (GetTextColor(hdc) == 0x0FFFFFF)
    {
        return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);

    }
    else
    {
        HWND hWnd = WindowFromDC(hdc);

        if (!hWnd) // 如果是0就没法加入到cache中去
        {
            return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
        }

        int wndclass = CConfig_Single.WhichWnd(hWnd); // 是否在窗口句柄缓存中
        if (wndclass == 0) // 没有处理过
        {
            HWND hParent = GetParent(hWnd);
            wndclass = WND_OTHER; // other
            do 
            {
                char sname[100];
                GetClassNameA(hParent, sname, sizeof(sname));
                if (0 == _strnicmp(sname, "WinBaseClass", 12))
                {
                    GetWindowTextA(hParent, sname, sizeof(sname));
                    if (0 == _strnicmp(sname, "Disassembly", 11)) // 通常是 Disassembly - WinDbg:6.12.0002.633 X86 
                    {
                        wndclass = WND_ASM;
                        break;
                    }
                    else if(0 == _strnicmp(sname, "Command", 7))
                    {
                        wndclass = WND_COMMAND;
                        break;
                    }
                }
                hParent = GetParent(hParent);
            } while (hParent);

            CConfig_Single.AddWndCache(hWnd, wndclass); // 添加到缓存中
        }

        if (WND_OTHER == wndclass)
        {
            return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
        }

        // Disassembly窗口
        if (WND_ASM == wndclass)
        {
            const LexerModule *lex = Catalogue::Find("asm");
            if (lex)
            {
                ILexer* p = lex->Create();

                // 添加关键字
                for(int i = 0; i< 7; ++i)
                {
                    p->WordListSet(i, CConfig_Single.get_keywords(i));
                }

                CText::hook_data hd;
                hd.hdc = hdc;
                hd.X = X;
                hd.Y = Y;
                hd.fuOptions = fuOptions;
                hd.lprc = lprc;
                hd.lpString = lpString;
                hd.cbCount = cbCount;
                hd.lpDx = lpDx;
                hd.ExtTextOutW_Org = ExtTextOutW_Org;

                class CText c(&hd);

                // 该函数在SetStyles中自己显示
                p->Lex(0, cbCount, SCE_ASM_DEFAULT, &c);
                return TRUE;
            }
            else
                return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
        }

        // Command窗口
        if (WND_COMMAND == wndclass)
        {
            const LexerModule *lex = Catalogue::Find("WindbgCmd");
            if (lex)
            {
                ILexer* p = lex->Create();

                //// 添加关键字
                //for(int i = 0; i< 6; ++i)
                //{
                //    p->WordListSet(i, CConfig_Single.get_keywords(i));
                //}

                CText::hook_data hd;
                hd.hdc = hdc;
                hd.X = X;
                hd.Y = Y;
                hd.fuOptions = fuOptions;
                hd.lprc = lprc;
                hd.lpString = lpString;
                hd.cbCount = cbCount;
                hd.lpDx = lpDx;
                hd.ExtTextOutW_Org = ExtTextOutW_Org;

                class CText c(&hd);

                // 该函数在SetStyles中自己显示
                p->Lex(0, cbCount, SCE_DBGCMD_DEFAULT, &c);
                return TRUE;
            }
            else
                return ExtTextOutW_Org(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
        }
    }
    return FALSE;
}

BOOL WINAPI
MyDllMain(HANDLE Instance, ULONG Reason, PVOID Reserved)
{
    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HMODULE)Instance);
        //ExtExtension::s_Module = (HMODULE)Instance;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)ExtTextOutW_Org, ExtTextOutW_Hook);
        DetourTransactionCommit();
        break;
    }
    return TRUE;
}

// 既然使用类，那么会在DllMain之类对类进行初始化，所以不必担心
ExtSetDllMain a(MyDllMain); // 这个很有技巧！engextcpp.cpp中定义了g_ExtDllMain，但是又想别的地方定义它
