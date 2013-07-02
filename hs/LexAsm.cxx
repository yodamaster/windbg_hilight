// Scintilla source code edit control
/** @file LexAsm.cxx
 ** Lexer for Assembler, just for the MASM syntax
 ** Written by The Black Horus
 ** Enhancements and NASM stuff by Kein-Hong Man, 2003-10
 ** SCE_ASM_COMMENTBLOCK and SCE_ASM_CHARACTER are for future GNU as colouring
 ** Converted to lexer object and added further folding features/properties by "Udo Lechner" <dlchnr(at)gmx(dot)net>
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/************************************************************************/
/* 2013年4月25日 特别添加对windbg反汇编的地址以及OPCODE的处理
 ChangeState 没有实际动作，只是修改state，state随时可以变，要等到Setstate才设置stylebuffer
 SetState 完成上次state的绘制
 ForwardSetState Forward+Setstate
 */
/************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>
#include <map>
#include <set>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"

#include "LexAsm.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

static inline bool IsAWordChar(const int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '.' ||
		ch == '_' || ch == '?');
}

static inline bool IsAWordStart(const int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '_' || ch == '.' ||
		ch == '%' || ch == '@' || ch == '$' || ch == '?');
}

static inline bool IsAsmOperator(const int ch) {
	if ((ch < 0x80) && (isalnum(ch)))
		return false;
	// '.' left out as it is used to make up numbers
	if (ch == '*' || ch == '/' || ch == '-' || ch == '+' ||
		ch == '(' || ch == ')' || ch == '=' || ch == '^' ||
		ch == '[' || ch == ']' || ch == '<' || ch == '&' ||
		ch == '>' || ch == ',' || ch == '|' || ch == '~' ||
		ch == '%' || ch == ':')
		return true;
	return false;
}

static bool IsStreamCommentStyle(int style) {
	return style == SCE_ASM_COMMENTDIRECTIVE || style == SCE_ASM_COMMENTBLOCK;
}

static inline int LowerCase(int c) {
	if (c >= 'A' && c <= 'Z')
		return 'a' + c - 'A';
	return c;
}

// An individual named option for use in an OptionSet

// Options used for LexerAsm
struct OptionsAsm {
	std::string delimiter;
	bool fold;
	bool foldSyntaxBased;
	bool foldCommentMultiline;
	bool foldCommentExplicit;
	std::string foldExplicitStart;
	std::string foldExplicitEnd;
	bool foldExplicitAnywhere;
	bool foldCompact;
	OptionsAsm() {
		delimiter = "";
		fold = false;
		foldSyntaxBased = true;
		foldCommentMultiline = false;
		foldCommentExplicit = false;
		foldExplicitStart = "";
		foldExplicitEnd   = "";
		foldExplicitAnywhere = false;
		foldCompact = true;
	}
};

static const char * const asmWordListDesc[] = {
	"CPU instructions",
	"FPU instructions",
	"Registers",
	"Directives",
	"Directive operands",
	"Extended instructions",
	"Directives4Foldstart",
	"Directives4Foldend",
	0
};

struct OptionSetAsm : public OptionSet<OptionsAsm> {
	OptionSetAsm() {
		DefineProperty("lexer.asm.comment.delimiter", &OptionsAsm::delimiter,
			"Character used for COMMENT directive's delimiter, replacing the standard \"~\".");

		DefineProperty("fold", &OptionsAsm::fold);

		DefineProperty("fold.asm.syntax.based", &OptionsAsm::foldSyntaxBased,
			"Set this property to 0 to disable syntax based folding.");

		DefineProperty("fold.asm.comment.multiline", &OptionsAsm::foldCommentMultiline,
			"Set this property to 1 to enable folding multi-line comments.");

		DefineProperty("fold.asm.comment.explicit", &OptionsAsm::foldCommentExplicit,
			"This option enables folding explicit fold points when using the Asm lexer. "
			"Explicit fold points allows adding extra folding by placing a ;{ comment at the start and a ;} "
			"at the end of a section that should fold.");

		DefineProperty("fold.asm.explicit.start", &OptionsAsm::foldExplicitStart,
			"The string to use for explicit fold start points, replacing the standard ;{.");

		DefineProperty("fold.asm.explicit.end", &OptionsAsm::foldExplicitEnd,
			"The string to use for explicit fold end points, replacing the standard ;}.");

		DefineProperty("fold.asm.explicit.anywhere", &OptionsAsm::foldExplicitAnywhere,
			"Set this property to 1 to enable explicit fold points anywhere, not just in line comments.");

		DefineProperty("fold.compact", &OptionsAsm::foldCompact);

		DefineWordListSets(asmWordListDesc);
	}
};

class LexerAsm : public ILexer {
	WordList cpuInstruction;
	WordList mathInstruction;
	WordList registers;
	WordList directive;
	WordList directiveOperand;
	WordList extInstruction;
	WordList directives4foldstart;
	WordList directives4foldend;
    WordList jumpcall;
	OptionsAsm options;
	OptionSetAsm osAsm;
public:
	LexerAsm() {
	}
	virtual ~LexerAsm() {
	}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvOriginal;
	}
	const char * SCI_METHOD PropertyNames() {
		return osAsm.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) {
		return osAsm.PropertyType(name);
	}
	const char * SCI_METHOD DescribeProperty(const char *name) {
		return osAsm.DescribeProperty(name);
	}
	int SCI_METHOD PropertySet(const char *key, const char *val);
	const char * SCI_METHOD DescribeWordListSets() {
		return osAsm.DescribeWordListSets();
	}
	int SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess);

	void * SCI_METHOD PrivateCall(int, void *) {
		return 0;
	}

	static ILexer *LexerFactoryAsm() {
		return new LexerAsm();
	}
};

int SCI_METHOD LexerAsm::PropertySet(const char *key, const char *val) {
	if (osAsm.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

int SCI_METHOD LexerAsm::WordListSet(int n, const char *wl) {
	WordList *wordListN = 0;
	switch (n) {
	case 0:
		wordListN = &cpuInstruction;
		break;
	case 1:
		wordListN = &mathInstruction;
		break;
	case 2:
		wordListN = &registers;
		break;
	case 3:
		wordListN = &directive;
		break;
	case 4:
		wordListN = &directiveOperand;
		break;
	case 5:
		wordListN = &extInstruction;
		break;
    case 6:
        wordListN = &jumpcall;
	//case 6:
	//	wordListN = &directives4foldstart;
	//	break;
	//case 7:
	//	wordListN = &directives4foldend;
	//	break;
	}
	int firstModification = -1;
	if (wordListN) {
		WordList wlNew;
		wlNew.Set(wl);
		if (*wordListN != wlNew) {
			wordListN->Set(wl);
			firstModification = 0;
		}
	}
	return firstModification;
}

void SCI_METHOD LexerAsm::Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	LexAccessor styler(pAccess);

    bool bAddressReaded = false; // 地址是SCE_ASM_NUMBER，并且总要出现，主要是处理有源码显示的情况

	// Do not leak onto next line
	if (initStyle == SCE_ASM_STRINGEOL)
		initStyle = SCE_ASM_DEFAULT;

	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward())
	{

		// Prevent SCE_ASM_STRINGEOL from leaking back to previous line
		if (sc.atLineStart && (sc.state == SCE_ASM_STRING)) {
			sc.SetState(SCE_ASM_STRING);
		} else if (sc.atLineStart && (sc.state == SCE_ASM_CHARACTER)) {
			sc.SetState(SCE_ASM_CHARACTER);
		}

		// Handle line continuation generically.
		if (sc.ch == '\\') {
			if (sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}

		// Determine if the current state should terminate.
		if (sc.state == SCE_ASM_OPERATOR) {
			if (!IsAsmOperator(sc.ch)) {
			    sc.SetState(SCE_ASM_DEFAULT);
			}
		} else if (sc.state == SCE_ASM_NUMBER) {
            if (!bAddressReaded)
            {
                if (sc.ch == '`') // 64位包含`字符，如 000007ff`fffe0000
                {
                    sc.Forward();
                }
            }
			if (!IsAWordChar(sc.ch)) {
                if (!bAddressReaded) // 如果还没有读取到地址
                {
                    char s[100];
                    sc.GetCurrentLowered(s, sizeof(s));
                    if (strlen(s) == 8 // 32位
                        || strlen(s) == 17) // 64位是8+1+8
                    {
                        bAddressReaded = true;
                        sc.ChangeState(SCE_ASM_ADDRESS);
                        sc.SetState(SCE_ASM_DEFAULT);
                        sc.ForwardSetState(SCE_ASM_OPCODE);
                    }
                    else
                        sc.SetState(SCE_ASM_DEFAULT);
                }
                else
				    sc.SetState(SCE_ASM_DEFAULT);
			}
		} else if (sc.state == SCE_ASM_IDENTIFIER) {
			if (!IsAWordChar(sc.ch) ) {
				char s[100];
				sc.GetCurrentLowered(s, sizeof(s));
				bool IsDirective = false;

				if (cpuInstruction.InList(s)) {
					sc.ChangeState(SCE_ASM_CPUINSTRUCTION);
				} else if (mathInstruction.InList(s)) {
					sc.ChangeState(SCE_ASM_MATHINSTRUCTION);
				} else if (registers.InList(s)) {
					sc.ChangeState(SCE_ASM_REGISTER);
				}  else if (directive.InList(s)) {
					sc.ChangeState(SCE_ASM_DIRECTIVE);
					IsDirective = true;
				} else if (directiveOperand.InList(s)) {
					sc.ChangeState(SCE_ASM_DIRECTIVEOPERAND);
				} else if (extInstruction.InList(s)) {
					sc.ChangeState(SCE_ASM_EXTINSTRUCTION);
				}
                
                if (jumpcall.InList(s))
                {
                    sc.ChangeState(SCE_ASM_JUMPCALL);
                }

				sc.SetState(SCE_ASM_DEFAULT);
				if (IsDirective && !strcmp(s, "comment")) {
					char delimiter = options.delimiter.empty() ? '~' : options.delimiter.c_str()[0];
					while (IsASpaceOrTab(sc.ch) && !sc.atLineEnd) {
						sc.ForwardSetState(SCE_ASM_DEFAULT);
					}
					if (sc.ch == delimiter) {
						sc.SetState(SCE_ASM_COMMENTDIRECTIVE);
					}
				}
			}
		} else if (sc.state == SCE_ASM_COMMENTDIRECTIVE) {
			char delimiter = options.delimiter.empty() ? '~' : options.delimiter.c_str()[0];
			if (sc.ch == delimiter) {
				while (!sc.atLineEnd) {
					sc.Forward();
				}
				sc.SetState(SCE_ASM_DEFAULT);
			}
		} else if (sc.state == SCE_ASM_COMMENT ) {
			if (sc.atLineEnd) {
				sc.SetState(SCE_ASM_DEFAULT);
			}
		} else if (sc.state == SCE_ASM_STRING) {
			if (sc.ch == '\\') {
				if (sc.chNext == '\"' || sc.chNext == '\'' || sc.chNext == '\\') {
					sc.Forward();
				}
			} else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_ASM_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_ASM_STRINGEOL);
				sc.ForwardSetState(SCE_ASM_DEFAULT);
			}
		} else if (sc.state == SCE_ASM_CHARACTER) {
			if (sc.ch == '\\') {
				if (sc.chNext == '\"' || sc.chNext == '\'' || sc.chNext == '\\') {
					sc.Forward();
				}
			} else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_ASM_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_ASM_STRINGEOL);
				sc.ForwardSetState(SCE_ASM_DEFAULT);
			}
		} 

        // add by lynnux，地址过后就是OPCODE，处理很简单，但是有源码的时候就不是这样了
        else if (sc.state == SCE_ASM_ADDRESS)
        {
            // x64位上是00000000`77751220
		    if (sc.ch == ' ')
		    {
                sc.SetState(SCE_ASM_DEFAULT);
                sc.ForwardSetState(SCE_ASM_OPCODE);
		    }
            //if (sc.ch == '!') // 有时候会出现单独一行如，ntdll!LdrGetFailureData:
            //{
            //    sc.ChangeState(SCE_ASM_IDENTIFIER);
            //}
		}
        else if(sc.state == SCE_ASM_OPCODE)
        {
            if (sc.ch == ' ')
            {
                sc.SetState(SCE_ASM_DEFAULT);
            }
        }


		// Determine if a new state should be entered.
		if (sc.state == SCE_ASM_DEFAULT) {
			if (sc.ch == ';'){
				sc.SetState(SCE_ASM_COMMENT);
			} else if (isascii(sc.ch) && (isdigit(sc.ch) || (sc.ch == '.' && isascii(sc.chNext) && isdigit(sc.chNext)))) {
				sc.SetState(SCE_ASM_NUMBER);
			} else if (IsAWordStart(sc.ch)) {
				sc.SetState(SCE_ASM_IDENTIFIER);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_ASM_STRING);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_ASM_CHARACTER);
			} else if (IsAsmOperator(sc.ch)) {
				sc.SetState(SCE_ASM_OPERATOR);
			}
		}

	}
	sc.Complete();
}

// Store both the current line's fold level and the next lines in the
// level store to make it easy to pick up with each increment
// and to make it possible to fiddle the current level for "else".

void SCI_METHOD LexerAsm::Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {

	if (!options.fold)
		return;

	LexAccessor styler(pAccess);

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelNext = levelCurrent;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;
	char word[100];
	int wordlen = 0;
	const bool userDefinedFoldMarkers = !options.foldExplicitStart.empty() && !options.foldExplicitEnd.empty();
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (options.foldCommentMultiline && IsStreamCommentStyle(style)) {
			if (!IsStreamCommentStyle(stylePrev)) {
				levelNext++;
			} else if (!IsStreamCommentStyle(styleNext) && !atEOL) {
				// Comments don't end at end of line and the next character may be unstyled.
				levelNext--;
			}
		}
		if (options.foldCommentExplicit && ((style == SCE_ASM_COMMENT) || options.foldExplicitAnywhere)) {
			if (userDefinedFoldMarkers) {
				if (styler.Match(i, options.foldExplicitStart.c_str())) {
 					levelNext++;
				} else if (styler.Match(i, options.foldExplicitEnd.c_str())) {
 					levelNext--;
 				}
			} else {
				if (ch == ';') {
					if (chNext == '{') {
						levelNext++;
					} else if (chNext == '}') {
						levelNext--;
					}
				}
 			}
 		}
		if (options.foldSyntaxBased && (style == SCE_ASM_DIRECTIVE)) {
			word[wordlen++] = static_cast<char>(LowerCase(ch));
			if (wordlen == 100) {                   // prevent overflow
				word[0] = '\0';
				wordlen = 1;
			}
			if (styleNext != SCE_ASM_DIRECTIVE) {   // reading directive ready
				word[wordlen] = '\0';
				wordlen = 0;
				if (directives4foldstart.InList(word)) {
					levelNext++;
				} else if (directives4foldend.InList(word)){
					levelNext--;
				}
			}
		}
		if (!IsASpace(ch))
			visibleChars++;
		if (atEOL || (i == endPos-1)) {
			int levelUse = levelCurrent;
			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 && options.foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			if (atEOL && (i == static_cast<unsigned int>(styler.Length()-1))) {
				// There is an empty line at end of file so give it same level and empty
				styler.SetLevel(lineCurrent, (levelCurrent | levelCurrent << 16) | SC_FOLDLEVELWHITEFLAG);
			}
			visibleChars = 0;
		}
	}
}

LexerModule lmAsm(SCLEX_ASM, LexerAsm::LexerFactoryAsm, "asm", asmWordListDesc);

