using namespace std;

#include "WikipediaTokenizerImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::wikipedia
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
std::deque<int> const WikipediaTokenizerImpl::ZZ_LEXSTATE = {
    0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9};
const wstring WikipediaTokenizerImpl::ZZ_CMAP_PACKED =
    wstring(L"\11\0\1\24\1\23\1\0\1\24\1\22\22\0\1\24\1\0\1\12") +
    L"\1\53\2\0\1\3\1\1\4\0\1\14\1\5\1\2\1\10\12\16" +
    L"\1\27\1\0\1\7\1\11\1\13\1\53\1\4\2\15\1\30\5\15" +
    L"\1\41\21\15\1\25\1\0\1\26\1\0\1\6\1\0\1\31\1\43" +
    L"\2\15\1\33\1\40\1\34\1\50\1\41\4\15\1\42\1\35\1\51" +
    L"\1\15\1\36\1\52\1\32\3\15\1\44\1\37\1\15\1\45\1\47" +
    L"\1\46\102\0\27\15\1\0\37\15\1\0\u0568\15\12\17\206\15\12\17" +
    L"\u026c\15\12\17\166\15\12\17\166\15\12\17\166\15\12\17\166\15\12\17" +
    L"\167\15\11\17\166\15\12\17\166\15\12\17\166\15\12\17\340\15\12\17" +
    L"\166\15\12\17\u0166\15\12\17\266\15\u0100\15\u0e00\15\u1040\0\u0150\21"
    L"\140\0" +
    L"\20\21\u0100\0\200\21\200\0\u19c0\21\100\0\u5200\21\u0c00\0\u2bb0\20"
    L"\u2150\0" +
    L"\u0200\21\u0465\0\73\21\75\15\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0"
    L"\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0\uffff\0"
    L"\uffff\0\uffff\0\63\0";
std::deque<wchar_t> const WikipediaTokenizerImpl::ZZ_CMAP =
    zzUnpackCMap(ZZ_CMAP_PACKED);
std::deque<int> const WikipediaTokenizerImpl::ZZ_ACTION = zzUnpackAction();
const wstring WikipediaTokenizerImpl::ZZ_ACTION_PACKED_0 =
    wstring(L"\12\0\4\1\4\2\1\3\1\4\1\1\2\5\1\6") +
    L"\1\5\1\7\1\5\2\10\1\11\1\5\1\12\1\11" +
    L"\1\13\1\14\1\15\1\16\1\15\1\17\1\20\1\10" +
    L"\1\21\1\10\4\22\1\23\1\24\1\25\1\26\3\0" +
    L"\1\27\14\0\1\30\1\31\1\32\1\33\1\11\1\0" +
    L"\1\34\1\35\1\36\1\0\1\37\1\0\1\40\3\0" +
    L"\1\41\1\42\2\43\1\42\2\44\2\0\1\43\1\0" +
    L"\14\43\1\42\3\0\1\11\1\45\3\0\1\46\1\47" +
    L"\5\0\1\50\4\0\1\50\2\0\2\50\2\0\1\11" +
    L"\5\0\1\31\1\42\1\43\1\51\3\0\1\11\2\0" +
    L"\1\52\30\0\1\53\2\0\1\54\1\55\1\56";

std::deque<int> WikipediaTokenizerImpl::zzUnpackAction()
{
  std::deque<int> result(181);
  int offset = 0;
  offset = zzUnpackAction(ZZ_ACTION_PACKED_0, offset, result);
  return result;
}

