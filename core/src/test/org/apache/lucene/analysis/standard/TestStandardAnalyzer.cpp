using namespace std;

#include "TestStandardAnalyzer.h"

namespace org::apache::lucene::analysis::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testLargePartiallyMatchingToken() throws
// Exception
void TestStandardAnalyzer::testLargePartiallyMatchingToken() throw(
    runtime_error)
{
  // TODO: get these lists of chars matching a property from ICU4J
  // http://www.unicode.org/Public/6.3.0/ucd/auxiliary/WordBreakProperty.txt
  std::deque<wchar_t> WordBreak_ExtendNumLet_chars =
      (wstring(L"_\u203f\u2040\u2054\ufe33\ufe34\ufe4d\ufe4e\ufe4f\uff3f"))
          .toCharArray();

  // http://www.unicode.org/Public/6.3.0/ucd/auxiliary/WordBreakProperty.txt
  std::deque<int> WordBreak_Format_chars = {
      0xAD,   0x600,  0x61C,  0x6DD,  0x70F,   0x180E,  0x200E,  0x202A,
      0x2060, 0x2066, 0xFEFF, 0xFFF9, 0x110BD, 0x1D173, 0xE0001, 0xE0020};

  // http://www.unicode.org/Public/6.3.0/ucd/auxiliary/WordBreakProperty.txt
  std::deque<int> WordBreak_Extend_chars = {
      0x300,   0x483,   0x591,   0x5bf,   0x5c1,   0x5c4,   0x5c7,   0x610,
      0x64b,   0x670,   0x6d6,   0x6df,   0x6e7,   0x6ea,   0x711,   0x730,
      0x7a6,   0x7eb,   0x816,   0x81b,   0x825,   0x829,   0x859,   0x8e4,
      0x900,   0x93a,   0x93e,   0x951,   0x962,   0x981,   0x9bc,   0x9be,
      0x9c7,   0x9cb,   0x9d7,   0x9e2,   0xa01,   0xa3c,   0xa3e,   0xa47,
      0xa4b,   0xa51,   0xa70,   0xa75,   0xa81,   0xabc,   0xabe,   0xac7,
      0xacb,   0xae2,   0xb01,   0xb3c,   0xb3e,   0xb47,   0xb4b,   0xb56,
      0xb62,   0xb82,   0xbbe,   0xbc6,   0xbca,   0xbd7,   0xc01,   0xc3e,
      0xc46,   0xc4a,   0xc55,   0xc62,   0xc82,   0xcbc,   0xcbe,   0xcc6,
      0xcca,   0xcd5,   0xce2,   0xd02,   0xd3e,   0xd46,   0xd4a,   0xd57,
      0xd62,   0xd82,   0xdca,   0xdcf,   0xdd6,   0xdd8,   0xdf2,   0xe31,
      0xe34,   0xe47,   0xeb1,   0xeb4,   0xebb,   0xec8,   0xf18,   0xf35,
      0xf37,   0xf39,   0xf3e,   0xf71,   0xf86,   0xf8d,   0xf99,   0xfc6,
      0x102b,  0x1056,  0x105e,  0x1062,  0x1067,  0x1071,  0x1082,  0x108f,
      0x109a,  0x135d,  0x1712,  0x1732,  0x1752,  0x1772,  0x17b4,  0x17dd,
      0x180b,  0x18a9,  0x1920,  0x1930,  0x19b0,  0x19c8,  0x1a17,  0x1a55,
      0x1a60,  0x1a7f,  0x1b00,  0x1b34,  0x1b6b,  0x1b80,  0x1ba1,  0x1be6,
      0x1c24,  0x1cd0,  0x1cd4,  0x1ced,  0x1cf2,  0x1dc0,  0x1dfc,  0x200c,
      0x20d0,  0x2cef,  0x2d7f,  0x2de0,  0x302a,  0x3099,  0xa66f,  0xa674,
      0xa69f,  0xa6f0,  0xa802,  0xa806,  0xa80b,  0xa823,  0xa880,  0xa8b4,
      0xa8e0,  0xa926,  0xa947,  0xa980,  0xa9b3,  0xaa29,  0xaa43,  0xaa4c,
      0xaa7b,  0xaab0,  0xaab2,  0xaab7,  0xaabe,  0xaac1,  0xaaeb,  0xaaf5,
      0xabe3,  0xabec,  0xfb1e,  0xfe00,  0xfe20,  0xff9e,  0x101fd, 0x10a01,
      0x10a05, 0x10a0C, 0x10a38, 0x10a3F, 0x11000, 0x11001, 0x11038, 0x11080,
      0x11082, 0x110b0, 0x110b3, 0x110b7, 0x110b9, 0x11100, 0x11127, 0x1112c,
      0x11180, 0x11182, 0x111b3, 0x111b6, 0x111bF, 0x116ab, 0x116ac, 0x116b0,
      0x116b6, 0x16f51, 0x16f8f, 0x1d165, 0x1d167, 0x1d16d, 0x1d17b, 0x1d185,
      0x1d1aa, 0x1d242, 0xe0100};

  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int numChars = TestUtil::nextInt(random(), 100 * 1024, 1024 * 1024);
  for (int i = 0; i < numChars;) {
    builder->append(WordBreak_ExtendNumLet_chars[random()->nextInt(
        WordBreak_ExtendNumLet_chars.size())]);
    ++i;
    if (random()->nextBoolean()) {
      int numFormatExtendChars = TestUtil::nextInt(random(), 1, 8);
      for (int j = 0; j < numFormatExtendChars; ++j) {
        int codepoint;
        if (random()->nextBoolean()) {
          codepoint = WordBreak_Format_chars[random()->nextInt(
              WordBreak_Format_chars.size())];
        } else {
          codepoint = WordBreak_Extend_chars[random()->nextInt(
              WordBreak_Extend_chars.size())];
        }
        std::deque<wchar_t> chars = Character::toChars(codepoint);
        builder->append(chars);
        i += chars.size();
      }
    }
  }
  shared_ptr<StandardTokenizer> ts = make_shared<StandardTokenizer>();
  ts->setReader(make_shared<StringReader>(builder->toString()));
  ts->reset();
  while (ts->incrementToken()) {
  }
  ts->end();
  delete ts;

  int newBufferSize = TestUtil::nextInt(random(), 200, 8192);
  ts->setMaxTokenLength(newBufferSize); // try a different buffer size
  ts->setReader(make_shared<StringReader>(builder->toString()));
  ts->reset();
  while (ts->incrementToken()) {
  }
  ts->end();
  delete ts;
}

