#include "StdAfx.h"
#include "Config.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "SciLexer.h"
#include "LexAsm.h"
#include <tchar.h>
#include <assert.h>
#include "LexDbgCmd.h"
#define FILE_NAME "asm.ini"

bool CConfig::load( const std::string &filename )
{
    using boost::property_tree::ptree;
    ptree pt;

    bool bExsit = true;
    try
    {
        read_ini(filename, pt);
    }catch(...)
    {
        bExsit = false;
    }

#define READ_VALUE(x, default_value) \
    m_color[SCE_##x] = pt.get<COLORREF>("color."#x, default_value);

    READ_VALUE(ASM_DEFAULT, 0x0);
    READ_VALUE(ASM_COMMENT, 65280);
    READ_VALUE(ASM_NUMBER, 0);
    READ_VALUE(ASM_STRING, 16711935); 
    READ_VALUE(ASM_OPERATOR, 0x006A240A);
    READ_VALUE(ASM_IDENTIFIER, 0x00FF0000);
    READ_VALUE(ASM_CPUINSTRUCTION, 0x006A240A);
    READ_VALUE(ASM_MATHINSTRUCTION, 16776960);
    READ_VALUE(ASM_REGISTER, 0x000080FF);
    READ_VALUE(ASM_DIRECTIVE, 5197615);
    READ_VALUE(ASM_DIRECTIVEOPERAND, 0x006A240A );
    READ_VALUE(ASM_COMMENTBLOCK, 65280);
    READ_VALUE(ASM_CHARACTER, 16711935);
    READ_VALUE(ASM_STRINGEOL, 0);
    READ_VALUE(ASM_EXTINSTRUCTION, 8855416);
    READ_VALUE(ASM_COMMENTDIRECTIVE,65280);
    READ_VALUE(ASM_ADDRESS,0x00808080);
    READ_VALUE(ASM_OPCODE,0);
    READ_VALUE(ASM_JUMPCALL, 255);

    READ_VALUE(DBGCMD_DEFAULT, 0x0);
    READ_VALUE(DBGCMD_STRINGEOL, 0x0);
    READ_VALUE(DBGCMD_CMD, 0xff);
    READ_VALUE(DBGCMD_CMD_CHAR, 0x0);

    // from E:\src\c\npp\PowerEditor\src\langs.model.xml
    /*cpuInstruction*/m_keywords[0] = pt.get<std::string>("keywords.cpuInstruction", 
"aaa aad aam aas adc add and call cbw clc cld cli cmc cmp cmps cmpsb cmpsw cwd daa das dec div esc hlt idiv imul in inc int into iret ja jae jb jbe jc jcxz je jg jge jl jle jmp jna jnae jnb jnbe jnc jne jng jnge jnl jnle jno jnp jns jnz jo jp jpe jpo js jz lahf lds lea les lods lodsb lodsw loop loope loopew loopne loopnew loopnz loopnzw loopw loopz loopzw mov movs movsb movsw mul neg nop not or out pop popf push pushf rcl rcr ret retf retn rol ror sahf sal sar sbb scas scasb scasw shl shr stc std sti stos stosb stosw sub test wait xchg xlat xlatb xor bound enter ins insb insw leave outs outsb outsw popa pusha pushw arpl lar lsl sgdt sidt sldt smsw str verr verw clts lgdt lidt lldt lmsw ltr bsf bsr bt btc btr bts cdq cmpsd cwde insd iretd iretdf iretf jecxz lfs lgs lodsd loopd looped loopned loopnzd loopzd lss movsd movsx movzx outsd popad popfd pushad pushd pushfd scasd seta setae setb setbe setc sete setg setge setl setle setna setnae setnb setnbe setnc setne setng setnge setnl setnle setno setnp setns setnz seto setp setpe setpo sets setz shld shrd stosd bswap cmpxchg invd invlpg wbinvd xadd lock rep repe repne repnz repz cflush cpuid emms femms cmovo cmovno cmovb cmovc cmovnae cmovae cmovnb cmovnc cmove cmovz cmovne cmovnz cmovbe cmovna cmova cmovnbe cmovs cmovns cmovp cmovpe cmovnp cmovpo cmovl cmovnge cmovge cmovnl cmovle cmovng cmovg cmovnle cmpxchg486 cmpxchg8b loadall loadall286 ibts icebp int1 int3 int01 int03 iretw popaw popfw pushaw pushfw rdmsr rdpmc rdshr rdtsc rsdc rsldt rsm rsts salc smi smint smintold svdc svldt svts syscall sysenter sysexit sysret ud0 ud1 ud2 umov xbts wrmsr wrshr"
        );
    /*mathInstruction*/m_keywords[1] = pt.get<std::string>("keywords.mathInstruction",
"f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcom fcomp fcompp fdecstp fdisi fdiv fdivp fdivr fdivrp feni ffree fiadd ficom ficomp fidiv fidivr fild fimul fincstp finit fist fistp fisub fisubr fld fld1 fldcw fldenv fldenvw fldl2e fldl2t fldlg2 fldln2 fldpi fldz fmul fmulp fnclex fndisi fneni fninit fnop fnsave fnsavew fnstcw fnstenv fnstenvw fnstsw fpatan fprem fptan frndint frstor frstorw fsave fsavew fscale fsqrt fst fstcw fstenv fstenvw fstp fstsw fsub fsubp fsubr fsubrp ftst fwait fxam fxch fxtract fyl2x fyl2xp1 fsetpm fcos fldenvd fnsaved fnstenvd fprem1 frstord fsaved fsin fsincos fstenvd fucom fucomp fucompp fcomi fcomip ffreep fcmovb fcmove fcmovbe fcmovu fcmovnb fcmovne fcmovnbe fcmovnu"
        );
    /*registers*/m_keywords[2] = pt.get<std::string>("keywords.registers",
"ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp ebx ecx edi edx es esi esp fs gs si sp ss st tr3 tr4 tr5 tr6 tr7 st0 st1 st2 st3 st4 st5 st6 st7 mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7"
        );    
    /*directive*/m_keywords[3] = pt.get<std::string>("keywords.directive",
".186 .286 .286c .286p .287 .386 .386c .386p .387 .486 .486p .8086 .8087 .alpha .break .code .const .continue .cref .data .data? .dosseg .else .elseif .endif .endw .err .err1 .err2 .errb .errdef .errdif .errdifi .erre .erridn .erridni .errnb .errndef .errnz .exit .fardata .fardata? .if .lall .lfcond .list .listall .listif .listmacro .listmacroall .model .no87 .nocref .nolist .nolistif .nolistmacro .radix .repeat .sall .seq .sfcond .stack .startup .tfcond .type .until .untilcxz .while .xall .xcref .xlist alias align assume catstr comm comment db dd df dosseg dq dt dup dw echo else elseif elseif1 elseif2 elseifb elseifdef elseifdif elseifdifi elseife elseifidn elseifidni elseifnb elseifndef end endif endm endp ends eq equ even exitm extern externdef extrn for forc ge goto group gt high highword if if1 if2 ifb ifdef ifdif ifdifi ife ifidn ifidni ifnb ifndef include includelib instr invoke irp irpc label le length lengthof local low lowword lroffset lt macro mask mod .msfloat name ne offset opattr option org %out page popcontext proc proto ptr public purge pushcontext record repeat rept seg segment short size sizeof sizestr struc struct substr subtitle subttl textequ this title type typedef union while width db dw dd dq dt resb resw resd resq rest incbin equ times %define %idefine %xdefine %xidefine %undef %assign %iassign %strlen %substr %macro %imacro %endmacro %rotate .nolist %if %elif %else %endif %ifdef %ifndef %elifdef %elifndef %ifmacro %ifnmacro %elifmacro %elifnmacro %ifctk %ifnctk %elifctk %elifnctk %ifidn %ifnidn %elifidn %elifnidn %ifidni %ifnidni %elifidni %elifnidni %ifid %ifnid %elifid %elifnid %ifstr %ifnstr %elifstr %elifnstr %ifnum %ifnnum %elifnum %elifnnum %error %rep %endrep %exitrep %include %push %pop %repl struct endstruc istruc at iend align alignb %arg %stacksize %local %line bits use16 use32 section absolute extern global common cpu org section group import export"
        );    
    /*directiveOperand*/m_keywords[4] = pt.get<std::string>("keywords.directiveOperand",
"$ ? @b @f addr basic byte c carry? dword far far16 fortran fword near near16 overflow? parity? pascal qword real4 real8 real10 sbyte sdword sign? stdcall sword syscall tbyte vararg word zero? flat near32 far32 abs all assumes at casemap common compact cpu dotname emulator epilogue error export expr16 expr32 farstack flat forceframe huge language large listing ljmp loadds m510 medium memory nearstack nodotname noemulator nokeyword noljmp nom510 none nonunique nooldmacros nooldstructs noreadonly noscoped nosignextend nothing notpublic oldmacros oldstructs os_dos para private prologue radix readonly req scoped setif2 smallstack tiny use16 use32 uses a16 a32 o16 o32 byte word dword nosplit $ $$ seq wrt flat large small .text .data .bss near far %0 %1 %2 %3 %4 %5 %6 %7 %8 %9"
        );    
    /*extInstruction*/m_keywords[5] = pt.get<std::string>("keywords.extInstruction",
"addpd addps addsd addss andpd andps andnpd andnps cmpeqpd cmpltpd cmplepd cmpunordpd cmpnepd cmpnltpd cmpnlepd cmpordpd cmpeqps cmpltps cmpleps cmpunordps cmpneps cmpnltps cmpnleps cmpordps cmpeqsd cmpltsd cmplesd cmpunordsd cmpnesd cmpnltsd cmpnlesd cmpordsd cmpeqss cmpltss cmpless cmpunordss cmpness cmpnltss cmpnless cmpordss comisd comiss cvtdq2pd cvtdq2ps cvtpd2dq cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd cvtps2pi cvtss2sd cvtss2si cvtsd2si cvtsd2ss cvtsi2sd cvtsi2ss cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi cvttsd2si cvttss2si divpd divps divsd divss fxrstor fxsave ldmxscr lfence mfence maskmovdqu maskmovdq maxpd maxps paxsd maxss minpd minps minsd minss movapd movaps movdq2q movdqa movdqu movhlps movhpd movhps movd movq movlhps movlpd movlps movmskpd movmskps movntdq movnti movntpd movntps movntq movq2dq movsd movss movupd movups mulpd mulps mulsd mulss orpd orps packssdw packsswb packuswb paddb paddsb paddw paddsw paddd paddsiw paddq paddusb paddusw pand pandn pause paveb pavgb pavgw pavgusb pdistib pextrw pcmpeqb pcmpeqw pcmpeqd pcmpgtb pcmpgtw pcmpgtd pf2id pf2iw pfacc pfadd pfcmpeq pfcmpge pfcmpgt pfmax pfmin pfmul pmachriw pmaddwd pmagw pmaxsw pmaxub pminsw pminub pmovmskb pmulhrwc pmulhriw pmulhrwa pmulhuw pmulhw pmullw pmuludq pmvzb pmvnzb pmvlzb pmvgezb pfnacc pfpnacc por prefetch prefetchw prefetchnta prefetcht0 prefetcht1 prefetcht2 pfrcp pfrcpit1 pfrcpit2 pfrsqit1 pfrsqrt pfsub pfsubr pi2fd pf2iw pinsrw psadbw pshufd pshufhw pshuflw pshufw psllw pslld psllq pslldq psraw psrad psrlw psrld psrlq psrldq psubb psubw psubd psubq psubsb psubsw psubusb psubusw psubsiw pswapd punpckhbw punpckhwd punpckhdq punpckhqdq punpcklbw punpcklwd punpckldq punpcklqdq pxor rcpps rcpss rsqrtps rsqrtss sfence shufpd shufps sqrtpd sqrtps sqrtsd sqrtss stmxcsr subpd subps subsd subss ucomisd ucomiss unpckhpd unpckhps unpcklpd unpcklps xorpd xorps"
        );

    m_keywords[6] = pt.get<std::string>("keywords.jumpcall",
        "call ja jae jb jbe jc jcxz je jg jge jl jle jmp jna jnae jnb jnbe jnc jne jng jnge jnl jnle jno jnp jns jnz jo jp jpe jpo js jz"
        );

    if (!bExsit)
    {
        save(FILE_NAME);
    }
    return true;
}

void CConfig::save( const std::string &filename )
{
    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;

#define WRITE_VALUE(x) \
    pt.put("color."#x, m_color[SCE_##x]);
    WRITE_VALUE(ASM_DEFAULT);
    WRITE_VALUE(ASM_COMMENT);
    WRITE_VALUE(ASM_NUMBER);
    WRITE_VALUE(ASM_STRING);
    WRITE_VALUE(ASM_OPERATOR);
    WRITE_VALUE(ASM_IDENTIFIER);
    WRITE_VALUE(ASM_CPUINSTRUCTION);
    WRITE_VALUE(ASM_MATHINSTRUCTION);
    WRITE_VALUE(ASM_REGISTER);
    WRITE_VALUE(ASM_DIRECTIVE);
    WRITE_VALUE(ASM_DIRECTIVEOPERAND);
    WRITE_VALUE(ASM_COMMENTBLOCK);
    WRITE_VALUE(ASM_CHARACTER);
    WRITE_VALUE(ASM_STRINGEOL);
    WRITE_VALUE(ASM_EXTINSTRUCTION);
    WRITE_VALUE(ASM_COMMENTDIRECTIVE);
    WRITE_VALUE(ASM_ADDRESS);
    WRITE_VALUE(ASM_OPCODE);
    WRITE_VALUE(ASM_JUMPCALL);

    WRITE_VALUE(DBGCMD_DEFAULT);
    WRITE_VALUE(DBGCMD_STRINGEOL);
    WRITE_VALUE(DBGCMD_CMD);
    WRITE_VALUE(DBGCMD_CMD_CHAR);

    pt.put("keywords.cpuInstruction", m_keywords[0]);
    pt.put("keywords.mathInstruction", m_keywords[1]);
    pt.put("keywords.registers", m_keywords[2]);
    pt.put("keywords.directive", m_keywords[3]);
    pt.put("keywords.directiveOperand", m_keywords[4]);
    pt.put("keywords.extInstruction", m_keywords[5]);
    pt.put("keywords.jumpcall", m_keywords[6]);

    try
    {
        write_ini(filename, pt); // uac开了无权限就失败
    }catch(...)
    {
    }
}

COLORREF CConfig::get_color( unsigned int index )
{
    return m_color[index];
}

CConfig::CConfig()
{
    load(FILE_NAME);
}

const char* CConfig::get_keywords( int index )
{
    return m_keywords[index].c_str();
}

static const TCHAR _afxOldWndProc[] = _T("AfxOldWndProc456");

LRESULT CALLBACK
CConfig::_AfxActivationWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, _afxOldWndProc);
    //ASSERT(oldWndProc != NULL);	

    LRESULT lResult = 0;

    BOOL bCallDefault = TRUE;
    switch (nMsg)
    {
    case WM_NCDESTROY:
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<INT_PTR>(oldWndProc));
        RemoveProp(hWnd, _afxOldWndProc);
        GlobalDeleteAtom(GlobalFindAtom(_afxOldWndProc));
        CConfig_Single.DelWnd(hWnd);
        break;
    }

    return  CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
}
void CConfig::AddWndCache( HWND hWnd, int which )
{
    assert(which != 0);
    // from D:\Program Files\Microsoft Visual Studio 9.0\VC\atlmfc\src\mfc\wincore.cpp
    WNDPROC oldWndProc;
    oldWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    if (oldWndProc != NULL && GetProp(hWnd, _afxOldWndProc) == NULL)
    {
        SetProp(hWnd, _afxOldWndProc, oldWndProc);
        if ((WNDPROC)GetProp(hWnd, _afxOldWndProc) == oldWndProc)
        {
            GlobalAddAtom(_afxOldWndProc);
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (DWORD_PTR)_AfxActivationWndProc);
            //ASSERT(oldWndProc != NULL);

            m_hwnd[hWnd] = which;
        }
    }
}

int CConfig::WhichWnd( HWND hWnd )
{
    MAP_WND::const_iterator ci = m_hwnd.find(hWnd);
    if (ci == m_hwnd.end())
    {
        return 0;
    }
    else
    {
        return m_hwnd[hWnd];
    }
}

void CConfig::DelWnd( HWND hWnd)
{
    m_hwnd.erase(hWnd);
}
