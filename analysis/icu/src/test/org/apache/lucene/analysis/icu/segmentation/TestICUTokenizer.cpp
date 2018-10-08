using namespace std;

#include "TestICUTokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/DefaultICUTokenizerConfig.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizer.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/tokenattributes/ScriptAttribute.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ScriptAttribute =
    org::apache::lucene::analysis::icu::tokenattributes::ScriptAttribute;
using com::ibm::icu::lang::UScript;

void TestICUTokenizer::testHugeDoc() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wchar_t> whitespace(4094);
  Arrays::fill(whitespace, L' ');
  sb->append(whitespace);
  sb->append(L"testing 1234");
  wstring input = sb->toString();
  shared_ptr<ICUTokenizer> tokenizer = make_shared<ICUTokenizer>(
      newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, true));
  tokenizer->setReader(make_shared<StringReader>(input));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"testing", L"1234"});
}

void TestICUTokenizer::testHugeTerm2() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < 40960; i++) {
    sb->append(L'a');
  }
  wstring input = sb->toString();
  shared_ptr<ICUTokenizer> tokenizer = make_shared<ICUTokenizer>(
      newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, true));
  tokenizer->setReader(make_shared<StringReader>(input));
  std::deque<wchar_t> token(4096);
  Arrays::fill(token, L'a');
  wstring expectedToken = wstring(token);
  std::deque<wstring> expected = {expectedToken, expectedToken, expectedToken,
                                   expectedToken, expectedToken, expectedToken,
                                   expectedToken, expectedToken, expectedToken,
                                   expectedToken};
  assertTokenStreamContents(tokenizer, expected);
}