void TestStandardAnalyzer::testHugeDoc() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wchar_t> whitespace(4094);
  Arrays::fill(whitespace, L' ');
  sb->append(whitespace);
  sb->append(L"testing 1234");
  wstring input = sb->toString();
  shared_ptr<StandardTokenizer> tokenizer = make_shared<StandardTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(input));
  BaseTokenStreamTestCase::assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"testing", L"1234"});
}

void TestStandardAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestStandardAnalyzer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestStandardAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestStandardAnalyzer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<StandardTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestStandardAnalyzer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestStandardAnalyzer::testArmenian() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"Վիքիպեդիայի 13 միլիոն հոդվածները (4,600` հայերեն վիքիպեդիայում) գրվել "
      L"են կամավորների կողմից ու համարյա բոլոր հոդվածները կարող է խմբագրել "
      L"ցանկաց մարդ ով կարող է բացել Վիքիպեդիայի կայքը։",
      std::deque<wstring>{L"Վիքիպեդիայի",   L"13",     L"միլիոն",
                           L"հոդվածները",    L"4,600",  L"հայերեն",
                           L"վիքիպեդիայում", L"գրվել",  L"են",
                           L"կամավորների",   L"կողմից", L"ու",
                           L"համարյա",       L"բոլոր",  L"հոդվածները",
                           L"կարող",         L"է",      L"խմբագրել",
                           L"ցանկաց",        L"մարդ",   L"ով",
                           L"կարող",         L"է",      L"բացել",
                           L"Վիքիպեդիայի",   L"կայքը"});
}

