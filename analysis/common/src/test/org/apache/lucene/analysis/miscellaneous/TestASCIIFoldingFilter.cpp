using namespace std;

#include "TestASCIIFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

void TestASCIIFoldingFilter::assertNextTerms(
    const wstring &expectedUnfolded, const wstring &expectedFolded,
    shared_ptr<ASCIIFoldingFilter> filter,
    shared_ptr<CharTermAttribute> termAtt) 
{
  assertTrue(filter->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedFolded, termAtt->toString());
  if (filter->isPreserveOriginal() && expectedUnfolded != expectedFolded) {
    assertTrue(filter->incrementToken());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(expectedUnfolded, termAtt->toString());
  }
}

void TestASCIIFoldingFilter::testLatin1Accents() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(
      wstring(
          L"Des mot clés À LA CHAÎNE À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï Ĳ Ð Ñ") +
      L" Ò Ó Ô Õ Ö Ø Œ Þ Ù Ú Û Ü Ý Ÿ à á â ã ä å æ ç è é ê ë ì í î ï ĳ" +
      L" ð ñ ò ó ô õ ö ø œ ß þ ù ú û ü ý ÿ ﬁ ﬂ");
  shared_ptr<ASCIIFoldingFilter> filter =
      make_shared<ASCIIFoldingFilter>(stream, random()->nextBoolean());

  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  filter->reset();
  assertNextTerms(L"Des", L"Des", filter, termAtt);
  assertNextTerms(L"mot", L"mot", filter, termAtt);
  assertNextTerms(L"clés", L"cles", filter, termAtt);
  assertNextTerms(L"À", L"A", filter, termAtt);
  assertNextTerms(L"LA", L"LA", filter, termAtt);
  assertNextTerms(L"CHAÎNE", L"CHAINE", filter, termAtt);
  assertNextTerms(L"À", L"A", filter, termAtt);
  assertNextTerms(L"Á", L"A", filter, termAtt);
  assertNextTerms(L"Â", L"A", filter, termAtt);
  assertNextTerms(L"Ã", L"A", filter, termAtt);
  assertNextTerms(L"Ä", L"A", filter, termAtt);
  assertNextTerms(L"Å", L"A", filter, termAtt);
  assertNextTerms(L"Æ", L"AE", filter, termAtt);
  assertNextTerms(L"Ç", L"C", filter, termAtt);
  assertNextTerms(L"È", L"E", filter, termAtt);
  assertNextTerms(L"É", L"E", filter, termAtt);
  assertNextTerms(L"Ê", L"E", filter, termAtt);
  assertNextTerms(L"Ë", L"E", filter, termAtt);
  assertNextTerms(L"Ì", L"I", filter, termAtt);
  assertNextTerms(L"Í", L"I", filter, termAtt);
  assertNextTerms(L"Î", L"I", filter, termAtt);
  assertNextTerms(L"Ï", L"I", filter, termAtt);
  assertNextTerms(L"Ĳ", L"IJ", filter, termAtt);
  assertNextTerms(L"Ð", L"D", filter, termAtt);
  assertNextTerms(L"Ñ", L"N", filter, termAtt);
  assertNextTerms(L"Ò", L"O", filter, termAtt);
  assertNextTerms(L"Ó", L"O", filter, termAtt);
  assertNextTerms(L"Ô", L"O", filter, termAtt);
  assertNextTerms(L"Õ", L"O", filter, termAtt);
  assertNextTerms(L"Ö", L"O", filter, termAtt);
  assertNextTerms(L"Ø", L"O", filter, termAtt);
  assertNextTerms(L"Œ", L"OE", filter, termAtt);
  assertNextTerms(L"Þ", L"TH", filter, termAtt);
  assertNextTerms(L"Ù", L"U", filter, termAtt);
  assertNextTerms(L"Ú", L"U", filter, termAtt);
  assertNextTerms(L"Û", L"U", filter, termAtt);
  assertNextTerms(L"Ü", L"U", filter, termAtt);
  assertNextTerms(L"Ý", L"Y", filter, termAtt);
  assertNextTerms(L"Ÿ", L"Y", filter, termAtt);
  assertNextTerms(L"à", L"a", filter, termAtt);
  assertNextTerms(L"á", L"a", filter, termAtt);
  assertNextTerms(L"â", L"a", filter, termAtt);
  assertNextTerms(L"ã", L"a", filter, termAtt);
  assertNextTerms(L"ä", L"a", filter, termAtt);
  assertNextTerms(L"å", L"a", filter, termAtt);
  assertNextTerms(L"æ", L"ae", filter, termAtt);
  assertNextTerms(L"ç", L"c", filter, termAtt);
  assertNextTerms(L"è", L"e", filter, termAtt);
  assertNextTerms(L"é", L"e", filter, termAtt);
  assertNextTerms(L"ê", L"e", filter, termAtt);
  assertNextTerms(L"ë", L"e", filter, termAtt);
  assertNextTerms(L"ì", L"i", filter, termAtt);
  assertNextTerms(L"í", L"i", filter, termAtt);
  assertNextTerms(L"î", L"i", filter, termAtt);
  assertNextTerms(L"ï", L"i", filter, termAtt);
  assertNextTerms(L"ĳ", L"ij", filter, termAtt);
  assertNextTerms(L"ð", L"d", filter, termAtt);
  assertNextTerms(L"ñ", L"n", filter, termAtt);
  assertNextTerms(L"ò", L"o", filter, termAtt);
  assertNextTerms(L"ó", L"o", filter, termAtt);
  assertNextTerms(L"ô", L"o", filter, termAtt);
  assertNextTerms(L"õ", L"o", filter, termAtt);
  assertNextTerms(L"ö", L"o", filter, termAtt);
  assertNextTerms(L"ø", L"o", filter, termAtt);
  assertNextTerms(L"œ", L"oe", filter, termAtt);
  assertNextTerms(L"ß", L"ss", filter, termAtt);
  assertNextTerms(L"þ", L"th", filter, termAtt);
  assertNextTerms(L"ù", L"u", filter, termAtt);
  assertNextTerms(L"ú", L"u", filter, termAtt);
  assertNextTerms(L"û", L"u", filter, termAtt);
  assertNextTerms(L"ü", L"u", filter, termAtt);
  assertNextTerms(L"ý", L"y", filter, termAtt);
  assertNextTerms(L"ÿ", L"y", filter, termAtt);
  assertNextTerms(L"ﬁ", L"fi", filter, termAtt);
  assertNextTerms(L"ﬂ", L"fl", filter, termAtt);
  assertFalse(filter->incrementToken());
}

