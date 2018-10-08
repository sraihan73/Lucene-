using namespace std;

#include "ClassicTokenizerImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::standard
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
std::deque<int> const ClassicTokenizerImpl::ZZ_LEXSTATE = {0, 0};
const wstring ClassicTokenizerImpl::ZZ_CMAP_PACKED =
    wstring(L"\46\0\1\5\1\3\4\0\1\11\1\7\1\4\1\11\12\2\6\0") +
    L"\1\6\32\12\4\0\1\10\1\0\32\12\57\0\1\12\12\0\1\12" +
    L"\4\0\1\12\5\0\27\12\1\0\37\12\1\0\u0128\12\2\0\22\12" +
    L"\34\0\136\12\2\0\11\12\2\0\7\12\16\0\2\12\16\0\5\12" +
    L"\11\0\1\12\213\0\1\12\13\0\1\12\1\0\3\12\1\0\1\12" +
    L"\1\0\24\12\1\0\54\12\1\0\10\12\2\0\32\12\14\0\202\12" +
    L"\12\0\71\12\2\0\2\12\2\0\2\12\3\0\46\12\2\0\2\12" +
    L"\67\0\46\12\2\0\1\12\7\0\47\12\110\0\33\12\5\0\3\12" +
    L"\56\0\32\12\5\0\13\12\25\0\12\2\7\0\143\12\1\0\1\12" +
    L"\17\0\2\12\11\0\12\2\3\12\23\0\1\12\1\0\33\12\123\0" +
    L"\46\12\u015f\0\65\12\3\0\1\12\22\0\1\12\7\0\12\12\4\0" +
    L"\12\2\25\0\10\12\2\0\2\12\2\0\26\12\1\0\7\12\1\0" +
    L"\1\12\3\0\4\12\42\0\2\12\1\0\3\12\4\0\12\2\2\12" +
    L"\23\0\6\12\4\0\2\12\2\0\26\12\1\0\7\12\1\0\2\12" +
    L"\1\0\2\12\1\0\2\12\37\0\4\12\1\0\1\12\7\0\12\2" +
    L"\2\0\3\12\20\0\7\12\1\0\1\12\1\0\3\12\1\0\26\12" +
    L"\1\0\7\12\1\0\2\12\1\0\5\12\3\0\1\12\22\0\1\12" +
    L"\17\0\1\12\5\0\12\2\25\0\10\12\2\0\2\12\2\0\26\12" +
    L"\1\0\7\12\1\0\2\12\2\0\4\12\3\0\1\12\36\0\2\12" +
    L"\1\0\3\12\4\0\12\2\25\0\6\12\3\0\3\12\1\0\4\12" +
    L"\3\0\2\12\1\0\1\12\1\0\2\12\3\0\2\12\3\0\3\12" +
    L"\3\0\10\12\1\0\3\12\55\0\11\2\25\0\10\12\1\0\3\12" +
    L"\1\0\27\12\1\0\12\12\1\0\5\12\46\0\2\12\4\0\12\2" +
    L"\25\0\10\12\1\0\3\12\1\0\27\12\1\0\12\12\1\0\5\12" +
    L"\44\0\1\12\1\0\2\12\4\0\12\2\25\0\10\12\1\0\3\12" +
    L"\1\0\27\12\1\0\20\12\46\0\2\12\4\0\12\2\25\0\22\12" +
    L"\3\0\30\12\1\0\11\12\1\0\1\12\2\0\7\12\71\0\1\1" +
    L"\60\12\1\1\2\12\14\1\7\12\11\1\12\2\47\0\2\12\1\0" +
    L"\1\12\2\0\2\12\1\0\1\12\2\0\1\12\6\0\4\12\1\0" +
    L"\7\12\1\0\3\12\1\0\1\12\1\0\1\12\2\0\2\12\1\0" +
    L"\4\12\1\0\2\12\11\0\1\12\2\0\5\12\1\0\1\12\11\0" +
    L"\12\2\2\0\2\12\42\0\1\12\37\0\12\2\26\0\10\12\1\0" +
    L"\42\12\35\0\4\12\164\0\42\12\1\0\5\12\1\0\2\12\25\0" +
    L"\12\2\6\0\6\12\112\0\46\12\12\0\47\12\11\0\132\12\5\0" +
    L"\104\12\5\0\122\12\6\0\7\12\1\0\77\12\1\0\1\12\1\0" +
    L"\4\12\2\0\7\12\1\0\1\12\1\0\4\12\2\0\47\12\1\0" +
    L"\1\12\1\0\4\12\2\0\37\12\1\0\1\12\1\0\4\12\2\0" +
    L"\7\12\1\0\1\12\1\0\4\12\2\0\7\12\1\0\7\12\1\0" +
    L"\27\12\1\0\37\12\1\0\1\12\1\0\4\12\2\0\7\12\1\0" +
    L"\47\12\1\0\23\12\16\0\11\2\56\0\125\12\14\0\u026c\12\2\0" +
    L"\10\12\12\0\32\12\5\0\113\12\225\0\64\12\54\0\12\2\46\0" +
    L"\12\2\6\0\130\12\10\0\51\12\u0557\0\234\12\4\0\132\12\6\0" +
    L"\26\12\2\0\6\12\2\0\46\12\2\0\6\12\2\0\10\12\1\0" +
    L"\1\12\1\0\1\12\1\0\1\12\1\0\37\12\2\0\65\12\1\0" +
    L"\7\12\1\0\1\12\3\0\3\12\1\0\7\12\3\0\4\12\2\0" +
    L"\6\12\4\0\15\12\5\0\3\12\1\0\7\12\202\0\1\12\202\0" +
    L"\1\12\4\0\1\12\2\0\12\12\1\0\1\12\3\0\5\12\6\0" +
    L"\1\12\1\0\1\12\1\0\1\12\1\0\4\12\1\0\3\12\1\0" +
    L"\7\12\u0ecb\0\2\12\52\0\5\12\12\0\1\13\124\13\10\13\2\13" +
    L"\2\13\132\13\1\13\3\13\6\13\50\13\3\13\1\0\136\12\21\0" +
    L"\30\12\70\0\20\13\u0100\0\200\13\200\0\u19b6\13\12\13\100\0\u51a6\13" +
    L"\132\13\u048d\12\u0773\0\u2ba4\12\u215c\0\u012e\13\322\13\7\12\14\0\5"
    L"\12" +
    L"\5\0\1\12\1\0\12\12\1\0\15\12\1\0\5\12\1\0\1\12" +
    L"\1\0\2\12\1\0\2\12\1\0\154\12\41\0\u016b\12\22\0\100\12" +
    L"\2\0\66\12\50\0\14\12\164\0\3\12\1\0\1\12\1\0\207\12" +
    L"\23\0\12\2\7\0\32\12\6\0\32\12\12\0\1\13\72\13\37\12" +
    L"\3\0\6\12\2\0\6\12\2\0\6\12\2\0\3\12\uffff\0\uffff\0\uffff\0\uffff\0"
    L"\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0"
    L"\uffff\0\uffff\0\uffff\0\63\0";