void TestStandardAnalyzer::testAmharic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ዊኪፔድያ የባለ ብዙ ቋንቋ የተሟላ ትክክለኛና ነጻ መዝገበ ዕውቀት (ኢንሳይክሎፒዲያ) ነው። ማንኛውም",
      std::deque<wstring>{L"ዊኪፔድያ", L"የባለ", L"ብዙ", L"ቋንቋ", L"የተሟላ", L"ትክክለኛና",
                           L"ነጻ", L"መዝገበ", L"ዕውቀት", L"ኢንሳይክሎፒዲያ", L"ነው",
                           L"ማንኛውም"});
}

void TestStandardAnalyzer::testArabic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"الفيلم الوثائقي الأول عن ويكيبيديا يسمى \"الحقيقة بالأرقام: قصة "
      L"ويكيبيديا\" (بالإنجليزية: Truth in Numbers: The Wikipedia Story)، سيتم "
      L"إطلاقه في 2008.",
      std::deque<wstring>{
          L"الفيلم",      L"الوثائقي", L"الأول",    L"عن",      L"ويكيبيديا",
          L"يسمى",        L"الحقيقة",  L"بالأرقام", L"قصة",     L"ويكيبيديا",
          L"بالإنجليزية", L"Truth",    L"in",       L"Numbers", L"The",
          L"Wikipedia",   L"Story",    L"سيتم",     L"إطلاقه",  L"في",
          L"2008"});
}

void TestStandardAnalyzer::testAramaic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"ܘܝܩܝܦܕܝܐ (ܐܢܓܠܝܐ: Wikipedia) ܗܘ ܐܝܢܣܩܠܘܦܕܝܐ ܚܐܪܬܐ ܕܐܢܛܪܢܛ ܒܠܫܢ̈ܐ ܣܓܝܐ̈ܐ܂ "
      L"ܫܡܗ ܐܬܐ ܡܢ ܡ̈ܠܬܐ ܕ\"ܘܝܩܝ\" ܘ\"ܐܝܢܣܩܠܘܦܕܝܐ\"܀",
      std::deque<wstring>{L"ܘܝܩܝܦܕܝܐ", L"ܐܢܓܠܝܐ", L"Wikipedia", L"ܗܘ",
                           L"ܐܝܢܣܩܠܘܦܕܝܐ", L"ܚܐܪܬܐ", L"ܕܐܢܛܪܢܛ", L"ܒܠܫܢ̈ܐ",
                           L"ܣܓܝܐ̈ܐ", L"ܫܡܗ", L"ܐܬܐ", L"ܡܢ", L"ܡ̈ܠܬܐ", L"ܕ",
                           L"ܘܝܩܝ", L"ܘ", L"ܐܝܢܣܩܠܘܦܕܝܐ"});
}

void TestStandardAnalyzer::testBengali() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"এই বিশ্বকোষ পরিচালনা করে উইকিমিডিয়া ফাউন্ডেশন (একটি অলাভজনক সংস্থা)। "
      L"উইকিপিডিয়ার শুরু ১৫ জানুয়ারি, ২০০১ সালে। এখন পর্যন্ত ২০০টিরও বেশী ভাষায় "
      L"উইকিপিডিয়া রয়েছে।",
      std::deque<wstring>{
          L"এই",         L"বিশ্বকোষ", L"পরিচালনা", L"করে",   L"উইকিমিডিয়া",
          L"ফাউন্ডেশন",   L"একটি",    L"অলাভজনক",  L"সংস্থা", L"উইকিপিডিয়ার",
          L"শুরু",         L"১৫",      L"জানুয়ারি",  L"২০০১",  L"সালে",
          L"এখন",        L"পর্যন্ত",   L"২০০টিরও",  L"বেশী",  L"ভাষায়",
          L"উইকিপিডিয়া", L"রয়েছে"});
}

