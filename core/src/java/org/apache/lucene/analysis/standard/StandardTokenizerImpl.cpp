using namespace std;

#include "StandardTokenizerImpl.h"
#include "../tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::standard
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
std::deque<int> const StandardTokenizerImpl::ZZ_LEXSTATE = {0, 0};
const wstring StandardTokenizerImpl::ZZ_CMAP_PACKED =
    wstring(L"\42\0\1\15\4\0\1\14\4\0\1\7\1\0\1\10\1\0\12\4") +
    L"\1\6\1\7\5\0\32\1\4\0\1\11\1\0\32\1\57\0\1\1" +
    L"\2\0\1\3\7\0\1\1\1\0\1\6\2\0\1\1\5\0\27\1" +
    L"\1\0\37\1\1\0\u01ca\1\4\0\14\1\5\0\1\6\10\0\5\1" +
    L"\7\0\1\1\1\0\1\1\21\0\160\3\5\1\1\0\2\1\2\0" +
    L"\4\1\1\7\7\0\1\1\1\6\3\1\1\0\1\1\1\0\24\1" +
    L"\1\0\123\1\1\0\213\1\1\0\7\3\236\1\11\0\46\1\2\0" +
    L"\1\1\7\0\47\1\1\0\1\7\7\0\55\3\1\0\1\3\1\0" +
    L"\2\3\1\0\2\3\1\0\1\3\10\0\33\16\5\0\3\16\1\1" +
    L"\1\6\13\0\5\3\7\0\2\7\2\0\13\3\1\0\1\3\3\0" +
    L"\53\1\25\3\12\4\1\0\1\4\1\7\1\0\2\1\1\3\143\1" +
    L"\1\0\1\1\10\3\1\0\6\3\2\1\2\3\1\0\4\3\2\1" +
    L"\12\4\3\1\2\0\1\1\17\0\1\3\1\1\1\3\36\1\33\3" +
    L"\2\0\131\1\13\3\1\1\16\0\12\4\41\1\11\3\2\1\2\0" +
    L"\1\7\1\0\1\1\5\0\26\1\4\3\1\1\11\3\1\1\3\3" +
    L"\1\1\5\3\22\0\31\1\3\3\104\0\1\1\1\0\13\1\67\0" +
    L"\33\3\1\0\4\3\66\1\3\3\1\1\22\3\1\1\7\3\12\1" +
    L"\2\3\2\0\12\4\1\0\7\1\1\0\7\1\1\0\3\3\1\0" +
    L"\10\1\2\0\2\1\2\0\26\1\1\0\7\1\1\0\1\1\3\0" +
    L"\4\1\2\0\1\3\1\1\7\3\2\0\2\3\2\0\3\3\1\1" +
    L"\10\0\1\3\4\0\2\1\1\0\3\1\2\3\2\0\12\4\2\1" +
    L"\17\0\3\3\1\0\6\1\4\0\2\1\2\0\26\1\1\0\7\1" +
    L"\1\0\2\1\1\0\2\1\1\0\2\1\2\0\1\3\1\0\5\3" +
    L"\4\0\2\3\2\0\3\3\3\0\1\3\7\0\4\1\1\0\1\1" +
    L"\7\0\12\4\2\3\3\1\1\3\13\0\3\3\1\0\11\1\1\0" +
    L"\3\1\1\0\26\1\1\0\7\1\1\0\2\1\1\0\5\1\2\0" +
    L"\1\3\1\1\10\3\1\0\3\3\1\0\3\3\2\0\1\1\17\0" +
    L"\2\1\2\3\2\0\12\4\21\0\3\3\1\0\10\1\2\0\2\1" +
    L"\2\0\26\1\1\0\7\1\1\0\2\1\1\0\5\1\2\0\1\3" +
    L"\1\1\7\3\2\0\2\3\2\0\3\3\10\0\2\3\4\0\2\1" +
    L"\1\0\3\1\2\3\2\0\12\4\1\0\1\1\20\0\1\3\1\1" +
    L"\1\0\6\1\3\0\3\1\1\0\4\1\3\0\2\1\1\0\1\1" +
    L"\1\0\2\1\3\0\2\1\3\0\3\1\3\0\14\1\4\0\5\3" +
    L"\3\0\3\3\1\0\4\3\2\0\1\1\6\0\1\3\16\0\12\4" +
    L"\21\0\3\3\1\0\10\1\1\0\3\1\1\0\27\1\1\0\12\1" +
    L"\1\0\5\1\3\0\1\1\7\3\1\0\3\3\1\0\4\3\7\0" +
    L"\2\3\1\0\2\1\6\0\2\1\2\3\2\0\12\4\22\0\2\3" +
    L"\1\0\10\1\1\0\3\1\1\0\27\1\1\0\12\1\1\0\5\1" +
    L"\2\0\1\3\1\1\7\3\1\0\3\3\1\0\4\3\7\0\2\3" +
    L"\7\0\1\1\1\0\2\1\2\3\2\0\12\4\1\0\2\1\17\0" +
    L"\2\3\1\0\10\1\1\0\3\1\1\0\51\1\2\0\1\1\7\3" +
    L"\1\0\3\3\1\0\4\3\1\1\10\0\1\3\10\0\2\1\2\3" +
    L"\2\0\12\4\12\0\6\1\2\0\2\3\1\0\22\1\3\0\30\1" +
    L"\1\0\11\1\1\0\1\1\2\0\7\1\3\0\1\3\4\0\6\3" +
    L"\1\0\1\3\1\0\10\3\22\0\2\3\15\0\60\20\1\21\2\20" +
    L"\7\21\5\0\7\20\10\21\1\0\12\4\47\0\2\20\1\0\1\20" +
    L"\2\0\2\20\1\0\1\20\2\0\1\20\6\0\4\20\1\0\7\20" +
    L"\1\0\3\20\1\0\1\20\1\0\1\20\2\0\2\20\1\0\4\20" +
    L"\1\21\2\20\6\21\1\0\2\21\1\20\2\0\5\20\1\0\1\20" +
    L"\1\0\6\21\2\0\12\4\2\0\4\20\40\0\1\1\27\0\2\3" +
    L"\6\0\12\4\13\0\1\3\1\0\1\3\1\0\1\3\4\0\2\3" +
    L"\10\1\1\0\44\1\4\0\24\3\1\0\2\3\5\1\13\3\1\0" +
    L"\44\3\11\0\1\3\71\0\53\20\24\21\1\20\12\4\6\0\6\20" +
    L"\4\21\4\20\3\21\1\20\3\21\2\20\7\21\3\20\4\21\15\20" +
    L"\14\21\1\20\1\21\12\4\4\21\2\20\46\1\1\0\1\1\5\0" +
    L"\1\1\2\0\53\1\1\0\4\1\u0100\2\111\1\1\0\4\1\2\0" +
    L"\7\1\1\0\1\1\1\0\4\1\2\0\51\1\1\0\4\1\2\0" +
    L"\41\1\1\0\4\1\2\0\7\1\1\0\1\1\1\0\4\1\2\0" +
    L"\17\1\1\0\71\1\1\0\4\1\2\0\103\1\2\0\3\3\40\0" +
    L"\20\1\20\0\125\1\14\0\u026c\1\2\0\21\1\1\0\32\1\5\0" +
    L"\113\1\3\0\3\1\17\0\15\1\1\0\4\1\3\3\13\0\22\1" +
    L"\3\3\13\0\22\1\2\3\14\0\15\1\1\0\3\1\1\0\2\3" +
    L"\14\0\64\20\40\21\3\0\1\20\4\0\1\20\1\21\2\0\12\4" +
    L"\41\0\4\3\1\0\12\4\6\0\130\1\10\0\51\1\1\3\1\1" +
    L"\5\0\106\1\12\0\35\1\3\0\14\3\4\0\14\3\12\0\12\4" +
    L"\36\20\2\0\5\20\13\0\54\20\4\0\21\21\7\20\2\21\6\0" +
    L"\12\4\1\20\3\0\2\20\40\0\27\1\5\3\4\0\65\20\12\21" +
    L"\1\0\35\21\2\0\1\3\12\4\6\0\12\4\6\0\16\20\122\0" +
    L"\5\3\57\1\21\3\7\1\4\0\12\4\21\0\11\3\14\0\3\3" +
    L"\36\1\15\3\2\1\12\4\54\1\16\3\14\0\44\1\24\3\10\0" +
    L"\12\4\3\0\3\1\12\4\44\1\122\0\3\3\1\0\25\3\4\1" +
    L"\1\3\4\1\3\3\2\1\11\0\300\1\47\3\25\0\4\3\u0116\1" +
    L"\2\0\6\1\2\0\46\1\2\0\6\1\2\0\10\1\1\0\1\1" +
    L"\1\0\1\1\1\0\1\1\1\0\37\1\2\0\65\1\1\0\7\1" +
    L"\1\0\1\1\3\0\3\1\1\0\7\1\3\0\4\1\2\0\6\1" +
    L"\4\0\15\1\5\0\3\1\1\0\7\1\17\0\4\3\10\0\2\10" +
    L"\12\0\1\10\2\0\1\6\2\0\5\3\20\0\2\11\3\0\1\7" +
    L"\17\0\1\11\13\0\5\3\1\0\12\3\1\0\1\1\15\0\1\1" +
    L"\20\0\15\1\63\0\41\3\21\0\1\1\4\0\1\1\2\0\12\1" +
    L"\1\0\1\1\3\0\5\1\6\0\1\1\1\0\1\1\1\0\1\1" +
    L"\1\0\4\1\1\0\13\1\2\0\4\1\5\0\5\1\4\0\1\1" +
    L"\21\0\51\1\u032d\0\64\1\u0716\0\57\1\1\0\57\1\1\0\205\1" +
    L"\6\0\4\1\3\3\2\1\14\0\46\1\1\0\1\1\5\0\1\1" +
    L"\2\0\70\1\7\0\1\1\17\0\1\3\27\1\11\0\7\1\1\0" +
    L"\7\1\1\0\7\1\1\0\7\1\1\0\7\1\1\0\7\1\1\0" +
    L"\7\1\1\0\7\1\1\0\40\3\57\0\1\1\120\0\32\12\1\0" +
    L"\131\12\14\0\326\12\57\0\1\1\1\0\1\12\31\0\11\12\6\3" +
    L"\1\0\5\5\2\0\3\12\1\1\1\1\4\0\126\13\2\0\2\3" +
    L"\2\5\3\13\133\5\1\0\4\5\5\0\51\1\3\0\136\2\21\0" +
    L"\33\1\65\0\20\5\320\0\57\5\1\0\130\5\250\0\u19b6\12\112\0" +
    L"\u51cd\12\63\0\u048d\1\103\0\56\1\2\0\u010d\1\3\0\20\1\12\4" +
    L"\2\1\24\0\57\1\4\3\1\0\12\3\1\0\31\1\7\0\1\3" +
    L"\120\1\2\3\45\0\11\1\2\0\147\1\2\0\4\1\1\0\4\1" +
    L"\14\0\13\1\115\0\12\1\1\3\3\1\1\3\4\1\1\3\27\1" +
    L"\5\3\30\0\64\1\14\0\2\3\62\1\21\3\13\0\12\4\6\0" +
    L"\22\3\6\1\3\0\1\1\4\0\12\4\34\1\10\3\2\0\27\1" +
    L"\15\3\14\0\35\2\3\0\4\3\57\1\16\3\16\0\1\1\12\4" +
    L"\46\0\51\1\16\3\11\0\3\1\1\3\10\1\2\3\2\0\12\4" +
    L"\6\0\33\20\1\21\4\0\60\20\1\21\1\20\3\21\2\20\2\21" +
    L"\5\20\2\21\1\20\1\21\1\20\30\0\5\20\13\1\5\3\2\0" +
    L"\3\1\2\3\12\0\6\1\2\0\6\1\2\0\6\1\11\0\7\1" +
    L"\1\0\7\1\221\0\43\1\10\3\1\0\2\3\2\0\12\4\6\0" +
    L"\u2ba4\2\14\0\27\2\4\0\61\2\u2104\0\u016e\12\2\0\152\12\46\0" +
    L"\7\1\14\0\5\1\5\0\1\16\1\3\12\16\1\0\15\16\1\0" +
    L"\5\16\1\0\1\16\1\0\2\16\1\0\2\16\1\0\12\16\142\1" +
    L"\41\0\u016b\1\22\0\100\1\2\0\66\1\50\0\14\1\4\0\20\3" +
    L"\1\7\2\0\1\6\1\7\13\0\7\3\14\0\2\11\30\0\3\11" +
    L"\1\7\1\0\1\10\1\0\1\7\1\6\32\0\5\1\1\0\207\1" +
    L"\2\0\1\3\7\0\1\10\4\0\1\7\1\0\1\10\1\0\12\4" +
    L"\1\6\1\7\5\0\32\1\4\0\1\11\1\0\32\1\13\0\70\5" +
    L"\2\3\37\2\3\0\6\2\2\0\6\2\2\0\6\2\2\0\3\2" +
    L"\34\0\3\3\4\0\14\1\1\0\32\1\1\0\23\1\1\0\2\1" +
    L"\1\0\17\1\2\0\16\1\42\0\173\1\105\0\65\1\210\0\1\3" +
    L"\202\0\35\1\3\0\61\1\57\0\37\1\21\0\33\1\65\0\36\1" +
    L"\2\0\44\1\4\0\10\1\1\0\5\1\52\0\236\1\2\0\12\4" +
    L"\u0356\0\6\1\2\0\1\1\1\0\54\1\1\0\2\1\3\0\1\1" +
    L"\2\0\27\1\252\0\26\1\12\0\32\1\106\0\70\1\6\0\2\1" +
    L"\100\0\1\1\3\3\1\0\2\3\5\0\4\3\4\1\1\0\3\1" +
    L"\1\0\33\1\4\0\3\3\4\0\1\3\40\0\35\1\203\0\66\1" +
    L"\12\0\26\1\12\0\23\1\215\0\111\1\u03b7\0\3\3\65\1\17\3" +
    L"\37\0\12\4\20\0\3\3\55\1\13\3\2\0\1\3\22\0\31\1" +
    L"\7\0\12\4\6\0\3\3\44\1\16\3\1\0\12\4\100\0\3\3" +
    L"\60\1\16\3\4\1\13\0\12\4\u04a6\0\53\1\15\3\10\0\12\4" +
    L"\u0936\0\u036f\1\221\0\143\1\u0b9d\0\u042f\1\u33d1\0\u0239\1\u04c7\0\105"
    L"\1" +
    L"\13\0\1\1\56\3\20\0\4\3\15\1\u4060\0\1\5\1\13\u2163\0" +
    L"\5\3\3\0\26\3\2\0\7\3\36\0\4\3\224\0\3\3\u01bb\0" +
    L"\125\1\1\0\107\1\1\0\2\1\2\0\1\1\2\0\2\1\2\0" +
    L"\4\1\1\0\14\1\1\0\1\1\1\0\7\1\1\0\101\1\1\0" +
    L"\4\1\2\0\10\1\1\0\7\1\1\0\34\1\1\0\4\1\1\0" +
    L"\5\1\1\0\1\1\3\0\7\1\1\0\u0154\1\2\0\31\1\1\0" +
    L"\31\1\1\0\37\1\1\0\31\1\1\0\37\1\1\0\31\1\1\0" +
    L"\37\1\1\0\31\1\1\0\37\1\1\0\31\1\1\0\10\1\2\0" +
    L"\62\4\u1600\0\4\1\1\0\33\1\1\0\2\1\1\0\1\1\2\0" +
    L"\1\1\1\0\12\1\1\0\4\1\1\0\1\1\1\0\1\1\6\0" +
    L"\1\1\4\0\1\1\1\0\1\1\1\0\1\1\1\0\3\1\1\0" +
    L"\2\1\1\0\1\1\2\0\1\1\1\0\1\1\1\0\1\1\1\0" +
    L"\1\1\1\0\1\1\1\0\2\1\1\0\1\1\2\0\4\1\1\0" +
    L"\7\1\1\0\4\1\1\0\4\1\1\0\1\1\1\0\12\1\1\0" +
    L"\21\1\5\0\3\1\1\0\5\1\1\0\21\1\u032a\0\32\17\1\13" +
    L"\u0dff\0\ua6d7\12\51\0\u1035\12\13\0\336\12\u3fe2\0\u021e\12\uffff\0"
    L"\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0"
    L"\uffff\0\u05ee\0" +
    L"\1\3\36\0\140\3\200\0\360\3\uffff\0\uffff\0\ufe12\0";