std::deque<wchar_t> const ClassicTokenizerImpl::ZZ_CMAP =
    zzUnpackCMap(ZZ_CMAP_PACKED);
std::deque<int> const ClassicTokenizerImpl::ZZ_ACTION = zzUnpackAction();
const wstring ClassicTokenizerImpl::ZZ_ACTION_PACKED_0 =
    wstring(L"\1\0\1\1\3\2\1\3\13\0\1\2\3\4\2\0") +
    L"\1\5\1\0\1\5\3\4\6\5\1\6\1\4\2\7" +
    L"\1\10\1\0\1\10\3\0\2\10\1\11\1\12\1\4";

std::deque<int> ClassicTokenizerImpl::zzUnpackAction()
{
  std::deque<int> result(50);
  int offset = 0;
  offset = zzUnpackAction(ZZ_ACTION_PACKED_0, offset, result);
  return result;
}

int ClassicTokenizerImpl::zzUnpackAction(const wstring &packed, int offset,
                                         std::deque<int> &result)
{
  int i = 0;      // index in packed string
  int j = offset; // index in unpacked array
  int l = packed.length();
  while (i < l) {
    int count = packed[i++];
    int value = packed[i++];
    do {
      result[j++] = value;
    } while (--count > 0);
  }
  return j;
}