void TestStandardAnalyzer::testFarsi() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"ویکی پدیای انگلیسی در تاریخ ۲۵ دی ۱۳۷۹ به صورت مکملی برای دانشنامهٔ "
      L"تخصصی نوپدیا نوشته شد.",
      std::deque<wstring>{L"ویکی", L"پدیای", L"انگلیسی", L"در", L"تاریخ",
                           L"۲۵", L"دی", L"۱۳۷۹", L"به", L"صورت", L"مکملی",
                           L"برای", L"دانشنامهٔ", L"تخصصی", L"نوپدیا", L"نوشته",
                           L"شد"});
}

void TestStandardAnalyzer::testGreek() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"Γράφεται σε συνεργασία από εθελοντές με το λογισμικό wiki, κάτι που "
      L"σημαίνει ότι άρθρα μπορεί να προστεθούν ή να αλλάξουν από τον καθένα.",
      std::deque<wstring>{L"Γράφεται",   L"σε",    L"συνεργασία", L"από",
                           L"εθελοντές",  L"με",    L"το",         L"λογισμικό",
                           L"wiki",       L"κάτι",  L"που",        L"σημαίνει",
                           L"ότι",        L"άρθρα", L"μπορεί",     L"να",
                           L"προστεθούν", L"ή",     L"να",         L"αλλάξουν",
                           L"από",        L"τον",   L"καθένα"});
}

void TestStandardAnalyzer::testThai() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"การที่ได้ต้องแสดงว่างานดี. แล้วเธอจะไปไหน? ๑๒๓๔",
      std::deque<wstring>{L"การที่ได้ต้องแสดงว่างานดี", L"แล้วเธอจะไปไหน", L"๑๒๓๔"});
}

void TestStandardAnalyzer::testLao() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ສາທາລະນະລັດ ປະຊາທິປະໄຕ ປະຊາຊົນລາວ",
      std::deque<wstring>{L"ສາທາລະນະລັດ", L"ປະຊາທິປະໄຕ", L"ປະຊາຊົນລາວ"});
}

void TestStandardAnalyzer::testTibetan() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །",
      std::deque<wstring>{L"སྣོན", L"མཛོད", L"དང", L"ལས",  L"འདིས", L"བོད",  L"ཡིག",
                           L"མི",  L"ཉམས", L"གོང", L"འཕེལ", L"དུ",   L"གཏོང", L"བར",
                           L"ཧ",  L"ཅང",  L"དགེ", L"མཚན", L"མཆིས", L"སོ"});
}

void TestStandardAnalyzer::testChinese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"我是中国人。 １２３４ Ｔｅｓｔｓ ",
      std::deque<wstring>{L"我", L"是", L"中", L"国", L"人", L"１２３４",
                           L"Ｔｅｓｔｓ"});
}

void TestStandardAnalyzer::testEmpty() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L".", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L" ", std::deque<wstring>());
}

void TestStandardAnalyzer::testLUCENE1545() 
{
  /*
   * Standard analyzer does not correctly tokenize combining character U+0364
   * COMBINING LATIN SMALL LETTRE E. The word "moͤchte" is incorrectly tokenized
   * into "mo" "chte", the combining character is lost. Expected result is only
   * on token "moͤchte".
   */
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"moͤchte",
                                            std::deque<wstring>{L"moͤchte"});
}

void TestStandardAnalyzer::testAlphanumericSA() 
{
  // alphanumeric tokens
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"B2B",
                                            std::deque<wstring>{L"B2B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"2B",
                                            std::deque<wstring>{L"2B"});
}

