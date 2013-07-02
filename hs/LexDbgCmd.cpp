
#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "LexDbgCmd.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

static inline bool IsDbgCmdStart(const int ch)
{
    if (ch == '>')
        return true;

    return false;
}

static void ColouriseDbgCmd(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
                               Accessor &styler)
{

    //WordList &Command = *keywordlists[0];
    //WordList &Parameter = *keywordlists[1];
    //WordList &Constant = *keywordlists[2];

    // Do not leak onto next line
    if (initStyle == SCE_DBGCMD_STRINGEOL)
        initStyle = SCE_DBGCMD_DEFAULT;

    StyleContext sc(startPos, length, initStyle, styler, 63); // 注意这个63，默认是31即0b11111，而我们的SCE_DBGCMD起点比较大

    for (; sc.More(); sc.Forward())
    {

        // Determine if the current state should terminate.
        if (sc.atLineEnd)
        {
            sc.SetState(SCE_DBGCMD_DEFAULT);
        }

        // Determine if a new state should be entered.
        if (sc.state == SCE_DBGCMD_DEFAULT)
        {
            if (sc.ch == '>')
            {
                sc.SetState(SCE_DBGCMD_CMD_CHAR);
                sc.ForwardSetState(SCE_DBGCMD_CMD);
            }
        }

    }
    sc.Complete();
}


static const char* const DbgCmdDesc[] = {"Command","Parameter", "Constant", 0};

LexerModule lmDbgCmd(SCLEX_DBGCMD, ColouriseDbgCmd, "WindbgCmd", 0, DbgCmdDesc);