std::deque<int> const ClassicTokenizerImpl::ZZ_ROWMAP = zzUnpackRowMap();
const wstring ClassicTokenizerImpl::ZZ_ROWMAP_PACKED_0 =
    wstring(L"\0\0\0\14\0\30\0\44\0\60\0\14\0\74\0\110") +
    L"\0\124\0\140\0\154\0\170\0\204\0\220\0\234\0\250" +
    L"\0\264\0\300\0\314\0\330\0\344\0\360\0\374\0\u0108" +
    L"\0\u0114\0\u0120\0\u012c\0\u0138\0\u0144\0\u0150\0\u015c\0\u0168" +
    L"\0\u0174\0\u0180\0\u018c\0\u0198\0\u01a4\0\250\0\u01b0\0\u01bc" +
    L"\0\u01c8\0\u01d4\0\u01e0\0\u01ec\0\u01f8\0\74\0\154\0\u0204" +
    L"\0\u0210\0\u021c";

std::deque<int> ClassicTokenizerImpl::zzUnpackRowMap()
{
  std::deque<int> result(50);
  int offset = 0;
  offset = zzUnpackRowMap(ZZ_ROWMAP_PACKED_0, offset, result);
  return result;
}

int ClassicTokenizerImpl::zzUnpackRowMap(const wstring &packed, int offset,
                                         std::deque<int> &result)
{
  int i = 0;      // index in packed string
  int j = offset; // index in unpacked array
  int l = packed.length();
  while (i < l) {
    int high = packed[i++] << 16;
    result[j++] = high | packed[i++];
  }
  return j;
}

std::deque<int> const ClassicTokenizerImpl::ZZ_TRANS = zzUnpackTrans();
const wstring ClassicTokenizerImpl::ZZ_TRANS_PACKED_0 =
    wstring(L"\1\2\1\3\1\4\7\2\1\5\1\6\15\0\2\3") +
    L"\1\0\1\7\1\0\1\10\2\11\1\12\1\3\2\0" +
    L"\1\3\1\4\1\0\1\13\1\0\1\10\2\14\1\15" +
    L"\1\4\2\0\1\3\1\4\1\16\1\17\1\20\1\21" +
    L"\2\11\1\12\1\22\2\0\1\23\1\24\7\0\1\25" +
    L"\2\0\2\26\7\0\1\26\2\0\1\27\1\30\7\0" +
    L"\1\31\3\0\1\32\7\0\1\12\2\0\1\33\1\34" +
    L"\7\0\1\35\2\0\1\36\1\37\7\0\1\40\2\0" +
    L"\1\41\1\42\7\0\1\43\13\0\1\44\2\0\1\23" +
    L"\1\24\7\0\1\45\13\0\1\46\2\0\2\26\7\0" +
    L"\1\47\2\0\1\3\1\4\1\16\1\7\1\20\1\21" +
    L"\2\11\1\12\1\22\2\0\2\23\1\0\1\50\1\0" +
    L"\1\10\2\51\1\0\1\23\2\0\1\23\1\24\1\0" +
    L"\1\52\1\0\1\10\2\53\1\54\1\24\2\0\1\23" +
    L"\1\24\1\0\1\50\1\0\1\10\2\51\1\0\1\25" +
    L"\2\0\2\26\1\0\1\55\2\0\1\55\2\0\1\26" +
    L"\2\0\2\27\1\0\1\51\1\0\1\10\2\51\1\0" +
    L"\1\27\2\0\1\27\1\30\1\0\1\53\1\0\1\10" +
    L"\2\53\1\54\1\30\2\0\1\27\1\30\1\0\1\51" +
    L"\1\0\1\10\2\51\1\0\1\31\3\0\1\32\1\0" +
    L"\1\54\2\0\3\54\1\32\2\0\2\33\1\0\1\56" +
    L"\1\0\1\10\2\11\1\12\1\33\2\0\1\33\1\34" +
    L"\1\0\1\57\1\0\1\10\2\14\1\15\1\34\2\0" +
    L"\1\33\1\34\1\0\1\56\1\0\1\10\2\11\1\12" +
    L"\1\35\2\0\2\36\1\0\1\11\1\0\1\10\2\11" +
    L"\1\12\1\36\2\0\1\36\1\37\1\0\1\14\1\0" +
    L"\1\10\2\14\1\15\1\37\2\0\1\36\1\37\1\0" +
    L"\1\11\1\0\1\10\2\11\1\12\1\40\2\0\2\41" +
    L"\1\0\1\12\2\0\3\12\1\41\2\0\1\41\1\42" +
    L"\1\0\1\15\2\0\3\15\1\42\2\0\1\41\1\42" +
    L"\1\0\1\12\2\0\3\12\1\43\4\0\1\16\6\0" +
    L"\1\44\2\0\1\23\1\24\1\0\1\60\1\0\1\10" +
    L"\2\51\1\0\1\25\2\0\2\26\1\0\1\55\2\0" +
    L"\1\55\2\0\1\47\2\0\2\23\7\0\1\23\2\0" +
    L"\2\27\7\0\1\27\2\0\2\33\7\0\1\33\2\0" +
    L"\2\36\7\0\1\36\2\0\2\41\7\0\1\41\2\0" +
    L"\2\61\7\0\1\61\2\0\2\23\7\0\1\62\2\0" +
    L"\2\61\1\0\1\55\2\0\1\55\2\0\1\61\2\0" +
    L"\2\23\1\0\1\60\1\0\1\10\2\51\1\0\1\23" + L"\1\0";