void TestASCIIFoldingFilter::testUnmodifiedLetters() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"§ ¦ ¤ END");
  shared_ptr<ASCIIFoldingFilter> filter =
      make_shared<ASCIIFoldingFilter>(stream, true);

  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  filter->reset();
  assertNextTerms(L"§", L"§", filter, termAtt);
  assertNextTerms(L"¦", L"¦", filter, termAtt);
  assertNextTerms(L"¤", L"¤", filter, termAtt);
  assertNextTerms(L"END", L"END", filter, termAtt);
  assertFalse(filter->incrementToken());
}

void TestASCIIFoldingFilter::testAllFoldings() 
{
  // Alternating strings of:
  //   1. All non-ASCII characters to be folded, concatenated together as a
  //      single string.
  //   2. The string of ASCII characters to which each of the above
  //      characters should be folded.
  std::deque<wstring> foldings = {
      wstring(L"À") + L"Á" + L"Â" + L"Ã" + L"Ä" + L"Å" + L"Ā" + L"Ă" + L"Ą" +
          L"Ə" + L"Ǎ" + L"Ǟ" + L"Ǡ" + L"Ǻ" + L"Ȁ" + L"Ȃ" + L"Ȧ" + L"Ⱥ" + L"ᴀ" +
          L"Ḁ" + L"Ạ" + L"Ả" + L"Ấ" + L"Ầ" + L"Ẩ" + L"Ẫ" + L"Ậ" + L"Ắ" + L"Ằ" +
          L"Ẳ" + L"Ẵ" + L"Ặ" + L"Ⓐ" + L"Ａ",
      L"A",
      wstring(L"à") + L"á" + L"â" + L"ã" + L"ä" + L"å" + L"ā" + L"ă" + L"ą" +
          L"ǎ" + L"ǟ" + L"ǡ" + L"ǻ" + L"ȁ" + L"ȃ" + L"ȧ" + L"ɐ" + L"ə" + L"ɚ" +
          L"ᶏ" + L"ḁ" + L"ᶕ" + L"ẚ" + L"ạ" + L"ả" + L"ấ" + L"ầ" + L"ẩ" + L"ẫ" +
          L"ậ" + L"ắ" + L"ằ" + L"ẳ" + L"ẵ" + L"ặ" + L"ₐ" + L"ₔ" + L"ⓐ" + L"ⱥ" +
          L"Ɐ" + L"ａ",
      L"a",
      L"Ꜳ",
      L"AA",
      wstring(L"Æ") + L"Ǣ" + L"Ǽ" + L"ᴁ",
      L"AE",
      L"Ꜵ",
      L"AO",
      L"Ꜷ",
      L"AU",
      wstring(L"Ꜹ") + L"Ꜻ",
      L"AV",
      L"Ꜽ",
      L"AY",
      L"⒜",
      L"(a)",
      L"ꜳ",
      L"aa",
      wstring(L"æ") + L"ǣ" + L"ǽ" + L"ᴂ",
      L"ae",
      L"ꜵ",
      L"ao",
      L"ꜷ",
      L"au",
      wstring(L"ꜹ") + L"ꜻ",
      L"av",
      L"ꜽ",
      L"ay",
      wstring(L"Ɓ") + L"Ƃ" + L"Ƀ" + L"ʙ" + L"ᴃ" + L"Ḃ" + L"Ḅ" + L"Ḇ" + L"Ⓑ" +
          L"Ｂ",
      L"B",
      wstring(L"ƀ") + L"ƃ" + L"ɓ" + L"ᵬ" + L"ᶀ" + L"ḃ" + L"ḅ" + L"ḇ" + L"ⓑ" +
          L"ｂ",
      L"b",
      L"⒝",
      L"(b)",
      wstring(L"Ç") + L"Ć" + L"Ĉ" + L"Ċ" + L"Č" + L"Ƈ" + L"Ȼ" + L"ʗ" + L"ᴄ" +
          L"Ḉ" + L"Ⓒ" + L"Ｃ",
      L"C",
      wstring(L"ç") + L"ć" + L"ĉ" + L"ċ" + L"č" + L"ƈ" + L"ȼ" + L"ɕ" + L"ḉ" +
          L"ↄ" + L"ⓒ" + L"Ꜿ" + L"ꜿ" + L"ｃ",
      L"c",
      L"⒞",
      L"(c)",
      wstring(L"Ð") + L"Ď" + L"Đ" + L"Ɖ" + L"Ɗ" + L"Ƌ" + L"ᴅ" + L"ᴆ" + L"Ḋ" +
          L"Ḍ" + L"Ḏ" + L"Ḑ" + L"Ḓ" + L"Ⓓ" + L"Ꝺ" + L"Ｄ",
      L"D",
      wstring(L"ð") + L"ď" + L"đ" + L"ƌ" + L"ȡ" + L"ɖ" + L"ɗ" + L"ᵭ" + L"ᶁ" +
          L"ᶑ" + L"ḋ" + L"ḍ" + L"ḏ" + L"ḑ" + L"ḓ" + L"ⓓ" + L"ꝺ" + L"ｄ",
      L"d",
      wstring(L"Ǆ") + L"Ǳ",
      L"DZ",
      wstring(L"ǅ") + L"ǲ",
      L"Dz",
      L"⒟",
      L"(d)",
      L"ȸ",
      L"db",
      wstring(L"ǆ") + L"ǳ" + L"ʣ" + L"ʥ",
      L"dz",
      wstring(L"È") + L"É" + L"Ê" + L"Ë" + L"Ē" + L"Ĕ" + L"Ė" + L"Ę" + L"Ě" +
          L"Ǝ" + L"Ɛ" + L"Ȅ" + L"Ȇ" + L"Ȩ" + L"Ɇ" + L"ᴇ" + L"Ḕ" + L"Ḗ" + L"Ḙ" +
          L"Ḛ" + L"Ḝ" + L"Ẹ" + L"Ẻ" + L"Ẽ" + L"Ế" + L"Ề" + L"Ể" + L"Ễ" + L"Ệ" +
          L"Ⓔ" + L"ⱻ" + L"Ｅ",
      L"E",
      wstring(L"è") + L"é" + L"ê" + L"ë" + L"ē" + L"ĕ" + L"ė" + L"ę" + L"ě" +
          L"ǝ" + L"ȅ" + L"ȇ" + L"ȩ" + L"ɇ" + L"ɘ" + L"ɛ" + L"ɜ" + L"ɝ" + L"ɞ" +
          L"ʚ" + L"ᴈ" + L"ᶒ" + L"ᶓ" + L"ᶔ" + L"ḕ" + L"ḗ" + L"ḙ" + L"ḛ" + L"ḝ" +
          L"ẹ" + L"ẻ" + L"ẽ" + L"ế" + L"ề" + L"ể" + L"ễ" + L"ệ" + L"ₑ" + L"ⓔ" +
          L"ⱸ" + L"ｅ",
      L"e",
      L"⒠",
      L"(e)",
      wstring(L"Ƒ") + L"Ḟ" + L"Ⓕ" + L"ꜰ" + L"Ꝼ" + L"ꟻ" + L"Ｆ",
      L"F",
      wstring(L"ƒ") + L"ᵮ" + L"ᶂ" + L"ḟ" + L"ẛ" + L"ⓕ" + L"ꝼ" + L"ｆ",
      L"f",
      L"⒡",
      L"(f)",
      L"ﬀ",
      L"ff",
      L"ﬃ",
      L"ffi",
      L"ﬄ",
      L"ffl",
      L"ﬁ",
      L"fi",
      L"ﬂ",
      L"fl",
      wstring(L"Ĝ") + L"Ğ" + L"Ġ" + L"Ģ" + L"Ɠ" + L"Ǥ" + L"ǥ" + L"Ǧ" + L"ǧ" +
          L"Ǵ" + L"ɢ" + L"ʛ" + L"Ḡ" + L"Ⓖ" + L"Ᵹ" + L"Ꝿ" + L"Ｇ",
      L"G",
      wstring(L"ĝ") + L"ğ" + L"ġ" + L"ģ" + L"ǵ" + L"ɠ" + L"ɡ" + L"ᵷ" + L"ᵹ" +
          L"ᶃ" + L"ḡ" + L"ⓖ" + L"ꝿ" + L"ｇ",
      L"g",
      L"⒢",
      L"(g)",
      wstring(L"Ĥ") + L"Ħ" + L"Ȟ" + L"ʜ" + L"Ḣ" + L"Ḥ" + L"Ḧ" + L"Ḩ" + L"Ḫ" +
          L"Ⓗ" + L"Ⱨ" + L"Ⱶ" + L"Ｈ",
      L"H",
      wstring(L"ĥ") + L"ħ" + L"ȟ" + L"ɥ" + L"ɦ" + L"ʮ" + L"ʯ" + L"ḣ" + L"ḥ" +
          L"ḧ" + L"ḩ" + L"ḫ" + L"ẖ" + L"ⓗ" + L"ⱨ" + L"ⱶ" + L"ｈ",
      L"h",
      L"Ƕ",
      L"HV",
      L"⒣",
      L"(h)",
      L"ƕ",
      L"hv",
      wstring(L"Ì") + L"Í" + L"Î" + L"Ï" + L"Ĩ" + L"Ī" + L"Ĭ" + L"Į" + L"İ" +
          L"Ɩ" + L"Ɨ" + L"Ǐ" + L"Ȉ" + L"Ȋ" + L"ɪ" + L"ᵻ" + L"Ḭ" + L"Ḯ" + L"Ỉ" +
          L"Ị" + L"Ⓘ" + L"ꟾ" + L"Ｉ",
      L"I",
      wstring(L"ì") + L"í" + L"î" + L"ï" + L"ĩ" + L"ī" + L"ĭ" + L"į" + L"ı" +
          L"ǐ" + L"ȉ" + L"ȋ" + L"ɨ" + L"ᴉ" + L"ᵢ" + L"ᵼ" + L"ᶖ" + L"ḭ" + L"ḯ" +
          L"ỉ" + L"ị" + L"ⁱ" + L"ⓘ" + L"ｉ",
      L"i",
      L"Ĳ",
      L"IJ",
      L"⒤",
      L"(i)",
      L"ĳ",
      L"ij",
      wstring(L"Ĵ") + L"Ɉ" + L"ᴊ" + L"Ⓙ" + L"Ｊ",
      L"J",
      wstring(L"ĵ") + L"ǰ" + L"ȷ" + L"ɉ" + L"ɟ" + L"ʄ" + L"ʝ" + L"ⓙ" + L"ⱼ" +
          L"ｊ",
      L"j",
      L"⒥",
      L"(j)",
      wstring(L"Ķ") + L"Ƙ" + L"Ǩ" + L"ᴋ" + L"Ḱ" + L"Ḳ" + L"Ḵ" + L"Ⓚ" + L"Ⱪ" +
          L"Ꝁ" + L"Ꝃ" + L"Ꝅ" + L"Ｋ",
      L"K",
      wstring(L"ķ") + L"ƙ" + L"ǩ" + L"ʞ" + L"ᶄ" + L"ḱ" + L"ḳ" + L"ḵ" + L"ⓚ" +
          L"ⱪ" + L"ꝁ" + L"ꝃ" + L"ꝅ" + L"ｋ",
      L"k",
      L"⒦",
      L"(k)",
      wstring(L"Ĺ") + L"Ļ" + L"Ľ" + L"Ŀ" + L"Ł" + L"Ƚ" + L"ʟ" + L"ᴌ" + L"Ḷ" +
          L"Ḹ" + L"Ḻ" + L"Ḽ" + L"Ⓛ" + L"Ⱡ" + L"Ɫ" + L"Ꝇ" + L"Ꝉ" + L"Ꞁ" + L"Ｌ",
      L"L",
      wstring(L"ĺ") + L"ļ" + L"ľ" + L"ŀ" + L"ł" + L"ƚ" + L"ȴ" + L"ɫ" + L"ɬ" +
          L"ɭ" + L"ᶅ" + L"ḷ" + L"ḹ" + L"ḻ" + L"ḽ" + L"ⓛ" + L"ⱡ" + L"ꝇ" + L"ꝉ" +
          L"ꞁ" + L"ｌ",
      L"l",
      L"Ǉ",
      L"LJ",
      L"Ỻ",
      L"LL",
      L"ǈ",
      L"Lj",
      L"⒧",
      L"(l)",
      L"ǉ",
      L"lj",
      L"ỻ",
      L"ll",
      L"ʪ",
      L"ls",
      L"ʫ",
      L"lz",
      wstring(L"Ɯ") + L"ᴍ" + L"Ḿ" + L"Ṁ" + L"Ṃ" + L"Ⓜ" + L"Ɱ" + L"ꟽ" + L"ꟿ" +
          L"Ｍ",
      L"M",
      wstring(L"ɯ") + L"ɰ" + L"ɱ" + L"ᵯ" + L"ᶆ" + L"ḿ" + L"ṁ" + L"ṃ" + L"ⓜ" +
          L"ｍ",
      L"m",
      L"⒨",
      L"(m)",
      wstring(L"Ñ") + L"Ń" + L"Ņ" + L"Ň" + L"Ŋ" + L"Ɲ" + L"Ǹ" + L"Ƞ" + L"ɴ" +
          L"ᴎ" + L"Ṅ" + L"Ṇ" + L"Ṉ" + L"Ṋ" + L"Ⓝ" + L"Ｎ",
      L"N",
      wstring(L"ñ") + L"ń" + L"ņ" + L"ň" + L"ŉ" + L"ŋ" + L"ƞ" + L"ǹ" + L"ȵ" +
          L"ɲ" + L"ɳ" + L"ᵰ" + L"ᶇ" + L"ṅ" + L"ṇ" + L"ṉ" + L"ṋ" + L"ⁿ" + L"ⓝ" +
          L"ｎ",
      L"n",
      L"Ǌ",
      L"NJ",
      L"ǋ",
      L"Nj",
      L"⒩",
      L"(n)",
      L"ǌ",
      L"nj",
      wstring(L"Ò") + L"Ó" + L"Ô" + L"Õ" + L"Ö" + L"Ø" + L"Ō" + L"Ŏ" + L"Ő" +
          L"Ɔ" + L"Ɵ" + L"Ơ" + L"Ǒ" + L"Ǫ" + L"Ǭ" + L"Ǿ" + L"Ȍ" + L"Ȏ" + L"Ȫ" +
          L"Ȭ" + L"Ȯ" + L"Ȱ" + L"ᴏ" + L"ᴐ" + L"Ṍ" + L"Ṏ" + L"Ṑ" + L"Ṓ" + L"Ọ" +
          L"Ỏ" + L"Ố" + L"Ồ" + L"Ổ" + L"Ỗ" + L"Ộ" + L"Ớ" + L"Ờ" + L"Ở" + L"Ỡ" +
          L"Ợ" + L"Ⓞ" + L"Ꝋ" + L"Ꝍ" + L"Ｏ",
      L"O",
      wstring(L"ò") + L"ó" + L"ô" + L"õ" + L"ö" + L"ø" + L"ō" + L"ŏ" + L"ő" +
          L"ơ" + L"ǒ" + L"ǫ" + L"ǭ" + L"ǿ" + L"ȍ" + L"ȏ" + L"ȫ" + L"ȭ" + L"ȯ" +
          L"ȱ" + L"ɔ" + L"ɵ" + L"ᴖ" + L"ᴗ" + L"ᶗ" + L"ṍ" + L"ṏ" + L"ṑ" + L"ṓ" +
          L"ọ" + L"ỏ" + L"ố" + L"ồ" + L"ổ" + L"ỗ" + L"ộ" + L"ớ" + L"ờ" + L"ở" +
          L"ỡ" + L"ợ" + L"ₒ" + L"ⓞ" + L"ⱺ" + L"ꝋ" + L"ꝍ" + L"ｏ",
      L"o",
      wstring(L"Œ") + L"ɶ",
      L"OE",
      L"Ꝏ",
      L"OO",
      wstring(L"Ȣ") + L"ᴕ",
      L"OU",
      L"⒪",
      L"(o)",
      wstring(L"œ") + L"ᴔ",
      L"oe",
      L"ꝏ",
      L"oo",
      L"ȣ",
      L"ou",
      wstring(L"Ƥ") + L"ᴘ" + L"Ṕ" + L"Ṗ" + L"Ⓟ" + L"Ᵽ" + L"Ꝑ" + L"Ꝓ" + L"Ꝕ" +
          L"Ｐ",
      L"P",
      wstring(L"ƥ") + L"ᵱ" + L"ᵽ" + L"ᶈ" + L"ṕ" + L"ṗ" + L"ⓟ" + L"ꝑ" + L"ꝓ" +
          L"ꝕ" + L"ꟼ" + L"ｐ",
      L"p",
      L"⒫",
      L"(p)",
      wstring(L"Ɋ") + L"Ⓠ" + L"Ꝗ" + L"Ꝙ" + L"Ｑ",
      L"Q",
      wstring(L"ĸ") + L"ɋ" + L"ʠ" + L"ⓠ" + L"ꝗ" + L"ꝙ" + L"ｑ",
      L"q",
      L"⒬",
      L"(q)",
      L"ȹ",
      L"qp",
      wstring(L"Ŕ") + L"Ŗ" + L"Ř" + L"Ȑ" + L"Ȓ" + L"Ɍ" + L"ʀ" + L"ʁ" + L"ᴙ" +
          L"ᴚ" + L"Ṙ" + L"Ṛ" + L"Ṝ" + L"Ṟ" + L"Ⓡ" + L"Ɽ" + L"Ꝛ" + L"Ꞃ" + L"Ｒ",
      L"R",
      wstring(L"ŕ") + L"ŗ" + L"ř" + L"ȑ" + L"ȓ" + L"ɍ" + L"ɼ" + L"ɽ" + L"ɾ" +
          L"ɿ" + L"ᵣ" + L"ᵲ" + L"ᵳ" + L"ᶉ" + L"ṙ" + L"ṛ" + L"ṝ" + L"ṟ" + L"ⓡ" +
          L"ꝛ" + L"ꞃ" + L"ｒ",
      L"r",
      L"⒭",
      L"(r)",
      wstring(L"Ś") + L"Ŝ" + L"Ş" + L"Š" + L"Ș" + L"Ṡ" + L"Ṣ" + L"Ṥ" + L"Ṧ" +
          L"Ṩ" + L"Ⓢ" + L"ꜱ" + L"ꞅ" + L"Ｓ",
      L"S",
      wstring(L"ś") + L"ŝ" + L"ş" + L"š" + L"ſ" + L"ș" + L"ȿ" + L"ʂ" + L"ᵴ" +
          L"ᶊ" + L"ṡ" + L"ṣ" + L"ṥ" + L"ṧ" + L"ṩ" + L"ẜ" + L"ẝ" + L"ⓢ" + L"Ꞅ" +
          L"ｓ",
      L"s",
      L"ẞ",
      L"SS",
      L"⒮",
      L"(s)",
      L"ß",
      L"ss",
      L"ﬆ",
      L"st",
      wstring(L"Ţ") + L"Ť" + L"Ŧ" + L"Ƭ" + L"Ʈ" + L"Ț" + L"Ⱦ" + L"ᴛ" + L"Ṫ" +
          L"Ṭ" + L"Ṯ" + L"Ṱ" + L"Ⓣ" + L"Ꞇ" + L"Ｔ",
      L"T",
      wstring(L"ţ") + L"ť" + L"ŧ" + L"ƫ" + L"ƭ" + L"ț" + L"ȶ" + L"ʇ" + L"ʈ" +
          L"ᵵ" + L"ṫ" + L"ṭ" + L"ṯ" + L"ṱ" + L"ẗ" + L"ⓣ" + L"ⱦ" + L"ｔ",
      L"t",
      wstring(L"Þ") + L"Ꝧ",
      L"TH",
      L"Ꜩ",
      L"TZ",
      L"⒯",
      L"(t)",
      L"ʨ",
      L"tc",
      wstring(L"þ") + L"ᵺ" + L"ꝧ",
      L"th",
      L"ʦ",
      L"ts",
      L"ꜩ",
      L"tz",
      wstring(L"Ù") + L"Ú" + L"Û" + L"Ü" + L"Ũ" + L"Ū" + L"Ŭ" + L"Ů" + L"Ű" +
          L"Ų" + L"Ư" + L"Ǔ" + L"Ǖ" + L"Ǘ" + L"Ǚ" + L"Ǜ" + L"Ȕ" + L"Ȗ" + L"Ʉ" +
          L"ᴜ" + L"ᵾ" + L"Ṳ" + L"Ṵ" + L"Ṷ" + L"Ṹ" + L"Ṻ" + L"Ụ" + L"Ủ" + L"Ứ" +
          L"Ừ" + L"Ử" + L"Ữ" + L"Ự" + L"Ⓤ" + L"Ｕ",
      L"U",
      wstring(L"ù") + L"ú" + L"û" + L"ü" + L"ũ" + L"ū" + L"ŭ" + L"ů" + L"ű" +
          L"ų" + L"ư" + L"ǔ" + L"ǖ" + L"ǘ" + L"ǚ" + L"ǜ" + L"ȕ" + L"ȗ" + L"ʉ" +
          L"ᵤ" + L"ᶙ" + L"ṳ" + L"ṵ" + L"ṷ" + L"ṹ" + L"ṻ" + L"ụ" + L"ủ" + L"ứ" +
          L"ừ" + L"ử" + L"ữ" + L"ự" + L"ⓤ" + L"ｕ",
      L"u",
      L"⒰",
      L"(u)",
      L"ᵫ",
      L"ue",
      wstring(L"Ʋ") + L"Ʌ" + L"ᴠ" + L"Ṽ" + L"Ṿ" + L"Ỽ" + L"Ⓥ" + L"Ꝟ" + L"Ꝩ" +
          L"Ｖ",
      L"V",
      wstring(L"ʋ") + L"ʌ" + L"ᵥ" + L"ᶌ" + L"ṽ" + L"ṿ" + L"ⓥ" + L"ⱱ" + L"ⱴ" +
          L"ꝟ" + L"ｖ",
      L"v",
      L"Ꝡ",
      L"VY",
      L"⒱",
      L"(v)",
      L"ꝡ",
      L"vy",
      wstring(L"Ŵ") + L"Ƿ" + L"ᴡ" + L"Ẁ" + L"Ẃ" + L"Ẅ" + L"Ẇ" + L"Ẉ" + L"Ⓦ" +
          L"Ⱳ" + L"Ｗ",
      L"W",
      wstring(L"ŵ") + L"ƿ" + L"ʍ" + L"ẁ" + L"ẃ" + L"ẅ" + L"ẇ" + L"ẉ" + L"ẘ" +
          L"ⓦ" + L"ⱳ" + L"ｗ",
      L"w",
      L"⒲",
      L"(w)",
      wstring(L"Ẋ") + L"Ẍ" + L"Ⓧ" + L"Ｘ",
      L"X",
      wstring(L"ᶍ") + L"ẋ" + L"ẍ" + L"ₓ" + L"ⓧ" + L"ｘ",
      L"x",
      L"⒳",
      L"(x)",
      wstring(L"Ý") + L"Ŷ" + L"Ÿ" + L"Ƴ" + L"Ȳ" + L"Ɏ" + L"ʏ" + L"Ẏ" + L"Ỳ" +
          L"Ỵ" + L"Ỷ" + L"Ỹ" + L"Ỿ" + L"Ⓨ" + L"Ｙ",
      L"Y",
      wstring(L"ý") + L"ÿ" + L"ŷ" + L"ƴ" + L"ȳ" + L"ɏ" + L"ʎ" + L"ẏ" + L"ẙ" +
          L"ỳ" + L"ỵ" + L"ỷ" + L"ỹ" + L"ỿ" + L"ⓨ" + L"ｙ",
      L"y",
      L"⒴",
      L"(y)",
      wstring(L"Ź") + L"Ż" + L"Ž" + L"Ƶ" + L"Ȝ" + L"Ȥ" + L"ᴢ" + L"Ẑ" + L"Ẓ" +
          L"Ẕ" + L"Ⓩ" + L"Ⱬ" + L"Ꝣ" + L"Ｚ",
      L"Z",
      wstring(L"ź") + L"ż" + L"ž" + L"ƶ" + L"ȝ" + L"ȥ" + L"ɀ" + L"ʐ" + L"ʑ" +
          L"ᵶ" + L"ᶎ" + L"ẑ" + L"ẓ" + L"ẕ" + L"ⓩ" + L"ⱬ" + L"ꝣ" + L"ｚ",
      L"z",
      L"⒵",
      L"(z)",
      wstring(L"⁰") + L"₀" + L"⓪" + L"⓿" + L"０",
      L"0",
      wstring(L"¹") + L"₁" + L"①" + L"⓵" + L"❶" + L"➀" + L"➊" + L"１",
      L"1",
      L"⒈",
      L"1.",
      L"⑴",
      L"(1)",
      wstring(L"²") + L"₂" + L"②" + L"⓶" + L"❷" + L"➁" + L"➋" + L"２",
      L"2",
      L"⒉",
      L"2.",
      L"⑵",
      L"(2)",
      wstring(L"³") + L"₃" + L"③" + L"⓷" + L"❸" + L"➂" + L"➌" + L"３",
      L"3",
      L"⒊",
      L"3.",
      L"⑶",
      L"(3)",
      wstring(L"⁴") + L"₄" + L"④" + L"⓸" + L"❹" + L"➃" + L"➍" + L"４",
      L"4",
      L"⒋",
      L"4.",
      L"⑷",
      L"(4)",
      wstring(L"⁵") + L"₅" + L"⑤" + L"⓹" + L"❺" + L"➄" + L"➎" + L"５",
      L"5",
      L"⒌",
      L"5.",
      L"⑸",
      L"(5)",
      wstring(L"⁶") + L"₆" + L"⑥" + L"⓺" + L"❻" + L"➅" + L"➏" + L"６",
      L"6",
      L"⒍",
      L"6.",
      L"⑹",
      L"(6)",
      wstring(L"⁷") + L"₇" + L"⑦" + L"⓻" + L"❼" + L"➆" + L"➐" + L"７",
      L"7",
      L"⒎",
      L"7.",
      L"⑺",
      L"(7)",
      wstring(L"⁸") + L"₈" + L"⑧" + L"⓼" + L"❽" + L"➇" + L"➑" + L"８",
      L"8",
      L"⒏",
      L"8.",
      L"⑻",
      L"(8)",
      wstring(L"⁹") + L"₉" + L"⑨" + L"⓽" + L"❾" + L"➈" + L"➒" + L"９",
      L"9",
      L"⒐",
      L"9.",
      L"⑼",
      L"(9)",
      wstring(L"⑩") + L"⓾" + L"❿" + L"➉" + L"➓",
      L"10",
      L"⒑",
      L"10.",
      L"⑽",
      L"(10)",
      wstring(L"⑪") + L"⓫",
      L"11",
      L"⒒",
      L"11.",
      L"⑾",
      L"(11)",
      wstring(L"⑫") + L"⓬",
      L"12",
      L"⒓",
      L"12.",
      L"⑿",
      L"(12)",
      wstring(L"⑬") + L"⓭",
      L"13",
      L"⒔",
      L"13.",
      L"⒀",
      L"(13)",
      wstring(L"⑭") + L"⓮",
      L"14",
      L"⒕",
      L"14.",
      L"⒁",
      L"(14)",
      wstring(L"⑮") + L"⓯",
      L"15",
      L"⒖",
      L"15.",
      L"⒂",
      L"(15)",
      wstring(L"⑯") + L"⓰",
      L"16",
      L"⒗",
      L"16.",
      L"⒃",
      L"(16)",
      wstring(L"⑰") + L"⓱",
      L"17",
      L"⒘",
      L"17.",
      L"⒄",
      L"(17)",
      wstring(L"⑱") + L"⓲",
      L"18",
      L"⒙",
      L"18.",
      L"⒅",
      L"(18)",
      wstring(L"⑲") + L"⓳",
      L"19",
      L"⒚",
      L"19.",
      L"⒆",
      L"(19)",
      wstring(L"⑳") + L"⓴",
      L"20",
      L"⒛",
      L"20.",
      L"⒇",
      L"(20)",
      wstring(L"«") + L"»" + L"“" + L"”" + L"„" + L"″" + L"‶" + L"❝" + L"❞" +
          L"❮" + L"❯" + L"＂",
      L"\"",
      wstring(L"‘") + L"’" + L"‚" + L"‛" + L"′" + L"‵" + L"‹" + L"›" + L"❛" +
          L"❜" + L"＇",
      L"'",
      wstring(L"‐") + L"‑" + L"‒" + L"–" + L"—" + L"⁻" + L"₋" + L"－",
      L"-",
      wstring(L"⁅") + L"❲" + L"［",
      L"[",
      wstring(L"⁆") + L"❳" + L"］",
      L"]",
      wstring(L"⁽") + L"₍" + L"❨" + L"❪" + L"（",
      L"(",
      L"⸨",
      L"((",
      wstring(L"⁾") + L"₎" + L"❩" + L"❫" + L"）",
      L")",
      L"⸩",
      L"))",
      wstring(L"❬") + L"❰" + L"＜",
      L"<",
      wstring(L"❭") + L"❱" + L"＞",
      L">",
      wstring(L"❴") + L"｛",
      L"{",
      wstring(L"❵") + L"｝",
      L"}",
      wstring(L"⁺") + L"₊" + L"＋",
      L"+",
      wstring(L"⁼") + L"₌" + L"＝",
      L"=",
      L"！",
      L"!",
      L"‼",
      L"!!",
      L"⁉",
      L"!?",
      L"＃",
      L"#",
      L"＄",
      L"$",
      wstring(L"⁒") + L"％",
      L"%",
      L"＆",
      L"&",
      wstring(L"⁎") + L"＊",
      L"*",
      L"，",
      L",",
      L"．",
      L".",
      wstring(L"⁄") + L"／",
      L"/",
      L"：",
      L":",
      wstring(L"⁏") + L"；",
      L";",
      L"？",
      L"?",
      L"⁇",
      L"??",
      L"⁈",
      L"?!",
      L"＠",
      L"@",
      L"＼",
      L"\\",
      wstring(L"‸") + L"＾",
      L"^",
      L"＿",
      L"_",
      wstring(L"⁓") + L"～",
      L"~"};

  // Construct input text and expected output tokens
  deque<wstring> expectedUnfoldedTokens = deque<wstring>();
  deque<wstring> expectedFoldedTokens = deque<wstring>();
  shared_ptr<StringBuilder> inputText = make_shared<StringBuilder>();
  for (int n = 0; n < foldings.size(); n += 2) {
    if (n > 0) {
      inputText->append(L' '); // Space between tokens
    }
    inputText->append(foldings[n]);

    // Construct the expected output tokens: both the unfolded and folded
    // string, with the folded duplicated as many times as the number of
    // characters in the input text.
    shared_ptr<StringBuilder> expected = make_shared<StringBuilder>();
    int numChars = foldings[n].length();
    for (int m = 0; m < numChars; ++m) {
      expected->append(foldings[n + 1]);
    }
    expectedUnfoldedTokens.push_back(foldings[n]);
    expectedFoldedTokens.push_back(expected->toString());
  }

  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(inputText->toString());
  shared_ptr<ASCIIFoldingFilter> filter =
      make_shared<ASCIIFoldingFilter>(stream, random()->nextBoolean());
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  deque<wstring>::const_iterator unfoldedIter = expectedUnfoldedTokens.begin();
  deque<wstring>::const_iterator foldedIter = expectedFoldedTokens.begin();
  filter->reset();
  while (foldedIter != expectedFoldedTokens.end()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertNextTerms(unfoldedIter.next(), *foldedIter, filter, termAtt);
    foldedIter++;
  }
  assertFalse(filter->incrementToken());
}

void TestASCIIFoldingFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestASCIIFoldingFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestASCIIFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestASCIIFoldingFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<ASCIIFoldingFilter>(tokenizer, random()->nextBoolean()));
}

void TestASCIIFoldingFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestASCIIFoldingFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestASCIIFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestASCIIFoldingFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<ASCIIFoldingFilter>(tokenizer, random()->nextBoolean()));
}
} // namespace org::apache::lucene::analysis::miscellaneous