void TestStandardAnalyzer::testDelimitersSA() 
{
  // other delimiters: "-", "/", ","
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"some-dashed-phrase",
      std::deque<wstring>{L"some", L"dashed", L"phrase"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"dogs,chase,cats", std::deque<wstring>{L"dogs", L"chase", L"cats"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"ac/dc",
                                            std::deque<wstring>{L"ac", L"dc"});
}

void TestStandardAnalyzer::testApostrophesSA() 
{
  // internal apostrophes: O'Reilly, you're, O'Reilly's
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"O'Reilly",
                                            std::deque<wstring>{L"O'Reilly"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"you're",
                                            std::deque<wstring>{L"you're"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"she's",
                                            std::deque<wstring>{L"she's"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"Jim's",
                                            std::deque<wstring>{L"Jim's"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"don't",
                                            std::deque<wstring>{L"don't"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"O'Reilly's", std::deque<wstring>{L"O'Reilly's"});
}

void TestStandardAnalyzer::testNumericSA() 
{
  // floating point, serial, model numbers, ip addresses, etc.
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"21.35",
                                            std::deque<wstring>{L"21.35"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"R2D2 C3PO", std::deque<wstring>{L"R2D2", L"C3PO"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"216.239.63.104", std::deque<wstring>{L"216.239.63.104"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"216.239.63.104", std::deque<wstring>{L"216.239.63.104"});
}

void TestStandardAnalyzer::testTextWithNumbersSA() 
{
  // numbers
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"});
}

void TestStandardAnalyzer::testVariousTextSA() 
{
  // various
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"C embedded developers wanted",
      std::deque<wstring>{L"C", L"embedded", L"developers", L"wanted"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"foo bar FOO BAR",
      std::deque<wstring>{L"foo", L"bar", L"FOO", L"BAR"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"foo      bar .  FOO <> BAR",
      std::deque<wstring>{L"foo", L"bar", L"FOO", L"BAR"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"\"QUOTED\" word", std::deque<wstring>{L"QUOTED", L"word"});
}

void TestStandardAnalyzer::testKoreanSA() 
{
  // Korean words
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"안녕하세요 한글입니다",
      std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestStandardAnalyzer::testOffsets() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
      std::deque<int>{0, 6, 10, 15}, std::deque<int>{5, 9, 14, 20});
}

void TestStandardAnalyzer::testTypes() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<NUM>",
                           L"<ALPHANUM>"});
}

void TestStandardAnalyzer::testUnicodeWordBreaks() 
{
  shared_ptr<WordBreakTestUnicode_6_3_0> wordBreakTest =
      make_shared<WordBreakTestUnicode_6_3_0>();
  wordBreakTest->test(a);
}

void TestStandardAnalyzer::testSupplementary() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>"});
}

void TestStandardAnalyzer::testKorean() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"훈민정음",
                                            std::deque<wstring>{L"훈민정음"},
                                            std::deque<wstring>{L"<HANGUL>"});
}

void TestStandardAnalyzer::testJapanese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"仮名遣い カタカナ",
      std::deque<wstring>{L"仮", L"名", L"遣", L"い", L"カタカナ"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<HIRAGANA>", L"<KATAKANA>"});
}

void TestStandardAnalyzer::testCombiningMarks() 
{
  checkOneTerm(a, L"ざ", L"ざ"); // hiragana
  checkOneTerm(a, L"ザ", L"ザ"); // katakana
  checkOneTerm(a, L"壹゙", L"壹゙"); // ideographic
  checkOneTerm(a, L"아゙", L"아゙"); // hangul
}

void TestStandardAnalyzer::testMid() 
{
  // ':' is in \p{WB:MidLetter}, which should trigger a split unless there is a
  // Letter char on both sides
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A:B",
                                            std::deque<wstring>{L"A:B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A::B",
                                            std::deque<wstring>{L"A", L"B"});

  // '.' is in \p{WB:MidNumLet}, which should trigger a split unless there is a
  // Letter or Numeric char on both sides
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1.2",
                                            std::deque<wstring>{L"1.2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A.B",
                                            std::deque<wstring>{L"A.B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1..2",
                                            std::deque<wstring>{L"1", L"2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A..B",
                                            std::deque<wstring>{L"A", L"B"});

  // ',' is in \p{WB:MidNum}, which should trigger a split unless there is a
  // Numeric char on both sides
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1,2",
                                            std::deque<wstring>{L"1,2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1,,2",
                                            std::deque<wstring>{L"1", L"2"});

  // Mixed consecutive \p{WB:MidLetter} and \p{WB:MidNumLet} should trigger a
  // split
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A.:B",
                                            std::deque<wstring>{L"A", L"B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A:.B",
                                            std::deque<wstring>{L"A", L"B"});

  // Mixed consecutive \p{WB:MidNum} and \p{WB:MidNumLet} should trigger a split
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1,.2",
                                            std::deque<wstring>{L"1", L"2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1.,2",
                                            std::deque<wstring>{L"1", L"2"});

  // '_' is in \p{WB:ExtendNumLet}

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A:B_A:B",
                                            std::deque<wstring>{L"A:B_A:B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"A:B_A::B", std::deque<wstring>{L"A:B_A", L"B"});

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1.2_1.2",
                                            std::deque<wstring>{L"1.2_1.2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"A.B_A.B",
                                            std::deque<wstring>{L"A.B_A.B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"1.2_1..2", std::deque<wstring>{L"1.2_1", L"2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"A.B_A..B", std::deque<wstring>{L"A.B_A", L"B"});

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"1,2_1,2",
                                            std::deque<wstring>{L"1,2_1,2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"1,2_1,,2", std::deque<wstring>{L"1,2_1", L"2"});

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"C_A.:B",
                                            std::deque<wstring>{L"C_A", L"B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"C_A:.B",
                                            std::deque<wstring>{L"C_A", L"B"});

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"3_1,.2",
                                            std::deque<wstring>{L"3_1", L"2"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"3_1.,2",
                                            std::deque<wstring>{L"3_1", L"2"});
}

void TestStandardAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<StandardAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

void TestStandardAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<StandardAnalyzer>();
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}

void TestStandardAnalyzer::testRandomHugeStringsGraphAfter() throw(
    runtime_error)
{
  shared_ptr<Random> random = TestStandardAnalyzer::random();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random, analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}

TestStandardAnalyzer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestStandardAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestStandardAnalyzer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<StandardTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  shared_ptr<TokenStream> tokenStream =
      make_shared<MockGraphTokenFilter>(random(), tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenStream);
}

void TestStandardAnalyzer::testNormalize()
{
  shared_ptr<Analyzer> a = make_shared<StandardAnalyzer>();
  TestUtil::assertEquals(make_shared<BytesRef>(L"\"\\à3[]()! cz@"),
                         a->normalize(L"dummy", L"\"\\À3[]()! Cz@"));
}

void TestStandardAnalyzer::testMaxTokenLengthDefault() 
{
  shared_ptr<StandardAnalyzer> a = make_shared<StandardAnalyzer>();

  shared_ptr<StringBuilder> bToken = make_shared<StringBuilder>();
  // exact max length:
  for (int i = 0; i < StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH; i++) {
    bToken->append(L'b');
  }

  wstring bString = bToken->toString();
  // first bString is exact max default length; next one is 1 too long
  wstring input = L"x " + bString + L" " + bString + L"b";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertAnalyzesTo(a, input.toString(),
                   std::deque<wstring>{L"x", bString, bString, L"b"});
  delete a;
}

void TestStandardAnalyzer::testMaxTokenLengthNonDefault() 
{
  shared_ptr<StandardAnalyzer> a = make_shared<StandardAnalyzer>();
  a->setMaxTokenLength(5);
  assertAnalyzesTo(
      a, L"ab cd toolong xy z",
      std::deque<wstring>{L"ab", L"cd", L"toolo", L"ng", L"xy", L"z"});
  delete a;
}
} // namespace org::apache::lucene::analysis::standard