std::deque<int> ClassicTokenizerImpl::zzUnpackTrans()
{
  std::deque<int> result(552);
  int offset = 0;
  offset = zzUnpackTrans(ZZ_TRANS_PACKED_0, offset, result);
  return result;
}

int ClassicTokenizerImpl::zzUnpackTrans(const wstring &packed, int offset,
                                        std::deque<int> &result)
{
  int i = 0;      // index in packed string
  int j = offset; // index in unpacked array
  int l = packed.length();
  while (i < l) {
    int count = packed[i++];
    int value = packed[i++];
    value--;
    do {
      result[j++] = value;
    } while (--count > 0);
  }
  return j;
}

std::deque<wstring> const ClassicTokenizerImpl::ZZ_ERROR_MSG = {
    L"Unkown internal scanner error", L"Error: could not match input",
    L"Error: pushback value was too large"};
std::deque<int> const ClassicTokenizerImpl::ZZ_ATTRIBUTE = zzUnpackAttribute();
const wstring ClassicTokenizerImpl::ZZ_ATTRIBUTE_PACKED_0 =
    wstring(L"\1\0\1\11\3\1\1\11\13\0\4\1\2\0\1\1") +
    L"\1\0\17\1\1\0\1\1\3\0\5\1";

std::deque<int> ClassicTokenizerImpl::zzUnpackAttribute()
{
  std::deque<int> result(50);
  int offset = 0;
  offset = zzUnpackAttribute(ZZ_ATTRIBUTE_PACKED_0, offset, result);
  return result;
}

int ClassicTokenizerImpl::zzUnpackAttribute(const wstring &packed, int offset,
                                            std::deque<int> &result)
{
  int i = 0;      // index in packed string
  int j = offset; // index in unpacked array
  int l = packed.length();
  while (i < l) {
    int count = packed[i++];
    int value = packed[i++];
    do {
      result[j++] = value;
    } while (--count > 0);
  }
  return j;
}

std::deque<wstring> const ClassicTokenizerImpl::TOKEN_TYPES =
    ClassicTokenizer::TOKEN_TYPES;

int ClassicTokenizerImpl::yychar() { return yychar_; }

