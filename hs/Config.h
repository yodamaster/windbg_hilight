#pragma once

#include <windows.h>
#include <string>
#include <map>



#define WND_OTHER 10
#define WND_ASM 1
#define WND_COMMAND 2

class CConfig
{
    typedef std::map<unsigned int, COLORREF> MAP_COLOR;
    typedef std::map<int, std::string> MAP_KEYWORDS;
    typedef std::map<HWND, int> MAP_WND;

    MAP_COLOR m_color;
    MAP_KEYWORDS m_keywords;
    MAP_WND m_hwnd;

public:
    CConfig();
    static CConfig& get_instance()
    {
        static CConfig c;
        return c;
    }
    COLORREF get_color(unsigned int index);
    const char* get_keywords(int index);
    bool load(const std::string &filename);
    void save(const std::string &filename);

    void AddWndCache(HWND hWnd, int which);
    // »˝Ã¨£¨0£∫Œ¥…Ë÷√£¨ 1:Disassembly£¨ 2:Command
    int WhichWnd(HWND hWnd);

protected:
    void DelWnd(HWND);
    static LRESULT CALLBACK
        _AfxActivationWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
};

#define CConfig_Single (CConfig::get_instance())