int WikipediaTokenizerImpl::zzUnpackAction(const wstring &packed, int offset,
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

std::deque<int> const WikipediaTokenizerImpl::ZZ_ROWMAP = zzUnpackRowMap();
const wstring WikipediaTokenizerImpl::ZZ_ROWMAP_PACKED_0 =
    wstring(L"\0\0\0\54\0\130\0\204\0\260\0\334\0\u0108\0\u0134") +
    L"\0\u0160\0\u018c\0\u01b8\0\u01e4\0\u0210\0\u023c\0\u0268\0\u0294" +
    L"\0\u02c0\0\u02ec\0\u01b8\0\u0318\0\u0344\0\u01b8\0\u0370\0\u039c" +
    L"\0\u03c8\0\u03f4\0\u0420\0\u01b8\0\u0370\0\u044c\0\u0478\0\u01b8" +
    L"\0\u04a4\0\u04d0\0\u04fc\0\u0528\0\u0554\0\u0580\0\u05ac\0\u05d8" +
    L"\0\u0604\0\u0630\0\u065c\0\u01b8\0\u0688\0\u0370\0\u06b4\0\u06e0" +
    L"\0\u070c\0\u01b8\0\u01b8\0\u0738\0\u0764\0\u0790\0\u01b8\0\u07bc" +
    L"\0\u07e8\0\u0814\0\u0840\0\u086c\0\u0898\0\u08c4\0\u08f0\0\u091c" +
    L"\0\u0948\0\u0974\0\u09a0\0\u09cc\0\u09f8\0\u01b8\0\u01b8\0\u0a24" +
    L"\0\u0a50\0\u0a7c\0\u0a7c\0\u01b8\0\u0aa8\0\u0ad4\0\u0b00\0\u0b2c" +
    L"\0\u0b58\0\u0b84\0\u0bb0\0\u0bdc\0\u0c08\0\u0c34\0\u0c60\0\u0c8c" +
    L"\0\u0814\0\u0cb8\0\u0ce4\0\u0d10\0\u0d3c\0\u0d68\0\u0d94\0\u0dc0" +
    L"\0\u0dec\0\u0e18\0\u0e44\0\u0e70\0\u0e9c\0\u0ec8\0\u0ef4\0\u0f20" +
    L"\0\u0f4c\0\u0f78\0\u0fa4\0\u0fd0\0\u0ffc\0\u1028\0\u1054\0\u01b8" +
    L"\0\u1080\0\u10ac\0\u10d8\0\u1104\0\u01b8\0\u1130\0\u115c\0\u1188" +
    L"\0\u11b4\0\u11e0\0\u120c\0\u1238\0\u1264\0\u1290\0\u12bc\0\u12e8" +
    L"\0\u1314\0\u1340\0\u07e8\0\u0974\0\u136c\0\u1398\0\u13c4\0\u13f0" +
    L"\0\u141c\0\u1448\0\u1474\0\u14a0\0\u01b8\0\u14cc\0\u14f8\0\u1524" +
    L"\0\u1550\0\u157c\0\u15a8\0\u15d4\0\u1600\0\u162c\0\u01b8\0\u1658" +
    L"\0\u1684\0\u16b0\0\u16dc\0\u1708\0\u1734\0\u1760\0\u178c\0\u17b8" +
    L"\0\u17e4\0\u1810\0\u183c\0\u1868\0\u1894\0\u18c0\0\u18ec\0\u1918" +
    L"\0\u1944\0\u1970\0\u199c\0\u19c8\0\u19f4\0\u1a20\0\u1a4c\0\u1a78" +
    L"\0\u1aa4\0\u1ad0\0\u01b8\0\u01b8\0\u01b8";

std::deque<int> WikipediaTokenizerImpl::zzUnpackRowMap()
{
  std::deque<int> result(181);
  int offset = 0;
  offset = zzUnpackRowMap(ZZ_ROWMAP_PACKED_0, offset, result);
  return result;
}

int WikipediaTokenizerImpl::zzUnpackRowMap(const wstring &packed, int offset,
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

std::deque<int> const WikipediaTokenizerImpl::ZZ_TRANS = zzUnpackTrans();
const wstring WikipediaTokenizerImpl::ZZ_TRANS_PACKED_0 =
    wstring(L"\1\13\1\14\5\13\1\15\1\13\1\16\3\13\1\17") +
    L"\1\20\1\21\1\22\1\23\3\13\1\24\2\13\15\17" +
    L"\1\25\2\13\3\17\1\13\7\26\1\27\5\26\4\30" +
    L"\5\26\1\31\1\26\15\30\3\26\3\30\10\26\1\27" +
    L"\5\26\4\32\5\26\1\33\1\26\15\32\3\26\3\32" +
    L"\1\26\7\34\1\35\5\34\4\36\1\34\1\37\2\26" +
    L"\1\34\1\40\1\34\15\36\3\34\1\41\2\36\2\34" +
    L"\1\42\5\34\1\35\5\34\4\43\4\34\1\44\2\34" +
    L"\15\43\3\34\3\43\10\34\1\35\5\34\4\45\4\34" +
    L"\1\44\2\34\15\45\3\34\3\45\10\34\1\35\5\34" +
    L"\4\45\4\34\1\46\2\34\15\45\3\34\3\45\10\34" +
    L"\1\35\1\34\1\47\3\34\4\50\7\34\15\50\3\34" +
    L"\3\50\10\34\1\51\5\34\4\52\7\34\15\52\1\34" +
    L"\1\53\1\34\3\52\1\34\1\54\1\55\5\54\1\56" +
    L"\1\54\1\57\3\54\4\60\4\54\1\61\2\54\15\60" +
    L"\2\54\1\62\3\60\1\54\55\0\1\63\62\0\1\64" +
    L"\4\0\4\65\7\0\6\65\1\66\6\65\3\0\3\65" +
    L"\12\0\1\67\43\0\1\70\1\71\1\72\1\73\2\74" +
    L"\1\0\1\75\3\0\1\75\1\17\1\20\1\21\1\22" +
    L"\7\0\15\17\3\0\3\17\3\0\1\76\1\0\1\77" +
    L"\2\100\1\0\1\101\3\0\1\101\3\20\1\22\7\0" +
    L"\15\20\3\0\3\20\2\0\1\70\1\102\1\72\1\73" +
    L"\2\100\1\0\1\101\3\0\1\101\1\21\1\20\1\21" +
    L"\1\22\7\0\15\21\3\0\3\21\3\0\1\103\1\0" +
    L"\1\77\2\74\1\0\1\75\3\0\1\75\4\22\7\0" +
    L"\15\22\3\0\3\22\26\0\1\104\73\0\1\105\16\0" +
    L"\1\64\4\0\4\65\7\0\15\65\3\0\3\65\16\0" +
    L"\4\30\7\0\15\30\3\0\3\30\27\0\1\106\42\0" +
    L"\4\32\7\0\15\32\3\0\3\32\27\0\1\107\42\0" +
    L"\4\36\7\0\15\36\3\0\3\36\24\0\1\26\45\0" +
    L"\4\36\7\0\2\36\1\110\12\36\3\0\3\36\2\0" +
    L"\1\111\67\0\4\43\7\0\15\43\3\0\3\43\26\0" +
    L"\1\112\43\0\4\45\7\0\15\45\3\0\3\45\26\0" +
    L"\1\113\37\0\1\114\57\0\4\50\7\0\15\50\3\0" +
    L"\3\50\11\0\1\115\4\0\4\65\7\0\15\65\3\0" +
    L"\3\65\16\0\4\52\7\0\15\52\3\0\3\52\47\0" +
    L"\1\114\6\0\1\116\63\0\1\117\57\0\4\60\7\0" +
    L"\15\60\3\0\3\60\26\0\1\120\43\0\4\65\7\0" +
    L"\15\65\3\0\3\65\14\0\1\34\1\0\4\121\1\0" +
    L"\3\122\3\0\15\121\3\0\3\121\14\0\1\34\1\0" +
    L"\4\121\1\0\3\122\3\0\3\121\1\123\11\121\3\0" +
    L"\3\121\16\0\1\124\1\0\1\124\10\0\15\124\3\0" +
    L"\3\124\16\0\1\125\1\126\1\127\1\130\7\0\15\125" +
    L"\3\0\3\125\16\0\1\131\1\0\1\131\10\0\15\131" +
    L"\3\0\3\131\16\0\1\132\1\133\1\132\1\133\7\0" +
    L"\15\132\3\0\3\132\16\0\1\134\2\135\1\136\7\0" +
    L"\15\134\3\0\3\134\16\0\1\75\2\137\10\0\15\75" +
    L"\3\0\3\75\16\0\1\140\2\141\1\142\7\0\15\140" +
    L"\3\0\3\140\16\0\4\133\7\0\15\133\3\0\3\133" +
    L"\16\0\1\143\2\144\1\145\7\0\15\143\3\0\3\143" +
    L"\16\0\1\146\2\147\1\150\7\0\15\146\3\0\3\146" +
    L"\16\0\1\151\1\141\1\152\1\142\7\0\15\151\3\0" +
    L"\3\151\16\0\1\153\2\126\1\130\7\0\15\153\3\0" +
    L"\3\153\30\0\1\154\1\155\64\0\1\156\27\0\4\36" +
    L"\7\0\2\36\1\157\12\36\3\0\3\36\2\0\1\160" +
    L"\101\0\1\161\1\162\40\0\4\65\7\0\6\65\1\163" +
    L"\6\65\3\0\3\65\2\0\1\164\63\0\1\165\71\0" +
    L"\1\166\1\167\34\0\1\170\1\0\1\34\1\0\4\121" +
    L"\1\0\3\122\3\0\15\121\3\0\3\121\16\0\4\171" +
    L"\1\0\3\122\3\0\15\171\3\0\3\171\12\0\1\170" +
    L"\1\0\1\34\1\0\4\121\1\0\3\122\3\0\10\121" +
    L"\1\172\4\121\3\0\3\121\2\0\1\70\13\0\1\124" +
    L"\1\0\1\124\10\0\15\124\3\0\3\124\3\0\1\173" +
    L"\1\0\1\77\2\174\6\0\1\125\1\126\1\127\1\130" +
    L"\7\0\15\125\3\0\3\125\3\0\1\175\1\0\1\77" +
    L"\2\176\1\0\1\177\3\0\1\177\3\126\1\130\7\0" +
    L"\15\126\3\0\3\126\3\0\1\200\1\0\1\77\2\176" +
    L"\1\0\1\177\3\0\1\177\1\127\1\126\1\127\1\130" +
    L"\7\0\15\127\3\0\3\127\3\0\1\201\1\0\1\77" +
    L"\2\174\6\0\4\130\7\0\15\130\3\0\3\130\3\0" +
    L"\1\202\2\0\1\202\7\0\1\132\1\133\1\132\1\133" +
    L"\7\0\15\132\3\0\3\132\3\0\1\202\2\0\1\202" +
    L"\7\0\4\133\7\0\15\133\3\0\3\133\3\0\1\174" +
    L"\1\0\1\77\2\174\6\0\1\134\2\135\1\136\7\0" +
    L"\15\134\3\0\3\134\3\0\1\176\1\0\1\77\2\176" +
    L"\1\0\1\177\3\0\1\177\3\135\1\136\7\0\15\135" +
    L"\3\0\3\135\3\0\1\174\1\0\1\77\2\174\6\0" +
    L"\4\136\7\0\15\136\3\0\3\136\3\0\1\177\2\0" +
    L"\2\177\1\0\1\177\3\0\1\177\3\137\10\0\15\137" +
    L"\3\0\3\137\3\0\1\103\1\0\1\77\2\74\1\0" +
    L"\1\75\3\0\1\75\1\140\2\141\1\142\7\0\15\140" +
    L"\3\0\3\140\3\0\1\76\1\0\1\77\2\100\1\0" +
    L"\1\101\3\0\1\101\3\141\1\142\7\0\15\141\3\0" +
    L"\3\141\3\0\1\103\1\0\1\77\2\74\1\0\1\75" +
    L"\3\0\1\75\4\142\7\0\15\142\3\0\3\142\3\0" +
    L"\1\74\1\0\1\77\2\74\1\0\1\75\3\0\1\75" +
    L"\1\143\2\144\1\145\7\0\15\143\3\0\3\143\3\0" +
    L"\1\100\1\0\1\77\2\100\1\0\1\101\3\0\1\101" +
    L"\3\144\1\145\7\0\15\144\3\0\3\144\3\0\1\74" +
    L"\1\0\1\77\2\74\1\0\1\75\3\0\1\75\4\145" +
    L"\7\0\15\145\3\0\3\145\3\0\1\75\2\0\2\75" +
    L"\1\0\1\75\3\0\1\75\1\146\2\147\1\150\7\0" +
    L"\15\146\3\0\3\146\3\0\1\101\2\0\2\101\1\0" +
    L"\1\101\3\0\1\101\3\147\1\150\7\0\15\147\3\0" +
    L"\3\147\3\0\1\75\2\0\2\75\1\0\1\75\3\0" +
    L"\1\75\4\150\7\0\15\150\3\0\3\150\3\0\1\203" +
    L"\1\0\1\77\2\74\1\0\1\75\3\0\1\75\1\151" +
    L"\1\141\1\152\1\142\7\0\15\151\3\0\3\151\3\0" +
    L"\1\204\1\0\1\77\2\100\1\0\1\101\3\0\1\101" +
    L"\1\152\1\141\1\152\1\142\7\0\15\152\3\0\3\152" +
    L"\3\0\1\201\1\0\1\77\2\174\6\0\1\153\2\126" +
    L"\1\130\7\0\15\153\3\0\3\153\31\0\1\155\54\0" +
    L"\1\205\64\0\1\206\26\0\4\36\7\0\15\36\3\0" +
    L"\1\36\1\207\1\36\31\0\1\162\54\0\1\210\35\0" +
    L"\1\34\1\0\4\121\1\0\3\122\3\0\3\121\1\211" +
    L"\11\121\3\0\3\121\2\0\1\212\102\0\1\167\54\0" +
    L"\1\213\34\0\1\214\52\0\1\170\3\0\4\171\7\0" +
    L"\15\171\3\0\3\171\12\0\1\170\1\0\1\215\1\0" +
    L"\4\121\1\0\3\122\3\0\15\121\3\0\3\121\16\0" +
    L"\1\216\1\130\1\216\1\130\7\0\15\216\3\0\3\216" +
    L"\16\0\4\136\7\0\15\136\3\0\3\136\16\0\4\142" +
    L"\7\0\15\142\3\0\3\142\16\0\4\145\7\0\15\145" +
    L"\3\0\3\145\16\0\4\150\7\0\15\150\3\0\3\150" +
    L"\16\0\1\217\1\142\1\217\1\142\7\0\15\217\3\0" +
    L"\3\217\16\0\4\130\7\0\15\130\3\0\3\130\16\0" +
    L"\4\220\7\0\15\220\3\0\3\220\33\0\1\221\61\0" +
    L"\1\222\30\0\4\36\6\0\1\223\15\36\3\0\2\36" +
    L"\1\224\33\0\1\225\32\0\1\170\1\0\1\34\1\0" +
    L"\4\121\1\0\3\122\3\0\10\121\1\226\4\121\3\0" +
    L"\3\121\2\0\1\227\104\0\1\230\36\0\4\231\7\0" +
    L"\15\231\3\0\3\231\3\0\1\173\1\0\1\77\2\174" +
    L"\6\0\1\216\1\130\1\216\1\130\7\0\15\216\3\0" +
    L"\3\216\3\0\1\203\1\0\1\77\2\74\1\0\1\75" +
    L"\3\0\1\75\1\217\1\142\1\217\1\142\7\0\15\217" +
    L"\3\0\3\217\3\0\1\202\2\0\1\202\7\0\4\220" +
    L"\7\0\15\220\3\0\3\220\34\0\1\232\55\0\1\233" +
    L"\26\0\1\234\60\0\4\36\6\0\1\223\15\36\3\0" +
    L"\3\36\34\0\1\235\31\0\1\170\1\0\1\114\1\0" +
    L"\4\121\1\0\3\122\3\0\15\121\3\0\3\121\34\0" +
    L"\1\236\32\0\1\237\2\0\4\231\7\0\15\231\3\0" +
    L"\3\231\35\0\1\240\62\0\1\241\20\0\1\242\77\0" +
    L"\1\243\53\0\1\244\32\0\1\34\1\0\4\171\1\0" +
    L"\3\122\3\0\15\171\3\0\3\171\36\0\1\245\53\0" +
    L"\1\246\33\0\4\247\7\0\15\247\3\0\3\247\36\0" +
    L"\1\250\53\0\1\251\54\0\1\252\61\0\1\253\11\0" +
    L"\1\254\12\0\4\247\7\0\15\247\3\0\3\247\37\0" +
    L"\1\255\53\0\1\256\54\0\1\257\22\0\1\13\62\0" +
    L"\4\260\7\0\15\260\3\0\3\260\40\0\1\261\53\0" +
    L"\1\262\43\0\1\263\26\0\2\260\1\0\2\260\1\0" +
    L"\2\260\2\0\5\260\7\0\15\260\3\0\4\260\27\0" + L"\1\264\53\0\1\265\24\0";

std::deque<int> WikipediaTokenizerImpl::zzUnpackTrans()
{
  std::deque<int> result(6908);
  int offset = 0;
  offset = zzUnpackTrans(ZZ_TRANS_PACKED_0, offset, result);
  return result;
}

int WikipediaTokenizerImpl::zzUnpackTrans(const wstring &packed, int offset,
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

std::deque<wstring> const WikipediaTokenizerImpl::ZZ_ERROR_MSG = {
    L"Unkown internal scanner error", L"Error: could not match input",
    L"Error: pushback value was too large"};
std::deque<int> const WikipediaTokenizerImpl::ZZ_ATTRIBUTE =
    zzUnpackAttribute();
const wstring WikipediaTokenizerImpl::ZZ_ATTRIBUTE_PACKED_0 =
    wstring(L"\12\0\1\11\7\1\1\11\2\1\1\11\5\1\1\11") +
    L"\3\1\1\11\13\1\1\11\5\1\2\11\3\0\1\11" +
    L"\14\0\2\1\2\11\1\1\1\0\2\1\1\11\1\0" +
    L"\1\1\1\0\1\1\3\0\7\1\2\0\1\1\1\0" +
    L"\15\1\3\0\1\1\1\11\3\0\1\1\1\11\5\0" +
    L"\1\1\4\0\1\1\2\0\2\1\2\0\1\1\5\0" +
    L"\1\11\3\1\3\0\1\1\2\0\1\11\30\0\1\1" + L"\2\0\3\11";

std::deque<int> WikipediaTokenizerImpl::zzUnpackAttribute()
{
  std::deque<int> result(181);
  int offset = 0;
  offset = zzUnpackAttribute(ZZ_ATTRIBUTE_PACKED_0, offset, result);
  return result;
}

int WikipediaTokenizerImpl::zzUnpackAttribute(const wstring &packed, int offset,
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

std::deque<wstring> const WikipediaTokenizerImpl::TOKEN_TYPES =
    WikipediaTokenizer::TOKEN_TYPES;

int WikipediaTokenizerImpl::getNumWikiTokensSeen() { return numWikiTokensSeen; }

int WikipediaTokenizerImpl::yychar() { return yychar_; }

int WikipediaTokenizerImpl::getPositionIncrement() { return positionInc; }

void WikipediaTokenizerImpl::getText(shared_ptr<CharTermAttribute> t)
{
  t->copyBuffer(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

int WikipediaTokenizerImpl::setText(shared_ptr<StringBuilder> buffer)
{
  int length = zzMarkedPos - zzStartRead;
  buffer->append(zzBuffer, zzStartRead, length);
  return length;
}

void WikipediaTokenizerImpl::reset()
{
  currentTokType = 0;
  numBalanced = 0;
  positionInc = 1;
  numLinkToks = 0;
  numWikiTokensSeen = 0;
}

WikipediaTokenizerImpl::WikipediaTokenizerImpl(shared_ptr<java::io::Reader> in_)
{
  this->zzReader = in_;
}

std::deque<wchar_t> WikipediaTokenizerImpl::zzUnpackCMap(const wstring &packed)
{
  std::deque<wchar_t> map_obj(0x110000);
  int i = 0; // index in packed string
  int j = 0; // index in unpacked array
  while (i < 262) {
    int count = packed[i++];
    wchar_t value = packed[i++];
    do {
      map_obj[j++] = value;
    } while (--count > 0);
  }
  return map_obj;
}

bool WikipediaTokenizerImpl::zzRefill() 
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

void WikipediaTokenizerImpl::yyclose() 
{
  zzAtEOF = true;          // indicate end of file
  zzEndRead = zzStartRead; // invalidate buffer

  if (zzReader != nullptr) {
    zzReader->close();
  }
}

void WikipediaTokenizerImpl::yyreset(shared_ptr<java::io::Reader> reader)
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

int WikipediaTokenizerImpl::yystate() { return zzLexicalState; }

void WikipediaTokenizerImpl::yybegin(int newState)
{
  zzLexicalState = newState;
}

wstring WikipediaTokenizerImpl::yytext()
{
  return wstring(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
}

wchar_t WikipediaTokenizerImpl::yycharat(int pos)
{
  return zzBuffer[zzStartRead + pos];
}

int WikipediaTokenizerImpl::yylength() { return zzMarkedPos - zzStartRead; }

void WikipediaTokenizerImpl::zzScanError(int errorCode)
{
  wstring message;
  try {
    message = ZZ_ERROR_MSG[errorCode];
  } catch (const out_of_range &e) {
    message = ZZ_ERROR_MSG[ZZ_UNKNOWN_ERROR];
  }

  throw make_shared<Error>(message);
}

void WikipediaTokenizerImpl::yypushback(int number)
{
  if (number > yylength()) {
    zzScanError(ZZ_PUSHBACK_2BIG);
  }

  zzMarkedPos -= number;
}

int WikipediaTokenizerImpl::getNextToken() 
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
      numWikiTokensSeen = 0;
      positionInc = 1;
      break;
    }
    case 47:
      break;
    case 2: {
      positionInc = 1;
      return ALPHANUM;
    }
    case 48:
      break;
    case 3: {
      positionInc = 1;
      return CJ;
    }
    case 49:
      break;
    case 4: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      currentTokType = EXTERNAL_LINK_URL;
      yybegin(EXTERNAL_LINK_STATE);
      break;
    }
    case 50:
      break;
    case 5: {
      positionInc = 1;
      break;
    }
    case 51:
      break;
    case 6: {
      yybegin(CATEGORY_STATE);
      numWikiTokensSeen++;
      return currentTokType;
    }
    case 52:
      break;
    case 7: {
      yybegin(INTERNAL_LINK_STATE);
      numWikiTokensSeen++;
      return currentTokType;
    }
    case 53:
      break;
    case 8: {
      break; // ignore
    }
    case 54:
      break;
    case 9: {
      if (numLinkToks == 0) {
        positionInc = 0;
      } else {
        positionInc = 1;
      }
      numWikiTokensSeen++;
      currentTokType = EXTERNAL_LINK;
      yybegin(EXTERNAL_LINK_STATE);
      numLinkToks++;
      return currentTokType;
    }
    case 55:
      break;
    case 10: {
      numLinkToks = 0;
      positionInc = 0;
      yybegin(YYINITIAL);
      break;
    }
    case 56:
      break;
    case 11: {
      currentTokType = BOLD;
      yybegin(THREE_SINGLE_QUOTES_STATE);
      break;
    }
    case 57:
      break;
    case 12: {
      currentTokType = ITALICS;
      numWikiTokensSeen++;
      yybegin(STRING);
      return currentTokType; // italics
    }
    case 58:
      break;
    case 13: {
      currentTokType = EXTERNAL_LINK;
      numWikiTokensSeen = 0;
      yybegin(EXTERNAL_LINK_STATE);
      break;
    }
    case 59:
      break;
    case 14: {
      yybegin(STRING);
      numWikiTokensSeen++;
      return currentTokType;
    }
    case 60:
      break;
    case 15: {
      currentTokType = SUB_HEADING;
      numWikiTokensSeen = 0;
      yybegin(STRING);
      break;
    }
    case 61:
      break;
    case 16: {
      currentTokType = HEADING;
      yybegin(DOUBLE_EQUALS_STATE);
      numWikiTokensSeen++;
      return currentTokType;
    }
    case 62:
      break;
    case 17: {
      yybegin(DOUBLE_BRACE_STATE);
      numWikiTokensSeen = 0;
      return currentTokType;
    }
    case 63:
      break;
    case 18: {
      break; // ignore STRING
    }
    case 64:
      break;
    case 19: {
      yybegin(STRING);
      numWikiTokensSeen++;
      return currentTokType; // STRING ALPHANUM
    }
    case 65:
      break;
    case 20: {
      numBalanced = 0;
      numWikiTokensSeen = 0;
      currentTokType = EXTERNAL_LINK;
      yybegin(EXTERNAL_LINK_STATE);
      break;
    }
    case 66:
      break;
    case 21: {
      yybegin(STRING);
      return currentTokType; // pipe
    }
    case 67:
      break;
    case 22: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      if (numBalanced == 0) {
        numBalanced++;
        yybegin(TWO_SINGLE_QUOTES_STATE);
      } else {
        numBalanced = 0;
      }
      break;
    }
    case 68:
      break;
    case 23: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      yybegin(DOUBLE_EQUALS_STATE);
      break;
    }
    case 69:
      break;
    case 24: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      currentTokType = INTERNAL_LINK;
      yybegin(INTERNAL_LINK_STATE);
      break;
    }
    case 70:
      break;
    case 25: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      currentTokType = CITATION;
      yybegin(DOUBLE_BRACE_STATE);
      break;
    }
    case 71:
      break;
    case 26: {
      yybegin(YYINITIAL);
      break;
    }
    case 72:
      break;
    case 27: {
      numLinkToks = 0;
      yybegin(YYINITIAL);
      break;
    }
    case 73:
      break;
    case 28: {
      currentTokType = INTERNAL_LINK;
      numWikiTokensSeen = 0;
      yybegin(INTERNAL_LINK_STATE);
      break;
    }
    case 74:
      break;
    case 29: {
      currentTokType = INTERNAL_LINK;
      numWikiTokensSeen = 0;
      yybegin(INTERNAL_LINK_STATE);
      break;
    }
    case 75:
      break;
    case 30: {
      yybegin(YYINITIAL);
      break;
    }
    case 76:
      break;
    case 31: {
      numBalanced = 0;
      currentTokType = ALPHANUM;
      yybegin(YYINITIAL);
      break; // end italics
    }
    case 77:
      break;
    case 32: {
      numBalanced = 0;
      numWikiTokensSeen = 0;
      currentTokType = INTERNAL_LINK;
      yybegin(INTERNAL_LINK_STATE);
      break;
    }
    case 78:
      break;
    case 33: {
      positionInc = 1;
      return APOSTROPHE;
    }
    case 79:
      break;
    case 34: {
      positionInc = 1;
      return HOST;
    }
    case 80:
      break;
    case 35: {
      positionInc = 1;
      return NUM;
    }
    case 81:
      break;
    case 36: {
      positionInc = 1;
      return COMPANY;
    }
    case 82:
      break;
    case 37: {
      currentTokType = BOLD_ITALICS;
      yybegin(FIVE_SINGLE_QUOTES_STATE);
      break;
    }
    case 83:
      break;
    case 38: {
      numBalanced = 0;
      currentTokType = ALPHANUM;
      yybegin(YYINITIAL);
      break; // end bold
    }
    case 84:
      break;
    case 39: {
      numBalanced = 0;
      currentTokType = ALPHANUM;
      yybegin(YYINITIAL);
      break; // end sub header
    }
    case 85:
      break;
    case 40: {
      positionInc = 1;
      return ACRONYM;
    }
    case 86:
      break;
    case 41: {
      positionInc = 1;
      return EMAIL;
    }
    case 87:
      break;
    case 42: {
      numBalanced = 0;
      currentTokType = ALPHANUM;
      yybegin(YYINITIAL);
      break; // end bold italics
    }
    case 88:
      break;
    case 43: {
      positionInc = 1;
      numWikiTokensSeen++;
      yybegin(EXTERNAL_LINK_STATE);
      return currentTokType;
    }
    case 89:
      break;
    case 44: {
      numWikiTokensSeen = 0;
      positionInc = 1;
      currentTokType = CATEGORY;
      yybegin(CATEGORY_STATE);
      break;
    }
    case 90:
      break;
    case 45: {
      currentTokType = CATEGORY;
      numWikiTokensSeen = 0;
      yybegin(CATEGORY_STATE);
      break;
    }
    case 91:
      break;
    case 46: {
      numBalanced = 0;
      numWikiTokensSeen = 0;
      currentTokType = CATEGORY;
      yybegin(CATEGORY_STATE);
      break;
    }
    case 92:
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
} // namespace org::apache::lucene::analysis::wikipedia