void ClassicTokenizerImpl::getText(shared_ptr<CharTermAttribute> t)
{
  t->copyBuffer(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

void ClassicTokenizerImpl::setBufferSize(int numChars)
{
  throw make_shared<UnsupportedOperationException>();
}

ClassicTokenizerImpl::ClassicTokenizerImpl(shared_ptr<java::io::Reader> in_)
{
  this->zzReader = in_;
}

std::deque<wchar_t> ClassicTokenizerImpl::zzUnpackCMap(const wstring &packed)
{
  std::deque<wchar_t> map_obj(0x110000);
  int i = 0; // index in packed string
  int j = 0; // index in unpacked array
  while (i < 1170) {
    int count = packed[i++];
    wchar_t value = packed[i++];
    do {
      map_obj[j++] = value;
    } while (--count > 0);
  }
  return map_obj;
}

bool ClassicTokenizerImpl::zzRefill() 
{

  /* first: make room (if you can) */
  if (zzStartRead > 0) {
    zzEndRead += zzFinalHighSurrogate;
    zzFinalHighSurrogate = 0;
    System::arraycopy(zzBuffer, zzStartRead, zzBuffer, 0,
                      zzEndRead - zzStartRead);

    /* translate stored positions */
    zzEndRead -= zzStartRead;
    zzCurrentPos -= zzStartRead;
    zzMarkedPos -= zzStartRead;
    zzStartRead = 0;
  }

  /* is the buffer big enough? */
  if (zzCurrentPos >= zzBuffer.size() - zzFinalHighSurrogate) {
    /* if not: blow it up */
    std::deque<wchar_t> newBuffer(zzBuffer.size() * 2);
    System::arraycopy(zzBuffer, 0, newBuffer, 0, zzBuffer.size());
    zzBuffer = newBuffer;
    zzEndRead += zzFinalHighSurrogate;
    zzFinalHighSurrogate = 0;
  }

  /* fill the buffer with new input */
  int requested = zzBuffer.size() - zzEndRead;
  int totalRead = 0;
  while (totalRead < requested) {
    int numRead =
        zzReader->read(zzBuffer, zzEndRead + totalRead, requested - totalRead);
    if (numRead == -1) {
      break;
    }
    totalRead += numRead;
  }

  if (totalRead > 0) {
    zzEndRead += totalRead;
    if (totalRead == requested) { // possibly more input available
      if (Character::isHighSurrogate(zzBuffer[zzEndRead - 1])) {
        --zzEndRead;
        zzFinalHighSurrogate = 1;
      }
    }
    return false;
  }

  // totalRead = 0: End of stream
  return true;
}

void ClassicTokenizerImpl::yyclose() 
{
  zzAtEOF = true;          // indicate end of file
  zzEndRead = zzStartRead; // invalidate buffer

  if (zzReader != nullptr) {
    zzReader->close();
  }
}

void ClassicTokenizerImpl::yyreset(shared_ptr<java::io::Reader> reader)
{
  zzReader = reader;
  zzAtBOL = true;
  zzAtEOF = false;
  zzEOFDone = false;
  zzEndRead = zzStartRead = 0;
  zzCurrentPos = zzMarkedPos = 0;
  zzFinalHighSurrogate = 0;
  yyline = yychar_ = yycolumn = 0;
  zzLexicalState = YYINITIAL;
  if (zzBuffer.size() > ZZ_BUFFERSIZE) {
    zzBuffer = std::deque<wchar_t>(ZZ_BUFFERSIZE);
  }
}

int ClassicTokenizerImpl::yystate() { return zzLexicalState; }

void ClassicTokenizerImpl::yybegin(int newState) { zzLexicalState = newState; }

wstring ClassicTokenizerImpl::yytext()
{
  return wstring(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

wchar_t ClassicTokenizerImpl::yycharat(int pos)
{
  return zzBuffer[zzStartRead + pos];
}

int ClassicTokenizerImpl::yylength() { return zzMarkedPos - zzStartRead; }

void ClassicTokenizerImpl::zzScanError(int errorCode)
{
  wstring message;
  try {
    message = ZZ_ERROR_MSG[errorCode];
  } catch (const out_of_range &e) {
    message = ZZ_ERROR_MSG[ZZ_UNKNOWN_ERROR];
  }

  throw make_shared<Error>(message);
}

void ClassicTokenizerImpl::yypushback(int number)
{
  if (number > yylength()) {
    zzScanError(ZZ_PUSHBACK_2BIG);
  }

  zzMarkedPos -= number;
}

int ClassicTokenizerImpl::getNextToken() 
{
  int zzInput;
  int zzAction;

  // cached fields:
  int zzCurrentPosL;
  int zzMarkedPosL;
  int zzEndReadL = zzEndRead;
  std::deque<wchar_t> zzBufferL = zzBuffer;
  std::deque<wchar_t> zzCMapL = ZZ_CMAP;

  std::deque<int> zzTransL = ZZ_TRANS;
  std::deque<int> zzRowMapL = ZZ_ROWMAP;
  std::deque<int> zzAttrL = ZZ_ATTRIBUTE;

  while (true) {
    zzMarkedPosL = zzMarkedPos;

    yychar_ += zzMarkedPosL - zzStartRead;

    zzAction = -1;

    zzCurrentPosL = zzCurrentPos = zzStartRead = zzMarkedPosL;

    zzState = ZZ_LEXSTATE[zzLexicalState];

    // set up zzAction for empty match case:
    int zzAttributes = zzAttrL[zzState];
    if ((zzAttributes & 1) == 1) {
      zzAction = zzState;
    }

    {
      while (true) {

        if (zzCurrentPosL < zzEndReadL) {
          zzInput =
              Character::codePointAt(zzBufferL, zzCurrentPosL, zzEndReadL);
          zzCurrentPosL += Character::charCount(zzInput);
        } else if (zzAtEOF) {
          zzInput = YYEOF;
          goto zzForActionBreak;
        } else {
          // store back cached positions
          zzCurrentPos = zzCurrentPosL;
          zzMarkedPos = zzMarkedPosL;
          bool eof = zzRefill();
          // get translated positions and possibly new buffer
          zzCurrentPosL = zzCurrentPos;
          zzMarkedPosL = zzMarkedPos;
          zzBufferL = zzBuffer;
          zzEndReadL = zzEndRead;
          if (eof) {
            zzInput = YYEOF;
            goto zzForActionBreak;
          } else {
            zzInput =
                Character::codePointAt(zzBufferL, zzCurrentPosL, zzEndReadL);
            zzCurrentPosL += Character::charCount(zzInput);
          }
        }
        int zzNext = zzTransL[zzRowMapL[zzState] + zzCMapL[zzInput]];
        if (zzNext == -1) {
          goto zzForActionBreak;
        }
        zzState = zzNext;

        zzAttributes = zzAttrL[zzState];
        if ((zzAttributes & 1) == 1) {
          zzAction = zzState;
          zzMarkedPosL = zzCurrentPosL;
          if ((zzAttributes & 8) == 8) {
            goto zzForActionBreak;
          }
        }
      }
    }
  zzForActionBreak:

    // store back cached position
    zzMarkedPos = zzMarkedPosL;

    switch (zzAction < 0 ? zzAction : ZZ_ACTION[zzAction]) {
    case 1: {
      break; // ignore
    }
    case 11:
      break;
    case 2: {
      return ALPHANUM;
    }
    case 12:
      break;
    case 3: {
      return CJ;
    }
    case 13:
      break;
    case 4: {
      return HOST;
    }
    case 14:
      break;
    case 5: {
      return NUM;
    }
    case 15:
      break;
    case 6: {
      return APOSTROPHE;
    }
    case 16:
      break;
    case 7: {
      return COMPANY;
    }
    case 17:
      break;
    case 8: {
      return ACRONYM_DEP;
    }
    case 18:
      break;
    case 9: {
      return ACRONYM;
    }
    case 19:
      break;
    case 10: {
      return EMAIL;
    }
    case 20:
      break;
    default:
      if (zzInput == YYEOF && zzStartRead == zzCurrentPos) {
        zzAtEOF = true;
        return YYEOF;
      } else {
        zzScanError(ZZ_NO_MATCH);
      }
    }
  }
}
} // namespace org::apache::lucene::analysis::standard