std::deque<wchar_t> const StandardTokenizerImpl::ZZ_CMAP =
    zzUnpackCMap(ZZ_CMAP_PACKED);
std::deque<int> const StandardTokenizerImpl::ZZ_ACTION = zzUnpackAction();
const wstring StandardTokenizerImpl::ZZ_ACTION_PACKED_0 =
    wstring(L"\1\0\1\1\1\2\1\3\1\4\1\5\1\1\1\6") +
    L"\1\7\1\2\1\1\1\10\1\2\1\0\1\2\1\0" + L"\1\4\1\0\2\2\2\0\1\1\1\0";

std::deque<int> StandardTokenizerImpl::zzUnpackAction()
{
  std::deque<int> result(24);
  int offset = 0;
  offset = zzUnpackAction(ZZ_ACTION_PACKED_0, offset, result);
  return result;
}

int StandardTokenizerImpl::zzUnpackAction(const wstring &packed, int offset,
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

std::deque<int> const StandardTokenizerImpl::ZZ_ROWMAP = zzUnpackRowMap();
const wstring StandardTokenizerImpl::ZZ_ROWMAP_PACKED_0 =
    wstring(L"\0\0\0\22\0\44\0\66\0\110\0\132\0\154\0\176") +
    L"\0\220\0\242\0\264\0\306\0\330\0\352\0\374\0\u010e" +
    L"\0\u0120\0\154\0\u0132\0\u0144\0\u0156\0\264\0\u0168\0\u017a";

std::deque<int> StandardTokenizerImpl::zzUnpackRowMap()
{
  std::deque<int> result(24);
  int offset = 0;
  offset = zzUnpackRowMap(ZZ_ROWMAP_PACKED_0, offset, result);
  return result;
}

int StandardTokenizerImpl::zzUnpackRowMap(const wstring &packed, int offset,
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

std::deque<int> const StandardTokenizerImpl::ZZ_TRANS = zzUnpackTrans();
const wstring StandardTokenizerImpl::ZZ_TRANS_PACKED_0 =
    wstring(L"\1\2\1\3\1\4\1\2\1\5\1\6\3\2\1\7") +
    L"\1\10\1\11\2\2\1\12\1\13\2\14\23\0\3\3" +
    L"\1\15\1\0\1\16\1\0\1\16\1\17\2\0\1\16" +
    L"\1\0\1\12\2\0\1\3\1\0\1\3\2\4\1\15" +
    L"\1\0\1\16\1\0\1\16\1\17\2\0\1\16\1\0" +
    L"\1\12\2\0\1\4\1\0\2\3\2\5\2\0\2\20" +
    L"\1\21\2\0\1\20\1\0\1\12\2\0\1\5\3\0" +
    L"\1\6\1\0\1\6\3\0\1\17\7\0\1\6\1\0" +
    L"\2\3\1\22\1\5\1\23\3\0\1\22\4\0\1\12" +
    L"\2\0\1\22\3\0\1\10\15\0\1\10\3\0\1\11" +
    L"\15\0\1\11\1\0\2\3\1\12\1\15\1\0\1\16" +
    L"\1\0\1\16\1\17\2\0\1\24\1\25\1\12\2\0" +
    L"\1\12\3\0\1\26\13\0\1\27\1\0\1\26\3\0" +
    L"\1\14\14\0\2\14\1\0\2\3\2\15\2\0\2\30" +
    L"\1\17\2\0\1\30\1\0\1\12\2\0\1\15\1\0" +
    L"\2\3\1\16\12\0\1\3\2\0\1\16\1\0\2\3" +
    L"\1\17\1\15\1\23\3\0\1\17\4\0\1\12\2\0" +
    L"\1\17\3\0\1\20\1\5\14\0\1\20\1\0\2\3" +
    L"\1\21\1\5\1\23\3\0\1\21\4\0\1\12\2\0" +
    L"\1\21\3\0\1\23\1\0\1\23\3\0\1\17\7\0" +
    L"\1\23\1\0\2\3\1\24\1\15\4\0\1\17\4\0" +
    L"\1\12\2\0\1\24\3\0\1\25\12\0\1\24\2\0" +
    L"\1\25\3\0\1\27\13\0\1\27\1\0\1\27\3\0" + L"\1\30\1\15\14\0\1\30";

std::deque<int> StandardTokenizerImpl::zzUnpackTrans()
{
  std::deque<int> result(396);
  int offset = 0;
  offset = zzUnpackTrans(ZZ_TRANS_PACKED_0, offset, result);
  return result;
}

int StandardTokenizerImpl::zzUnpackTrans(const wstring &packed, int offset,
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

std::deque<wstring> const StandardTokenizerImpl::ZZ_ERROR_MSG = {
    L"Unkown internal scanner error", L"Error: could not match input",
    L"Error: pushback value was too large"};
std::deque<int> const StandardTokenizerImpl::ZZ_ATTRIBUTE =
    zzUnpackAttribute();
const wstring StandardTokenizerImpl::ZZ_ATTRIBUTE_PACKED_0 =
    wstring(L"\1\0\1\11\13\1\1\0\1\1\1\0\1\1\1\0") + L"\2\1\2\0\1\1\1\0";

std::deque<int> StandardTokenizerImpl::zzUnpackAttribute()
{
  std::deque<int> result(24);
  int offset = 0;
  offset = zzUnpackAttribute(ZZ_ATTRIBUTE_PACKED_0, offset, result);
  return result;
}

int StandardTokenizerImpl::zzUnpackAttribute(const wstring &packed, int offset,
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

int StandardTokenizerImpl::yychar() { return yychar_; }

void StandardTokenizerImpl::getText(shared_ptr<CharTermAttribute> t)
{
  t->copyBuffer(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

void StandardTokenizerImpl::setBufferSize(int numChars)
{
  ZZ_BUFFERSIZE = numChars;
  std::deque<wchar_t> newZzBuffer(ZZ_BUFFERSIZE);
  System::arraycopy(zzBuffer, 0, newZzBuffer, 0,
                    min(zzBuffer.size(), ZZ_BUFFERSIZE));
  zzBuffer = newZzBuffer;
}

StandardTokenizerImpl::StandardTokenizerImpl(shared_ptr<java::io::Reader> in_)
{
  this->zzReader = in_;
}

std::deque<wchar_t> StandardTokenizerImpl::zzUnpackCMap(const wstring &packed)
{
  std::deque<wchar_t> map_obj(0x110000);
  int i = 0; // index in packed string
  int j = 0; // index in unpacked array
  while (i < 2836) {
    int count = packed[i++];
    wchar_t value = packed[i++];
    do {
      map_obj[j++] = value;
    } while (--count > 0);
  }
  return map_obj;
}

bool StandardTokenizerImpl::zzRefill() 
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

  /* fill the buffer with new input */
  int requested = zzBuffer.size() - zzEndRead - zzFinalHighSurrogate;
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
        if (totalRead == 1) {
          return true;
        }
      }
    }
    return false;
  }

  // totalRead = 0: End of stream
  return true;
}

void StandardTokenizerImpl::yyclose() 
{
  zzAtEOF = true;          // indicate end of file
  zzEndRead = zzStartRead; // invalidate buffer

  if (zzReader != nullptr) {
    zzReader->close();
  }
}

void StandardTokenizerImpl::yyreset(shared_ptr<java::io::Reader> reader)
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

int StandardTokenizerImpl::yystate() { return zzLexicalState; }

void StandardTokenizerImpl::yybegin(int newState) { zzLexicalState = newState; }

wstring StandardTokenizerImpl::yytext()
{
  return wstring(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

wchar_t StandardTokenizerImpl::yycharat(int pos)
{
  return zzBuffer[zzStartRead + pos];
}

int StandardTokenizerImpl::yylength() { return zzMarkedPos - zzStartRead; }

void StandardTokenizerImpl::zzScanError(int errorCode)
{
  wstring message;
  try {
    message = ZZ_ERROR_MSG[errorCode];
  } catch (const out_of_range &e) {
    message = ZZ_ERROR_MSG[ZZ_UNKNOWN_ERROR];
  }

  throw make_shared<Error>(message);
}

void StandardTokenizerImpl::yypushback(int number)
{
  if (number > yylength()) {
    zzScanError(ZZ_PUSHBACK_2BIG);
  }

  zzMarkedPos -= number;
}

int StandardTokenizerImpl::getNextToken() 
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
      break; // Not numeric, word, ideographic, hiragana, or SE Asian -- ignore
             // it.
    }
    case 9:
      break;
    case 2: {
      return WORD_TYPE;
    }
    case 10:
      break;
    case 3: {
      return HANGUL_TYPE;
    }
    case 11:
      break;
    case 4: {
      return NUMERIC_TYPE;
    }
    case 12:
      break;
    case 5: {
      return KATAKANA_TYPE;
    }
    case 13:
      break;
    case 6: {
      return IDEOGRAPHIC_TYPE;
    }
    case 14:
      break;
    case 7: {
      return HIRAGANA_TYPE;
    }
    case 15:
      break;
    case 8: {
      return SOUTH_EAST_ASIAN_TYPE;
    }
    case 16:
      break;
    default:
      if (zzInput == YYEOF && zzStartRead == zzCurrentPos) {
        zzAtEOF = true;
        {
          return YYEOF;
        }
      } else {
        zzScanError(ZZ_NO_MATCH);
      }
    }
  }
}
} // namespace org::apache::lucene::analysis::standard