void TestICUTokenizer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestICUTokenizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestICUTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<ICUTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, true));
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestICUTokenizer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestICUTokenizer::testArmenian() 
{
  assertAnalyzesTo(
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

void TestICUTokenizer::testAmharic() 
{
  assertAnalyzesTo(
      a, L"ዊኪፔድያ የባለ ብዙ ቋንቋ የተሟላ ትክክለኛና ነጻ መዝገበ ዕውቀት (ኢንሳይክሎፒዲያ) ነው። ማንኛውም",
      std::deque<wstring>{L"ዊኪፔድያ", L"የባለ", L"ብዙ", L"ቋንቋ", L"የተሟላ", L"ትክክለኛና",
                           L"ነጻ", L"መዝገበ", L"ዕውቀት", L"ኢንሳይክሎፒዲያ", L"ነው",
                           L"ማንኛውም"});
}

void TestICUTokenizer::testArabic() 
{
  assertAnalyzesTo(a,
                   L"الفيلم الوثائقي الأول عن ويكيبيديا يسمى \"الحقيقة "
                   L"بالأرقام: قصة ويكيبيديا\" (بالإنجليزية: Truth in Numbers: "
                   L"The Wikipedia Story)، سيتم إطلاقه في 2008.",
                   std::deque<wstring>{L"الفيلم",    L"الوثائقي",    L"الأول",
                                        L"عن",        L"ويكيبيديا",   L"يسمى",
                                        L"الحقيقة",   L"بالأرقام",    L"قصة",
                                        L"ويكيبيديا", L"بالإنجليزية", L"Truth",
                                        L"in",        L"Numbers",     L"The",
                                        L"Wikipedia", L"Story",       L"سيتم",
                                        L"إطلاقه",    L"في",          L"2008"});
}

void TestICUTokenizer::testAramaic() 
{
  assertAnalyzesTo(a,
                   L"ܘܝܩܝܦܕܝܐ (ܐܢܓܠܝܐ: Wikipedia) ܗܘ ܐܝܢܣܩܠܘܦܕܝܐ ܚܐܪܬܐ ܕܐܢܛܪܢܛ "
                   L"ܒܠܫܢ̈ܐ ܣܓܝܐ̈ܐ܂ ܫܡܗ ܐܬܐ ܡܢ ܡ̈ܠܬܐ ܕ\"ܘܝܩܝ\" ܘ\"ܐܝܢܣܩܠܘܦܕܝܐ\"܀",
                   std::deque<wstring>{L"ܘܝܩܝܦܕܝܐ", L"ܐܢܓܠܝܐ", L"Wikipedia",
                                        L"ܗܘ", L"ܐܝܢܣܩܠܘܦܕܝܐ", L"ܚܐܪܬܐ",
                                        L"ܕܐܢܛܪܢܛ", L"ܒܠܫܢ̈ܐ", L"ܣܓܝܐ̈ܐ", L"ܫܡܗ",
                                        L"ܐܬܐ", L"ܡܢ", L"ܡ̈ܠܬܐ", L"ܕ", L"ܘܝܩܝ",
                                        L"ܘ", L"ܐܝܢܣܩܠܘܦܕܝܐ"});
}

void TestICUTokenizer::testBengali() 
{
  assertAnalyzesTo(a,
                   L"এই বিশ্বকোষ পরিচালনা করে উইকিমিডিয়া ফাউন্ডেশন (একটি অলাভজনক "
                   L"সংস্থা)। উইকিপিডিয়ার শুরু ১৫ জানুয়ারি, ২০০১ সালে। এখন পর্যন্ত "
                   L"২০০টিরও বেশী ভাষায় উইকিপিডিয়া রয়েছে।",
                   std::deque<wstring>{
                       L"এই",         L"বিশ্বকোষ",     L"পরিচালনা", L"করে",
                       L"উইকিমিডিয়া", L"ফাউন্ডেশন",    L"একটি",     L"অলাভজনক",
                       L"সংস্থা",      L"উইকিপিডিয়ার", L"শুরু",       L"১৫",
                       L"জানুয়ারি",    L"২০০১",        L"সালে",     L"এখন",
                       L"পর্যন্ত",      L"২০০টিরও",     L"বেশী",     L"ভাষায়",
                       L"উইকিপিডিয়া", L"রয়েছে"});
}

void TestICUTokenizer::testFarsi() 
{
  assertAnalyzesTo(a,
                   L"ویکی پدیای انگلیسی در تاریخ ۲۵ دی ۱۳۷۹ به صورت مکملی برای "
                   L"دانشنامهٔ تخصصی نوپدیا نوشته شد.",
                   std::deque<wstring>{L"ویکی", L"پدیای", L"انگلیسی", L"در",
                                        L"تاریخ", L"۲۵", L"دی", L"۱۳۷۹", L"به",
                                        L"صورت", L"مکملی", L"برای", L"دانشنامهٔ",
                                        L"تخصصی", L"نوپدیا", L"نوشته", L"شد"});
}

void TestICUTokenizer::testGreek() 
{
  assertAnalyzesTo(
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

void TestICUTokenizer::testKhmer() 
{
  assertAnalyzesTo(
      a, L"ផ្ទះស្កឹមស្កៃបីបួនខ្នងនេះ",
      std::deque<wstring>{L"ផ្ទះ", L"ស្កឹមស្កៃ", L"បី", L"បួន", L"ខ្នង", L"នេះ"});
}

void TestICUTokenizer::testLao() 
{
  assertAnalyzesTo(a, L"ກວ່າດອກ", std::deque<wstring>{L"ກວ່າ", L"ດອກ"});
  assertAnalyzesTo(a, L"ພາສາລາວ", std::deque<wstring>{L"ພາສາ", L"ລາວ"},
                   std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>"});
}

void TestICUTokenizer::testMyanmar() 
{
  assertAnalyzesTo(a, L"သက်ဝင်လှုပ်ရှားစေပြီး",
                   std::deque<wstring>{L"သက်ဝင်", L"လှုပ်ရှား", L"စေ", L"ပြီး"});
}

void TestICUTokenizer::testThai() 
{
  assertAnalyzesTo(a, L"การที่ได้ต้องแสดงว่างานดี. แล้วเธอจะไปไหน? ๑๒๓๔",
                   std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง",
                                        L"ว่า", L"งาน", L"ดี", L"แล้ว", L"เธอ",
                                        L"จะ", L"ไป", L"ไหน", L"๑๒๓๔"});
}

void TestICUTokenizer::testTibetan() 
{
  assertAnalyzesTo(
      a, L"སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །",
      std::deque<wstring>{L"སྣོན", L"མཛོད", L"དང", L"ལས",  L"འདིས", L"བོད",  L"ཡིག",
                           L"མི",  L"ཉམས", L"གོང", L"འཕེལ", L"དུ",   L"གཏོང", L"བར",
                           L"ཧ",  L"ཅང",  L"དགེ", L"མཚན", L"མཆིས", L"སོ"});
}

void TestICUTokenizer::testChinese() 
{
  assertAnalyzesTo(a, L"我是中国人。 １２３４ Ｔｅｓｔｓ ",
                   std::deque<wstring>{L"我", L"是", L"中", L"国", L"人",
                                        L"１２３４", L"Ｔｅｓｔｓ"});
}

void TestICUTokenizer::testHebrew() 
{
  assertAnalyzesTo(a, L"דנקנר תקף את הדו\"ח",
                   std::deque<wstring>{L"דנקנר", L"תקף", L"את", L"הדו\"ח"});
  assertAnalyzesTo(a, L"חברת בת של מודי'ס",
                   std::deque<wstring>{L"חברת", L"בת", L"של", L"מודי'ס"});
}

void TestICUTokenizer::testEmpty() 
{
  assertAnalyzesTo(a, L"", std::deque<wstring>());
  assertAnalyzesTo(a, L".", std::deque<wstring>());
  assertAnalyzesTo(a, L" ", std::deque<wstring>());
}

void TestICUTokenizer::testLUCENE1545() 
{
  /*
   * Standard analyzer does not correctly tokenize combining character U+0364
   * COMBINING LATIN SMALL LETTRE E. The word "moͤchte" is incorrectly tokenized
   * into "mo" "chte", the combining character is lost. Expected result is only
   * on token "moͤchte".
   */
  assertAnalyzesTo(a, L"moͤchte", std::deque<wstring>{L"moͤchte"});
}

void TestICUTokenizer::testAlphanumericSA() 
{
  // alphanumeric tokens
  assertAnalyzesTo(a, L"B2B", std::deque<wstring>{L"B2B"});
  assertAnalyzesTo(a, L"2B", std::deque<wstring>{L"2B"});
}

void TestICUTokenizer::testDelimitersSA() 
{
  // other delimiters: "-", "/", ","
  assertAnalyzesTo(a, L"some-dashed-phrase",
                   std::deque<wstring>{L"some", L"dashed", L"phrase"});
  assertAnalyzesTo(a, L"dogs,chase,cats",
                   std::deque<wstring>{L"dogs", L"chase", L"cats"});
  assertAnalyzesTo(a, L"ac/dc", std::deque<wstring>{L"ac", L"dc"});
}

void TestICUTokenizer::testApostrophesSA() 
{
  // internal apostrophes: O'Reilly, you're, O'Reilly's
  assertAnalyzesTo(a, L"O'Reilly", std::deque<wstring>{L"O'Reilly"});
  assertAnalyzesTo(a, L"you're", std::deque<wstring>{L"you're"});
  assertAnalyzesTo(a, L"she's", std::deque<wstring>{L"she's"});
  assertAnalyzesTo(a, L"Jim's", std::deque<wstring>{L"Jim's"});
  assertAnalyzesTo(a, L"don't", std::deque<wstring>{L"don't"});
  assertAnalyzesTo(a, L"O'Reilly's", std::deque<wstring>{L"O'Reilly's"});
}

void TestICUTokenizer::testNumericSA() 
{
  // floating point, serial, model numbers, ip addresses, etc.
  // every other segment must have at least one digit
  assertAnalyzesTo(a, L"21.35", std::deque<wstring>{L"21.35"});
  assertAnalyzesTo(a, L"R2D2 C3PO", std::deque<wstring>{L"R2D2", L"C3PO"});
  assertAnalyzesTo(a, L"216.239.63.104",
                   std::deque<wstring>{L"216.239.63.104"});
  assertAnalyzesTo(a, L"216.239.63.104",
                   std::deque<wstring>{L"216.239.63.104"});
}

void TestICUTokenizer::testTextWithNumbersSA() 
{
  // numbers
  assertAnalyzesTo(a, L"David has 5000 bones",
                   std::deque<wstring>{L"David", L"has", L"5000", L"bones"});
}

void TestICUTokenizer::testVariousTextSA() 
{
  // various
  assertAnalyzesTo(
      a, L"C embedded developers wanted",
      std::deque<wstring>{L"C", L"embedded", L"developers", L"wanted"});
  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"FOO", L"BAR"});
  assertAnalyzesTo(a, L"foo      bar .  FOO <> BAR",
                   std::deque<wstring>{L"foo", L"bar", L"FOO", L"BAR"});
  assertAnalyzesTo(a, L"\"QUOTED\" word",
                   std::deque<wstring>{L"QUOTED", L"word"});
}

void TestICUTokenizer::testKoreanSA() 
{
  // Korean words
  assertAnalyzesTo(a, L"안녕하세요 한글입니다",
                   std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestICUTokenizer::testReusableTokenStream() 
{
  assertAnalyzesTo(
      a, L"སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །",
      std::deque<wstring>{L"སྣོན", L"མཛོད", L"དང", L"ལས",  L"འདིས", L"བོད",  L"ཡིག",
                           L"མི",  L"ཉམས", L"གོང", L"འཕེལ", L"དུ",   L"གཏོང", L"བར",
                           L"ཧ",  L"ཅང",  L"དགེ", L"མཚན", L"མཆིས", L"སོ"});
}

void TestICUTokenizer::testOffsets() 
{
  assertAnalyzesTo(a, L"David has 5000 bones",
                   std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
                   std::deque<int>{0, 6, 10, 15},
                   std::deque<int>{5, 9, 14, 20});
}

void TestICUTokenizer::testTypes() 
{
  assertAnalyzesTo(a, L"David has 5000 bones",
                   std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
                   std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<NUM>",
                                        L"<ALPHANUM>"});
}

void TestICUTokenizer::testKorean() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"훈민정음",
                                            std::deque<wstring>{L"훈민정음"},
                                            std::deque<wstring>{L"<HANGUL>"});
}

void TestICUTokenizer::testJapanese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"仮名遣い カタカナ",
      std::deque<wstring>{L"仮", L"名", L"遣", L"い", L"カタカナ"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<HIRAGANA>", L"<KATAKANA>"});
}

void TestICUTokenizer::testEmoji() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"💩 💩💩", std::deque<wstring>{L"💩", L"💩", L"💩"},
      std::deque<wstring>{L"<EMOJI>", L"<EMOJI>", L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiSequence() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"👩‍❤️‍👩", std::deque<wstring>{L"👩‍❤️‍👩"},
      std::deque<wstring>{L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiSequenceWithModifier() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"👨🏼‍⚕️", std::deque<wstring>{L"👨🏼‍⚕️"},
      std::deque<wstring>{L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiRegionalIndicator() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"🇺🇸🇺🇸", std::deque<wstring>{L"🇺🇸", L"🇺🇸"},
      std::deque<wstring>{L"<EMOJI>", L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiVariationSequence() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"#️⃣",
                                            std::deque<wstring>{L"#️⃣"},
                                            std::deque<wstring>{L"<EMOJI>"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"3️⃣",
                                            std::deque<wstring>{L"3️⃣"},
                                            std::deque<wstring>{L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiTagSequence() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"🏴󠁧󠁢󠁥󠁮󠁧󠁿",
      std::deque<wstring>{L"🏴󠁧󠁢󠁥󠁮󠁧󠁿"},
      std::deque<wstring>{L"<EMOJI>"});
}

void TestICUTokenizer::testEmojiTokenization() 
{
  // simple emoji around latin
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"poo💩poo", std::deque<wstring>{L"poo", L"💩", L"poo"},
      std::deque<wstring>{L"<ALPHANUM>", L"<EMOJI>", L"<ALPHANUM>"});
  // simple emoji around non-latin
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"💩中國💩", std::deque<wstring>{L"💩", L"中", L"國", L"💩"},
      std::deque<wstring>{L"<EMOJI>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<EMOJI>"});
}

void TestICUTokenizer::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestICUTokenizer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestICUTokenizer::random();
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 8192);
}

void TestICUTokenizer::testTokenAttributes() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // a.tokenStream("dummy", "This is a test"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        a->tokenStream(L"dummy", L"This is a test");
    shared_ptr<ScriptAttribute> scriptAtt =
        ts->addAttribute(ScriptAttribute::typeid);
    ts->reset();
    while (ts->incrementToken()) {
      assertEquals(UScript::LATIN, scriptAtt->getCode());
      assertEquals(UScript::getName(UScript::LATIN), scriptAtt->getName());
      assertEquals(UScript::getShortName(UScript::LATIN),
                   scriptAtt->getShortName());
      assertTrue(ts->reflectAsString(false).find(L"script=Latin") !=
                 wstring::npos);
    }
    ts->end();
  }
}

void TestICUTokenizer::testICUConcurrency() 
{
  int numThreads = 8;
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(shared_from_this(),
                                                        startingGun, i);
    threads[i]->start();
  }
  startingGun->countDown();
  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }
}

TestICUTokenizer::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestICUTokenizer> outerInstance,
    shared_ptr<CountDownLatch> startingGun, int i)
{
  this->outerInstance = outerInstance;
  this->startingGun = startingGun;
  this->i = i;
}

void TestICUTokenizer::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    int64_t tokenCount = 0;
    const wstring contents = L"英 เบียร์ ビール ເບຍ abc";
    for (int i = 0; i < 1000; i++) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Tokenizer
      // tokenizer = new ICUTokenizer())
      {
        org::apache::lucene::analysis::Tokenizer tokenizer = ICUTokenizer();
        tokenizer->setReader(make_shared<StringReader>(contents));
        tokenizer->reset();
        while (tokenizer->incrementToken()) {
          tokenCount++;
        }
        tokenizer->end();
      }
    }
    if (VERBOSE) {
      wcout << tokenCount << endl;
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::analysis::icu::segmentation