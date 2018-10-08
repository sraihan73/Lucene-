using namespace std;

#include "TestUAX29URLEmailTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/standard/WordBreakTestUnicode_6_3_0.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/standard/UAX29URLEmailTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using UAX29URLEmailTokenizer =
    org::apache::lucene::analysis::standard::UAX29URLEmailTokenizer;
using WordBreakTestUnicode_6_3_0 =
    org::apache::lucene::analysis::standard::WordBreakTestUnicode_6_3_0;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testLongEMAILatomText() throws Exception
void TestUAX29URLEmailTokenizer::testLongEMAILatomText() 
{
  // EMAILatomText = [A-Za-z0-9!#$%&'*+-/=?\^_`{|}~]
  std::deque<wchar_t> emailAtomChars =
      (wstring(L"!#$%&'*+,-./"
               L"0123456789=?ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`"
               L"abcdefghijklmnopqrstuvwxyz{|}~"))
          .toCharArray();
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int numChars = TestUtil::nextInt(random(), 100 * 1024, 3 * 1024 * 1024);
  for (int i = 0; i < numChars; ++i) {
    builder->append(emailAtomChars[random()->nextInt(emailAtomChars.size())]);
  }
  int tokenCount = 0;
  shared_ptr<UAX29URLEmailTokenizer> ts = make_shared<UAX29URLEmailTokenizer>();
  wstring text = builder->toString();
  ts->setReader(make_shared<StringReader>(text));
  ts->reset();
  while (ts->incrementToken()) {
    tokenCount++;
  }
  ts->end();
  delete ts;
  assertTrue(tokenCount > 0);

  tokenCount = 0;
  int newBufferSize = TestUtil::nextInt(random(), 200, 8192);
  ts->setMaxTokenLength(newBufferSize);
  ts->setReader(make_shared<StringReader>(text));
  ts->reset();
  while (ts->incrementToken()) {
    tokenCount++;
  }
  ts->end();
  delete ts;
  assertTrue(tokenCount > 0);
}

void TestUAX29URLEmailTokenizer::testHugeDoc() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wchar_t> whitespace(4094);
  Arrays::fill(whitespace, L' ');
  sb->append(whitespace);
  sb->append(L"testing 1234");
  wstring input = sb->toString();
  shared_ptr<UAX29URLEmailTokenizer> tokenizer =
      make_shared<UAX29URLEmailTokenizer>(newAttributeFactory());
  tokenizer->setReader(make_shared<StringReader>(input));
  BaseTokenStreamTestCase::assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"testing", L"1234"});
}

void TestUAX29URLEmailTokenizer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  urlAnalyzer = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  emailAnalyzer = make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
}

TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestUAX29URLEmailTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<UAX29URLEmailTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestUAX29URLEmailTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<UAX29URLEmailTokenizer> tokenizer =
      make_shared<UAX29URLEmailTokenizer>(
          BaseTokenStreamTestCase::newAttributeFactory());
  tokenizer->setMaxTokenLength(
      UAX29URLEmailTokenizer::MAX_TOKEN_LENGTH_LIMIT); // Tokenize arbitrary
                                                       // length URLs
  shared_ptr<TokenFilter> filter = make_shared<URLFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestUAX29URLEmailTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<UAX29URLEmailTokenizer> tokenizer =
      make_shared<UAX29URLEmailTokenizer>(
          BaseTokenStreamTestCase::newAttributeFactory());
  shared_ptr<TokenFilter> filter = make_shared<EmailFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void TestUAX29URLEmailTokenizer::tearDown() 
{
  IOUtils::close({a, urlAnalyzer, emailAnalyzer});
  BaseTokenStreamTestCase::tearDown();
}

TestUAX29URLEmailTokenizer::URLFilter::URLFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TestUAX29URLEmailTokenizer::URLFilter::incrementToken() 
{
  bool isTokenAvailable = false;
  while (input->incrementToken()) {
    if (typeAtt->type() ==
        UAX29URLEmailTokenizer::TOKEN_TYPES[UAX29URLEmailTokenizer::URL]) {
      isTokenAvailable = true;
      break;
    }
  }
  return isTokenAvailable;
}

TestUAX29URLEmailTokenizer::EmailFilter::EmailFilter(
    shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TestUAX29URLEmailTokenizer::EmailFilter::incrementToken() throw(
    IOException)
{
  bool isTokenAvailable = false;
  while (input->incrementToken()) {
    if (typeAtt->type() ==
        UAX29URLEmailTokenizer::TOKEN_TYPES[UAX29URLEmailTokenizer::EMAIL]) {
      isTokenAvailable = true;
      break;
    }
  }
  return isTokenAvailable;
}

void TestUAX29URLEmailTokenizer::testArmenian() 
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

void TestUAX29URLEmailTokenizer::testAmharic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ዊኪፔድያ የባለ ብዙ ቋንቋ የተሟላ ትክክለኛና ነጻ መዝገበ ዕውቀት (ኢንሳይክሎፒዲያ) ነው። ማንኛውም",
      std::deque<wstring>{L"ዊኪፔድያ", L"የባለ", L"ብዙ", L"ቋንቋ", L"የተሟላ", L"ትክክለኛና",
                           L"ነጻ", L"መዝገበ", L"ዕውቀት", L"ኢንሳይክሎፒዲያ", L"ነው",
                           L"ማንኛውም"});
}

void TestUAX29URLEmailTokenizer::testArabic() 
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

void TestUAX29URLEmailTokenizer::testAramaic() 
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

void TestUAX29URLEmailTokenizer::testBengali() 
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

void TestUAX29URLEmailTokenizer::testFarsi() 
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

void TestUAX29URLEmailTokenizer::testGreek() 
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

void TestUAX29URLEmailTokenizer::testThai() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"การที่ได้ต้องแสดงว่างานดี. แล้วเธอจะไปไหน? ๑๒๓๔",
      std::deque<wstring>{L"การที่ได้ต้องแสดงว่างานดี", L"แล้วเธอจะไปไหน", L"๑๒๓๔"});
}

void TestUAX29URLEmailTokenizer::testLao() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ສາທາລະນະລັດ ປະຊາທິປະໄຕ ປະຊາຊົນລາວ",
      std::deque<wstring>{L"ສາທາລະນະລັດ", L"ປະຊາທິປະໄຕ", L"ປະຊາຊົນລາວ"});
}

void TestUAX29URLEmailTokenizer::testTibetan() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །",
      std::deque<wstring>{L"སྣོན", L"མཛོད", L"དང", L"ལས",  L"འདིས", L"བོད",  L"ཡིག",
                           L"མི",  L"ཉམས", L"གོང", L"འཕེལ", L"དུ",   L"གཏོང", L"བར",
                           L"ཧ",  L"ཅང",  L"དགེ", L"མཚན", L"མཆིས", L"སོ"});
}

void TestUAX29URLEmailTokenizer::testChinese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"我是中国人。 １２３４ Ｔｅｓｔｓ ",
      std::deque<wstring>{L"我", L"是", L"中", L"国", L"人", L"１２３４",
                           L"Ｔｅｓｔｓ"});
}

void TestUAX29URLEmailTokenizer::testEmpty() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L".", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L" ", std::deque<wstring>());
}

void TestUAX29URLEmailTokenizer::testLUCENE1545() 
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

void TestUAX29URLEmailTokenizer::testAlphanumericSA() 
{
  // alphanumeric tokens
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"B2B",
                                            std::deque<wstring>{L"B2B"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"2B",
                                            std::deque<wstring>{L"2B"});
}

void TestUAX29URLEmailTokenizer::testDelimitersSA() 
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

void TestUAX29URLEmailTokenizer::testApostrophesSA() 
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

void TestUAX29URLEmailTokenizer::testNumericSA() 
{
  // floating point, serial, model numbers, ip addresses, etc.
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"21.35",
                                            std::deque<wstring>{L"21.35"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"R2D2 C3PO", std::deque<wstring>{L"R2D2", L"C3PO"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"216.239.63.104", std::deque<wstring>{L"216.239.63.104"});
}

void TestUAX29URLEmailTokenizer::testTextWithNumbersSA() 
{
  // numbers
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"});
}

void TestUAX29URLEmailTokenizer::testVariousTextSA() 
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

void TestUAX29URLEmailTokenizer::testKoreanSA() 
{
  // Korean words
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"안녕하세요 한글입니다",
      std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestUAX29URLEmailTokenizer::testOffsets() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
      std::deque<int>{0, 6, 10, 15}, std::deque<int>{5, 9, 14, 20});
}

void TestUAX29URLEmailTokenizer::testTypes() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"David", L"has", L"5000", L"bones"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<NUM>",
                           L"<ALPHANUM>"});
}

void TestUAX29URLEmailTokenizer::testWikiURLs() 
{
  shared_ptr<Reader> reader = nullptr;
  wstring luceneResourcesWikiPage;
  try {
    reader = make_shared<InputStreamReader>(
        getClass().getResourceAsStream(L"LuceneResourcesWikiPage.html"),
        StandardCharsets::UTF_8);
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    std::deque<wchar_t> buffer(1024);
    int numCharsRead;
    while (-1 != (numCharsRead = reader->read(buffer))) {
      builder->append(buffer, 0, numCharsRead);
    }
    luceneResourcesWikiPage = builder->toString();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != reader) {
      reader->close();
    }
  }
  assertTrue(L"" != luceneResourcesWikiPage &&
             luceneResourcesWikiPage.length() > 0);
  shared_ptr<BufferedReader> bufferedReader = nullptr;
  std::deque<wstring> urls;
  try {
    deque<wstring> urlList = deque<wstring>();
    bufferedReader = make_shared<BufferedReader>(make_shared<InputStreamReader>(
        getClass().getResourceAsStream(L"LuceneResourcesWikiPageURLs.txt"),
        StandardCharsets::UTF_8));
    wstring line;
    while (L"" != (line = bufferedReader->readLine())) {
      line = StringHelper::trim(line);
      if (line.length() > 0) {
        urlList.push_back(line);
      }
    }
    urls = urlList.toArray(std::deque<wstring>(urlList.size()));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != bufferedReader) {
      bufferedReader->close();
    }
  }
  assertTrue(nullptr != urls && urls.size() > 0);
  BaseTokenStreamTestCase::assertAnalyzesTo(urlAnalyzer,
                                            luceneResourcesWikiPage, urls);
}

void TestUAX29URLEmailTokenizer::testEmails() 
{
  shared_ptr<Reader> reader = nullptr;
  wstring randomTextWithEmails;
  try {
    reader = make_shared<InputStreamReader>(
        getClass().getResourceAsStream(L"random.text.with.email.addresses.txt"),
        StandardCharsets::UTF_8);
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    std::deque<wchar_t> buffer(1024);
    int numCharsRead;
    while (-1 != (numCharsRead = reader->read(buffer))) {
      builder->append(buffer, 0, numCharsRead);
    }
    randomTextWithEmails = builder->toString();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != reader) {
      reader->close();
    }
  }
  assertTrue(L"" != randomTextWithEmails && randomTextWithEmails.length() > 0);
  shared_ptr<BufferedReader> bufferedReader = nullptr;
  std::deque<wstring> emails;
  try {
    deque<wstring> emailList = deque<wstring>();
    bufferedReader = make_shared<BufferedReader>(make_shared<InputStreamReader>(
        getClass().getResourceAsStream(
            L"email.addresses.from.random.text.with.email.addresses.txt"),
        StandardCharsets::UTF_8));
    wstring line;
    while (L"" != (line = bufferedReader->readLine())) {
      line = StringHelper::trim(line);
      if (line.length() > 0) {
        emailList.push_back(line);
      }
    }
    emails = emailList.toArray(std::deque<wstring>(emailList.size()));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != bufferedReader) {
      bufferedReader->close();
    }
  }
  assertTrue(nullptr != emails && emails.size() > 0);
  BaseTokenStreamTestCase::assertAnalyzesTo(emailAnalyzer, randomTextWithEmails,
                                            emails);
}

void TestUAX29URLEmailTokenizer::testMailtoSchemeEmails() 
{
  // See LUCENE-3880
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"mailto:test@example.org",
      std::deque<wstring>{L"mailto", L"test@example.org"},
      std::deque<wstring>{L"<ALPHANUM>", L"<EMAIL>"});

  // TODO: Support full mailto: scheme URIs. See RFC 6068:
  // http://tools.ietf.org/html/rfc6068
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      wstring(L"mailto:personA@example.com,personB@example.com?cc=personC@"
              L"example.com") +
          L"&subject=Subjectivity&body=Corpusivity%20or%20something%20like%"
          L"20that",
      std::deque<wstring>{L"mailto", L"personA@example.com",
                           L",personB@example.com", L"?cc=personC@example.com",
                           L"subject", L"Subjectivity", L"body", L"Corpusivity",
                           L"20or", L"20something", L"20like", L"20that"},
      std::deque<wstring>{L"<ALPHANUM>", L"<EMAIL>", L"<EMAIL>", L"<EMAIL>",
                           L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<ALPHANUM>"});
}

void TestUAX29URLEmailTokenizer::testURLs() 
{
  shared_ptr<Reader> reader = nullptr;
  wstring randomTextWithURLs;
  try {
    reader = make_shared<InputStreamReader>(
        getClass().getResourceAsStream(L"random.text.with.urls.txt"),
        StandardCharsets::UTF_8);
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    std::deque<wchar_t> buffer(1024);
    int numCharsRead;
    while (-1 != (numCharsRead = reader->read(buffer))) {
      builder->append(buffer, 0, numCharsRead);
    }
    randomTextWithURLs = builder->toString();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != reader) {
      reader->close();
    }
  }
  assertTrue(L"" != randomTextWithURLs && randomTextWithURLs.length() > 0);
  shared_ptr<BufferedReader> bufferedReader = nullptr;
  std::deque<wstring> urls;
  try {
    deque<wstring> urlList = deque<wstring>();
    bufferedReader = make_shared<BufferedReader>(make_shared<InputStreamReader>(
        getClass().getResourceAsStream(L"urls.from.random.text.with.urls.txt"),
        StandardCharsets::UTF_8));
    wstring line;
    while (L"" != (line = bufferedReader->readLine())) {
      line = StringHelper::trim(line);
      if (line.length() > 0) {
        urlList.push_back(line);
      }
    }
    urls = urlList.toArray(std::deque<wstring>(urlList.size()));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != bufferedReader) {
      bufferedReader->close();
    }
  }
  assertTrue(nullptr != urls && urls.size() > 0);
  BaseTokenStreamTestCase::assertAnalyzesTo(urlAnalyzer, randomTextWithURLs,
                                            urls);
}

void TestUAX29URLEmailTokenizer::testUnicodeWordBreaks() 
{
  shared_ptr<WordBreakTestUnicode_6_3_0> wordBreakTest =
      make_shared<WordBreakTestUnicode_6_3_0>();
  wordBreakTest->test(a);
}

void TestUAX29URLEmailTokenizer::testSupplementary() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>"});
}

void TestUAX29URLEmailTokenizer::testKorean() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"훈민정음",
                                            std::deque<wstring>{L"훈민정음"},
                                            std::deque<wstring>{L"<HANGUL>"});
}

void TestUAX29URLEmailTokenizer::testJapanese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"仮名遣い カタカナ",
      std::deque<wstring>{L"仮", L"名", L"遣", L"い", L"カタカナ"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<HIRAGANA>", L"<KATAKANA>"});
}

void TestUAX29URLEmailTokenizer::testCombiningMarks() 
{
  checkOneTerm(a, L"ざ", L"ざ"); // hiragana
  checkOneTerm(a, L"ザ", L"ザ"); // katakana
  checkOneTerm(a, L"壹゙", L"壹゙"); // ideographic
  checkOneTerm(a, L"아゙", L"아゙"); // hangul
}

void TestUAX29URLEmailTokenizer::testMid() 
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

void TestUAX29URLEmailTokenizer::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestUAX29URLEmailTokenizer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestUAX29URLEmailTokenizer::random();
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 8192);
}

void TestUAX29URLEmailTokenizer::testExampleURLs() 
{
  std::deque<wstring> TLDs = {L"aaa",
                               L"ac",
                               L"ai",
                               L"aarp",
                               L"abarth",
                               L"abb",
                               L"abbott",
                               L"abbvie",
                               L"abc",
                               L"able",
                               L"abogado",
                               L"abudhabi",
                               L"academy",
                               L"accenture",
                               L"accountant",
                               L"accountants",
                               L"aco",
                               L"active",
                               L"actor",
                               L"ad",
                               L"adac",
                               L"ads",
                               L"adult",
                               L"ae",
                               L"aeg",
                               L"aero",
                               L"aetna",
                               L"af",
                               L"afamilycompany",
                               L"afl",
                               L"africa",
                               L"ag",
                               L"agakhan",
                               L"agency",
                               L"aig",
                               L"aigo",
                               L"airbus",
                               L"airforce",
                               L"airtel",
                               L"akdn",
                               L"al",
                               L"alfaromeo",
                               L"alibaba",
                               L"alipay",
                               L"allfinanz",
                               L"allstate",
                               L"ally",
                               L"alsace",
                               L"alstom",
                               L"am",
                               L"americanexpress",
                               L"americanfamily",
                               L"amex",
                               L"amfam",
                               L"amica",
                               L"amsterdam",
                               L"analytics",
                               L"android",
                               L"anquan",
                               L"anz",
                               L"ao",
                               L"aol",
                               L"apartments",
                               L"app",
                               L"apple",
                               L"aq",
                               L"aquarelle",
                               L"ar",
                               L"arab",
                               L"aramco",
                               L"archi",
                               L"army",
                               L"arpa",
                               L"art",
                               L"arte",
                               L"as",
                               L"asda",
                               L"asia",
                               L"associates",
                               L"at",
                               L"athleta",
                               L"attorney",
                               L"au",
                               L"auction",
                               L"audi",
                               L"audible",
                               L"audio",
                               L"auspost",
                               L"author",
                               L"auto",
                               L"autos",
                               L"avianca",
                               L"aw",
                               L"aws",
                               L"ax",
                               L"axa",
                               L"az",
                               L"azure",
                               L"ba",
                               L"baby",
                               L"baidu",
                               L"banamex",
                               L"bananarepublic",
                               L"band",
                               L"bank",
                               L"bar",
                               L"barcelona",
                               L"barclaycard",
                               L"barclays",
                               L"barefoot",
                               L"bargains",
                               L"baseball",
                               L"basketball",
                               L"bauhaus",
                               L"bayern",
                               L"bb",
                               L"bbc",
                               L"bbt",
                               L"bbva",
                               L"bcg",
                               L"bcn",
                               L"bd",
                               L"be",
                               L"beats",
                               L"beauty",
                               L"beer",
                               L"bentley",
                               L"berlin",
                               L"best",
                               L"bestbuy",
                               L"bet",
                               L"bf",
                               L"bg",
                               L"bh",
                               L"bharti",
                               L"bi",
                               L"bible",
                               L"bid",
                               L"bike",
                               L"bing",
                               L"bingo",
                               L"bio",
                               L"biz",
                               L"bj",
                               L"black",
                               L"blackfriday",
                               L"blanco",
                               L"blockbuster",
                               L"blog",
                               L"bloomberg",
                               L"blue",
                               L"bm",
                               L"bms",
                               L"bmw",
                               L"bn",
                               L"bnl",
                               L"bnpparibas",
                               L"bo",
                               L"boats",
                               L"boehringer",
                               L"bofa",
                               L"bom",
                               L"bond",
                               L"boo",
                               L"book",
                               L"booking",
                               L"bosch",
                               L"bostik",
                               L"boston",
                               L"bot",
                               L"boutique",
                               L"box",
                               L"br",
                               L"bradesco",
                               L"bridgestone",
                               L"broadway",
                               L"broker",
                               L"brother",
                               L"brussels",
                               L"bs",
                               L"bt",
                               L"budapest",
                               L"bugatti",
                               L"build",
                               L"builders",
                               L"business",
                               L"buy",
                               L"buzz",
                               L"bv",
                               L"bw",
                               L"by",
                               L"bz",
                               L"bzh",
                               L"ca",
                               L"cab",
                               L"cafe",
                               L"cal",
                               L"call",
                               L"calvinklein",
                               L"cam",
                               L"camera",
                               L"camp",
                               L"cancerresearch",
                               L"canon",
                               L"capetown",
                               L"capital",
                               L"capitalone",
                               L"car",
                               L"caravan",
                               L"cards",
                               L"care",
                               L"career",
                               L"careers",
                               L"cars",
                               L"cartier",
                               L"casa",
                               L"case",
                               L"caseih",
                               L"cash",
                               L"casino",
                               L"cat",
                               L"catering",
                               L"catholic",
                               L"cba",
                               L"cbn",
                               L"cbre",
                               L"cbs",
                               L"cc",
                               L"cd",
                               L"ceb",
                               L"center",
                               L"ceo",
                               L"cern",
                               L"cf",
                               L"cfa",
                               L"cfd",
                               L"cg",
                               L"ch",
                               L"chanel",
                               L"channel",
                               L"chase",
                               L"chat",
                               L"cheap",
                               L"chintai",
                               L"christmas",
                               L"chrome",
                               L"chrysler",
                               L"church",
                               L"ci",
                               L"cipriani",
                               L"circle",
                               L"cisco",
                               L"citadel",
                               L"citi",
                               L"citic",
                               L"city",
                               L"cityeats",
                               L"ck",
                               L"cl",
                               L"claims",
                               L"cleaning",
                               L"click",
                               L"clinic",
                               L"clinique",
                               L"clothing",
                               L"cloud",
                               L"club",
                               L"clubmed",
                               L"cm",
                               L"cn",
                               L"co",
                               L"coach",
                               L"codes",
                               L"coffee",
                               L"college",
                               L"cologne",
                               L"com",
                               L"comcast",
                               L"commbank",
                               L"community",
                               L"company",
                               L"compare",
                               L"computer",
                               L"comsec",
                               L"condos",
                               L"construction",
                               L"consulting",
                               L"contact",
                               L"contractors",
                               L"cooking",
                               L"cookingchannel",
                               L"cool",
                               L"coop",
                               L"corsica",
                               L"country",
                               L"coupon",
                               L"coupons",
                               L"courses",
                               L"cr",
                               L"credit",
                               L"creditcard",
                               L"creditunion",
                               L"cricket",
                               L"crown",
                               L"crs",
                               L"cruise",
                               L"cruises",
                               L"csc",
                               L"cu",
                               L"cuisinella",
                               L"cv",
                               L"cw",
                               L"cx",
                               L"cy",
                               L"cymru",
                               L"cyou",
                               L"cz",
                               L"dabur",
                               L"dad",
                               L"dance",
                               L"data",
                               L"date",
                               L"dating",
                               L"datsun",
                               L"day",
                               L"dclk",
                               L"dds",
                               L"de",
                               L"deal",
                               L"dealer",
                               L"deals",
                               L"degree",
                               L"delivery",
                               L"dell",
                               L"deloitte",
                               L"delta",
                               L"democrat",
                               L"dental",
                               L"dentist",
                               L"desi",
                               L"design",
                               L"dev",
                               L"dhl",
                               L"diamonds",
                               L"diet",
                               L"digital",
                               L"direct",
                               L"directory",
                               L"discount",
                               L"discover",
                               L"dish",
                               L"diy",
                               L"dj",
                               L"dk",
                               L"dm",
                               L"dnp",
                               L"do",
                               L"docs",
                               L"doctor",
                               L"dodge",
                               L"dog",
                               L"doha",
                               L"domains",
                               L"dot",
                               L"download",
                               L"drive",
                               L"dtv",
                               L"dubai",
                               L"duck",
                               L"dunlop",
                               L"duns",
                               L"dupont",
                               L"durban",
                               L"dvag",
                               L"dvr",
                               L"dz",
                               L"earth",
                               L"eat",
                               L"ec",
                               L"eco",
                               L"edeka",
                               L"edu",
                               L"education",
                               L"ee",
                               L"eg",
                               L"email",
                               L"emerck",
                               L"energy",
                               L"engineer",
                               L"engineering",
                               L"enterprises",
                               L"epost",
                               L"epson",
                               L"equipment",
                               L"er",
                               L"ericsson",
                               L"erni",
                               L"es",
                               L"esq",
                               L"estate",
                               L"esurance",
                               L"et",
                               L"etisalat",
                               L"eu",
                               L"eurovision",
                               L"eus",
                               L"events",
                               L"everbank",
                               L"exchange",
                               L"expert",
                               L"exposed",
                               L"express",
                               L"extraspace",
                               L"fage",
                               L"fail",
                               L"fairwinds",
                               L"faith",
                               L"family",
                               L"fan",
                               L"fans",
                               L"farm",
                               L"farmers",
                               L"fashion",
                               L"fast",
                               L"fedex",
                               L"feedback",
                               L"ferrari",
                               L"ferrero",
                               L"fi",
                               L"fiat",
                               L"fidelity",
                               L"fido",
                               L"film",
                               L"final",
                               L"finance",
                               L"financial",
                               L"fire",
                               L"firestone",
                               L"firmdale",
                               L"fish",
                               L"fishing",
                               L"fit",
                               L"fitness",
                               L"fj",
                               L"fk",
                               L"flickr",
                               L"flights",
                               L"flir",
                               L"florist",
                               L"flowers",
                               L"fly",
                               L"fm",
                               L"fo",
                               L"foo",
                               L"food",
                               L"foodnetwork",
                               L"football",
                               L"ford",
                               L"forex",
                               L"forsale",
                               L"forum",
                               L"foundation",
                               L"fox",
                               L"fr",
                               L"free",
                               L"fresenius",
                               L"frl",
                               L"frogans",
                               L"frontdoor",
                               L"frontier",
                               L"ftr",
                               L"fujitsu",
                               L"fujixerox",
                               L"fun",
                               L"fund",
                               L"furniture",
                               L"futbol",
                               L"fyi",
                               L"ga",
                               L"gal",
                               L"gallery",
                               L"gallo",
                               L"gallup",
                               L"game",
                               L"games",
                               L"gap",
                               L"garden",
                               L"gb",
                               L"gbiz",
                               L"gd",
                               L"gdn",
                               L"ge",
                               L"gea",
                               L"gent",
                               L"genting",
                               L"george",
                               L"gf",
                               L"gg",
                               L"ggee",
                               L"gh",
                               L"gi",
                               L"gift",
                               L"gifts",
                               L"gives",
                               L"giving",
                               L"gl",
                               L"glade",
                               L"glass",
                               L"gle",
                               L"global",
                               L"globo",
                               L"gm",
                               L"gmail",
                               L"gmbh",
                               L"gmo",
                               L"gmx",
                               L"gn",
                               L"godaddy",
                               L"gold",
                               L"goldpoint",
                               L"golf",
                               L"goo",
                               L"goodhands",
                               L"goodyear",
                               L"goog",
                               L"google",
                               L"gop",
                               L"got",
                               L"gov",
                               L"gp",
                               L"gq",
                               L"gr",
                               L"grainger",
                               L"graphics",
                               L"gratis",
                               L"green",
                               L"gripe",
                               L"grocery",
                               L"group",
                               L"gs",
                               L"gt",
                               L"gu",
                               L"guardian",
                               L"gucci",
                               L"guge",
                               L"guide",
                               L"guitars",
                               L"guru",
                               L"gw",
                               L"gy",
                               L"hair",
                               L"hamburg",
                               L"hangout",
                               L"haus",
                               L"hbo",
                               L"hdfc",
                               L"hdfcbank",
                               L"health",
                               L"healthcare",
                               L"help",
                               L"helsinki",
                               L"here",
                               L"hermes",
                               L"hgtv",
                               L"hiphop",
                               L"hisamitsu",
                               L"hitachi",
                               L"hiv",
                               L"hk",
                               L"hkt",
                               L"hm",
                               L"hn",
                               L"hockey",
                               L"holdings",
                               L"holiday",
                               L"homedepot",
                               L"homegoods",
                               L"homes",
                               L"homesense",
                               L"honda",
                               L"honeywell",
                               L"horse",
                               L"hospital",
                               L"host",
                               L"hosting",
                               L"hot",
                               L"hoteles",
                               L"hotels",
                               L"hotmail",
                               L"house",
                               L"how",
                               L"hr",
                               L"hsbc",
                               L"ht",
                               L"hu",
                               L"hughes",
                               L"hyatt",
                               L"hyundai",
                               L"ibm",
                               L"icbc",
                               L"ice",
                               L"icu",
                               L"id",
                               L"ie",
                               L"ieee",
                               L"ifm",
                               L"ikano",
                               L"il",
                               L"im",
                               L"imamat",
                               L"imdb",
                               L"immo",
                               L"immobilien",
                               L"in",
                               L"industries",
                               L"infiniti",
                               L"info",
                               L"ing",
                               L"ink",
                               L"institute",
                               L"insurance",
                               L"insure",
                               L"int",
                               L"intel",
                               L"international",
                               L"intuit",
                               L"investments",
                               L"io",
                               L"ipiranga",
                               L"iq",
                               L"ir",
                               L"irish",
                               L"is",
                               L"iselect",
                               L"ismaili",
                               L"ist",
                               L"istanbul",
                               L"it",
                               L"itau",
                               L"itv",
                               L"iveco",
                               L"iwc",
                               L"jaguar",
                               L"java",
                               L"jcb",
                               L"jcp",
                               L"je",
                               L"jeep",
                               L"jetzt",
                               L"jewelry",
                               L"jio",
                               L"jlc",
                               L"jll",
                               L"jm",
                               L"jmp",
                               L"jnj",
                               L"jo",
                               L"jobs",
                               L"joburg",
                               L"jot",
                               L"joy",
                               L"jp",
                               L"jpmorgan",
                               L"jprs",
                               L"juegos",
                               L"juniper",
                               L"kaufen",
                               L"kddi",
                               L"ke",
                               L"kerryhotels",
                               L"kerrylogistics",
                               L"kerryproperties",
                               L"kfh",
                               L"kg",
                               L"kh",
                               L"ki",
                               L"kia",
                               L"kim",
                               L"kinder",
                               L"kindle",
                               L"kitchen",
                               L"kiwi",
                               L"km",
                               L"kn",
                               L"koeln",
                               L"komatsu",
                               L"kosher",
                               L"kp",
                               L"kpmg",
                               L"kpn",
                               L"kr",
                               L"krd",
                               L"kred",
                               L"kuokgroup",
                               L"kw",
                               L"ky",
                               L"kyoto",
                               L"kz",
                               L"la",
                               L"lacaixa",
                               L"ladbrokes",
                               L"lamborghini",
                               L"lamer",
                               L"lancaster",
                               L"lancia",
                               L"lancome",
                               L"land",
                               L"landrover",
                               L"lanxess",
                               L"lasalle",
                               L"lat",
                               L"latino",
                               L"latrobe",
                               L"law",
                               L"lawyer",
                               L"lb",
                               L"lc",
                               L"lds",
                               L"lease",
                               L"leclerc",
                               L"lefrak",
                               L"legal",
                               L"lego",
                               L"lexus",
                               L"lgbt",
                               L"li",
                               L"liaison",
                               L"lidl",
                               L"life",
                               L"lifeinsurance",
                               L"lifestyle",
                               L"lighting",
                               L"like",
                               L"lilly",
                               L"limited",
                               L"limo",
                               L"lincoln",
                               L"linde",
                               L"link",
                               L"lipsy",
                               L"live",
                               L"living",
                               L"lixil",
                               L"lk",
                               L"llc",
                               L"loan",
                               L"loans",
                               L"locker",
                               L"locus",
                               L"loft",
                               L"lol",
                               L"london",
                               L"lotte",
                               L"lotto",
                               L"love",
                               L"lpl",
                               L"lplfinancial",
                               L"lr",
                               L"ls",
                               L"lt",
                               L"ltd",
                               L"ltda",
                               L"lu",
                               L"lundbeck",
                               L"lupin",
                               L"luxe",
                               L"luxury",
                               L"lv",
                               L"ly",
                               L"ma",
                               L"macys",
                               L"madrid",
                               L"maif",
                               L"maison",
                               L"makeup",
                               L"man",
                               L"management",
                               L"mango",
                               L"map_obj",
                               L"market",
                               L"marketing",
                               L"markets",
                               L"marriott",
                               L"marshalls",
                               L"maserati",
                               L"mattel",
                               L"mba",
                               L"mc",
                               L"mckinsey",
                               L"md",
                               L"me",
                               L"med",
                               L"media",
                               L"meet",
                               L"melbourne",
                               L"meme",
                               L"memorial",
                               L"men",
                               L"menu",
                               L"merckmsd",
                               L"metlife",
                               L"mg",
                               L"mh",
                               L"miami",
                               L"microsoft",
                               L"mil",
                               L"mini",
                               L"mint",
                               L"mit",
                               L"mitsubishi",
                               L"mk",
                               L"ml",
                               L"mlb",
                               L"mls",
                               L"mm",
                               L"mma",
                               L"mn",
                               L"mo",
                               L"mobi",
                               L"mobile",
                               L"mobily",
                               L"moda",
                               L"moe",
                               L"moi",
                               L"mom",
                               L"monash",
                               L"money",
                               L"monster",
                               L"mopar",
                               L"mormon",
                               L"mortgage",
                               L"moscow",
                               L"moto",
                               L"motorcycles",
                               L"mov",
                               L"movie",
                               L"movistar",
                               L"mp",
                               L"mq",
                               L"mr",
                               L"ms",
                               L"msd",
                               L"mt",
                               L"mtn",
                               L"mtr",
                               L"mu",
                               L"museum",
                               L"mutual",
                               L"mv",
                               L"mw",
                               L"mx",
                               L"my",
                               L"mz",
                               L"na",
                               L"nab",
                               L"nadex",
                               L"nagoya",
                               L"name",
                               L"nationwide",
                               L"natura",
                               L"navy",
                               L"nba",
                               L"nc",
                               L"ne",
                               L"nec",
                               L"net",
                               L"netbank",
                               L"netflix",
                               L"network",
                               L"neustar",
                               L"new",
                               L"newholland",
                               L"news",
                               L"next",
                               L"nextdirect",
                               L"nexus",
                               L"nf",
                               L"nfl",
                               L"ng",
                               L"ngo",
                               L"nhk",
                               L"ni",
                               L"nico",
                               L"nike",
                               L"nikon",
                               L"ninja",
                               L"nissan",
                               L"nissay",
                               L"nl",
                               L"no",
                               L"nokia",
                               L"northwesternmutual",
                               L"norton",
                               L"now",
                               L"nowruz",
                               L"nowtv",
                               L"np",
                               L"nr",
                               L"nra",
                               L"nrw",
                               L"ntt",
                               L"nu",
                               L"nyc",
                               L"nz",
                               L"obi",
                               L"observer",
                               L"off",
                               L"office",
                               L"okinawa",
                               L"olayan",
                               L"olayangroup",
                               L"oldnavy",
                               L"ollo",
                               L"om",
                               L"omega",
                               L"one",
                               L"ong",
                               L"onl",
                               L"online",
                               L"onyourside",
                               L"ooo",
                               L"open",
                               L"oracle",
                               L"orange",
                               L"org",
                               L"organic",
                               L"origins",
                               L"osaka",
                               L"otsuka",
                               L"ott",
                               L"ovh",
                               L"pa",
                               L"page",
                               L"panasonic",
                               L"panerai",
                               L"paris",
                               L"pars",
                               L"partners",
                               L"parts",
                               L"party",
                               L"passagens",
                               L"pay",
                               L"pccw",
                               L"pe",
                               L"pet",
                               L"pf",
                               L"pfizer",
                               L"pg",
                               L"ph",
                               L"pharmacy",
                               L"phd",
                               L"philips",
                               L"phone",
                               L"photo",
                               L"photography",
                               L"photos",
                               L"physio",
                               L"piaget",
                               L"pics",
                               L"pictet",
                               L"pictures",
                               L"pid",
                               L"pin",
                               L"ping",
                               L"pink",
                               L"pioneer",
                               L"pizza",
                               L"pk",
                               L"pl",
                               L"place",
                               L"play",
                               L"playstation",
                               L"plumbing",
                               L"plus",
                               L"pm",
                               L"pn",
                               L"pnc",
                               L"pohl",
                               L"poker",
                               L"politie",
                               L"porn",
                               L"post",
                               L"pr",
                               L"pramerica",
                               L"praxi",
                               L"press",
                               L"prime",
                               L"pro",
                               L"prod",
                               L"productions",
                               L"prof",
                               L"progressive",
                               L"promo",
                               L"properties",
                               L"property",
                               L"protection",
                               L"pru",
                               L"prudential",
                               L"ps",
                               L"pt",
                               L"pub",
                               L"pw",
                               L"pwc",
                               L"py",
                               L"qa",
                               L"qpon",
                               L"quebec",
                               L"quest",
                               L"qvc",
                               L"racing",
                               L"radio",
                               L"raid",
                               L"re",
                               L"read",
                               L"realestate",
                               L"realtor",
                               L"realty",
                               L"recipes",
                               L"red",
                               L"redstone",
                               L"redumbrella",
                               L"rehab",
                               L"reise",
                               L"reisen",
                               L"reit",
                               L"reliance",
                               L"ren",
                               L"rent",
                               L"rentals",
                               L"repair",
                               L"report",
                               L"republican",
                               L"rest",
                               L"restaurant",
                               L"review",
                               L"reviews",
                               L"rexroth",
                               L"rich",
                               L"richardli",
                               L"ricoh",
                               L"rightathome",
                               L"ril",
                               L"rio",
                               L"rip",
                               L"rmit",
                               L"ro",
                               L"rocher",
                               L"rocks",
                               L"rodeo",
                               L"rogers",
                               L"room",
                               L"rs",
                               L"rsvp",
                               L"ru",
                               L"rugby",
                               L"ruhr",
                               L"run",
                               L"rw",
                               L"rwe",
                               L"ryukyu",
                               L"sa",
                               L"saarland",
                               L"safe",
                               L"safety",
                               L"sakura",
                               L"sale",
                               L"salon",
                               L"samsclub",
                               L"samsung",
                               L"sandvik",
                               L"sandvikcoromant",
                               L"sanofi",
                               L"sap",
                               L"sarl",
                               L"sas",
                               L"save",
                               L"saxo",
                               L"sb",
                               L"sbi",
                               L"sbs",
                               L"sc",
                               L"sca",
                               L"scb",
                               L"schaeffler",
                               L"schmidt",
                               L"scholarships",
                               L"school",
                               L"schule",
                               L"schwarz",
                               L"science",
                               L"scjohnson",
                               L"scor",
                               L"scot",
                               L"sd",
                               L"se",
                               L"search",
                               L"seat",
                               L"secure",
                               L"security",
                               L"seek",
                               L"select",
                               L"sener",
                               L"services",
                               L"ses",
                               L"seven",
                               L"sew",
                               L"sex",
                               L"sexy",
                               L"sfr",
                               L"sg",
                               L"sh",
                               L"shangrila",
                               L"sharp",
                               L"shaw",
                               L"shell",
                               L"shia",
                               L"shiksha",
                               L"shoes",
                               L"shop",
                               L"shopping",
                               L"shouji",
                               L"show",
                               L"showtime",
                               L"shriram",
                               L"si",
                               L"silk",
                               L"sina",
                               L"singles",
                               L"site",
                               L"sj",
                               L"sk",
                               L"ski",
                               L"skin",
                               L"sky",
                               L"skype",
                               L"sl",
                               L"sling",
                               L"sm",
                               L"smart",
                               L"smile",
                               L"sn",
                               L"sncf",
                               L"so",
                               L"soccer",
                               L"social",
                               L"softbank",
                               L"software",
                               L"sohu",
                               L"solar",
                               L"solutions",
                               L"song",
                               L"sony",
                               L"soy",
                               L"space",
                               L"spiegel",
                               L"sport",
                               L"spot",
                               L"spreadbetting",
                               L"sr",
                               L"srl",
                               L"srt",
                               L"st",
                               L"stada",
                               L"staples",
                               L"star",
                               L"starhub",
                               L"statebank",
                               L"statefarm",
                               L"statoil",
                               L"stc",
                               L"stcgroup",
                               L"stockholm",
                               L"storage",
                               L"store",
                               L"stream",
                               L"studio",
                               L"study",
                               L"style",
                               L"su",
                               L"sucks",
                               L"supplies",
                               L"supply",
                               L"support",
                               L"surf",
                               L"surgery",
                               L"suzuki",
                               L"sv",
                               L"swatch",
                               L"swiftcover",
                               L"swiss",
                               L"sx",
                               L"sy",
                               L"sydney",
                               L"symantec",
                               L"systems",
                               L"sz",
                               L"tab",
                               L"taipei",
                               L"talk",
                               L"taobao",
                               L"target",
                               L"tatamotors",
                               L"tatar",
                               L"tattoo",
                               L"tax",
                               L"taxi",
                               L"tc",
                               L"tci",
                               L"td",
                               L"tdk",
                               L"team",
                               L"tech",
                               L"technology",
                               L"tel",
                               L"telecity",
                               L"telefonica",
                               L"temasek",
                               L"tennis",
                               L"teva",
                               L"tf",
                               L"tg",
                               L"th",
                               L"thd",
                               L"theater",
                               L"theatre",
                               L"tiaa",
                               L"tickets",
                               L"tienda",
                               L"tiffany",
                               L"tips",
                               L"tires",
                               L"tirol",
                               L"tj",
                               L"tjmaxx",
                               L"tjx",
                               L"tk",
                               L"tkmaxx",
                               L"tl",
                               L"tm",
                               L"tmall",
                               L"tn",
                               L"to",
                               L"today",
                               L"tokyo",
                               L"tools",
                               L"top",
                               L"toray",
                               L"toshiba",
                               L"total",
                               L"tours",
                               L"town",
                               L"toyota",
                               L"toys",
                               L"tr",
                               L"trade",
                               L"trading",
                               L"training",
                               L"travel",
                               L"travelchannel",
                               L"travelers",
                               L"travelersinsurance",
                               L"trust",
                               L"trv",
                               L"tt",
                               L"tube",
                               L"tui",
                               L"tunes",
                               L"tushu",
                               L"tv",
                               L"tvs",
                               L"tw",
                               L"tz",
                               L"ua",
                               L"ubank",
                               L"ubs",
                               L"uconnect",
                               L"ug",
                               L"uk",
                               L"unicom",
                               L"university",
                               L"uno",
                               L"uol",
                               L"ups",
                               L"us",
                               L"uy",
                               L"uz",
                               L"va",
                               L"vacations",
                               L"vana",
                               L"vanguard",
                               L"vc",
                               L"ve",
                               L"vegas",
                               L"ventures",
                               L"verisign",
                               L"versicherung",
                               L"vet",
                               L"vg",
                               L"vi",
                               L"viajes",
                               L"video",
                               L"vig",
                               L"viking",
                               L"villas",
                               L"vin",
                               L"vip",
                               L"virgin",
                               L"visa",
                               L"vision",
                               L"vista",
                               L"vistaprint",
                               L"viva",
                               L"vivo",
                               L"vlaanderen",
                               L"vn",
                               L"vodka",
                               L"volkswagen",
                               L"volvo",
                               L"vote",
                               L"voting",
                               L"voto",
                               L"voyage",
                               L"vu",
                               L"vuelos",
                               L"wales",
                               L"walmart",
                               L"walter",
                               L"wang",
                               L"wanggou",
                               L"warman",
                               L"watch",
                               L"watches",
                               L"weather",
                               L"weatherchannel",
                               L"webcam",
                               L"weber",
                               L"website",
                               L"wed",
                               L"wedding",
                               L"weibo",
                               L"weir",
                               L"wf",
                               L"whoswho",
                               L"wien",
                               L"wiki",
                               L"williamhill",
                               L"win",
                               L"windows",
                               L"wine",
                               L"winners",
                               L"wme",
                               L"wolterskluwer",
                               L"woodside",
                               L"work",
                               L"works",
                               L"world",
                               L"wow",
                               L"ws",
                               L"wtc",
                               L"wtf",
                               L"xbox",
                               L"xerox",
                               L"xfinity",
                               L"xihuan",
                               L"xin",
                               L"xn--11b4c3d",
                               L"xn--1ck2e1b",
                               L"xn--1qqw23a",
                               L"xn--2scrj9c",
                               L"xn--30rr7y",
                               L"xn--3bst00m",
                               L"xn--3ds443g",
                               L"xn--3e0b707e",
                               L"xn--3hcrj9c",
                               L"xn--3oq18vl8pn36a",
                               L"xn--3pxu8k",
                               L"xn--42c2d9a",
                               L"xn--45br5cyl",
                               L"xn--45brj9c",
                               L"xn--45q11c",
                               L"xn--4gbrim",
                               L"xn--54b7fta0cc",
                               L"xn--55qw42g",
                               L"xn--55qx5d",
                               L"xn--5su34j936bgsg",
                               L"xn--5tzm5g",
                               L"xn--6frz82g",
                               L"xn--6qq986b3xl",
                               L"xn--80adxhks",
                               L"xn--80ao21a",
                               L"xn--80aqecdr1a",
                               L"xn--80asehdb",
                               L"xn--80aswg",
                               L"xn--8y0a063a",
                               L"xn--90a3ac",
                               L"xn--90ae",
                               L"xn--90ais",
                               L"xn--9dbq2a",
                               L"xn--9et52u",
                               L"xn--9krt00a",
                               L"xn--b4w605ferd",
                               L"xn--bck1b9a5dre4c",
                               L"xn--c1avg",
                               L"xn--c2br7g",
                               L"xn--cck2b3b",
                               L"xn--cg4bki",
                               L"xn--clchc0ea0b2g2a9gcd",
                               L"xn--czr694b",
                               L"xn--czrs0t",
                               L"xn--czru2d",
                               L"xn--d1acj3b",
                               L"xn--d1alf",
                               L"xn--e1a4c",
                               L"xn--eckvdtc9d",
                               L"xn--efvy88h",
                               L"xn--estv75g",
                               L"xn--fct429k",
                               L"xn--fhbei",
                               L"xn--fiq228c5hs",
                               L"xn--fiq64b",
                               L"xn--fiqs8s",
                               L"xn--fiqz9s",
                               L"xn--fjq720a",
                               L"xn--flw351e",
                               L"xn--fpcrj9c3d",
                               L"xn--fzc2c9e2c",
                               L"xn--fzys8d69uvgm",
                               L"xn--g2xx48c",
                               L"xn--gckr3f0f",
                               L"xn--gecrj9c",
                               L"xn--gk3at1e",
                               L"xn--h2breg3eve",
                               L"xn--h2brj9c",
                               L"xn--h2brj9c8c",
                               L"xn--hxt814e",
                               L"xn--i1b6b1a6a2e",
                               L"xn--imr513n",
                               L"xn--io0a7i",
                               L"xn--j1aef",
                               L"xn--j1amh",
                               L"xn--j6w193g",
                               L"xn--jlq61u9w7b",
                               L"xn--jvr189m",
                               L"xn--kcrx77d1x4a",
                               L"xn--kprw13d",
                               L"xn--kpry57d",
                               L"xn--kpu716f",
                               L"xn--kput3i",
                               L"xn--l1acc",
                               L"xn--lgbbat1ad8j",
                               L"xn--mgb9awbf",
                               L"xn--mgba3a3ejt",
                               L"xn--mgba3a4f16a",
                               L"xn--mgba7c0bbn0a",
                               L"xn--mgbaakc7dvf",
                               L"xn--mgbaam7a8h",
                               L"xn--mgbab2bd",
                               L"xn--mgbai9azgqp6j",
                               L"xn--mgbayh7gpa",
                               L"xn--mgbb9fbpob",
                               L"xn--mgbbh1a",
                               L"xn--mgbbh1a71e",
                               L"xn--mgbc0a9azcg",
                               L"xn--mgbca7dzdo",
                               L"xn--mgberp4a5d4ar",
                               L"xn--mgbgu82a",
                               L"xn--mgbi4ecexp",
                               L"xn--mgbpl2fh",
                               L"xn--mgbt3dhd",
                               L"xn--mgbtx2b",
                               L"xn--mgbx4cd0ab",
                               L"xn--mix891f",
                               L"xn--mk1bu44c",
                               L"xn--mxtq1m",
                               L"xn--ngbc5azd",
                               L"xn--ngbe9e0a",
                               L"xn--ngbrx",
                               L"xn--node",
                               L"xn--nqv7f",
                               L"xn--nqv7fs00ema",
                               L"xn--nyqy26a",
                               L"xn--o3cw4h",
                               L"xn--ogbpf8fl",
                               L"xn--otu796d",
                               L"xn--p1acf",
                               L"xn--p1ai",
                               L"xn--pbt977c",
                               L"xn--pgbs0dh",
                               L"xn--pssy2u",
                               L"xn--q9jyb4c",
                               L"xn--qcka1pmc",
                               L"xn--qxam",
                               L"xn--rhqv96g",
                               L"xn--rovu88b",
                               L"xn--rvc1e0am3e",
                               L"xn--s9brj9c",
                               L"xn--ses554g",
                               L"xn--t60b56a",
                               L"xn--tckwe",
                               L"xn--tiq49xqyj",
                               L"xn--unup4y",
                               L"xn--vermgensberater-ctb",
                               L"xn--vermgensberatung-pwb",
                               L"xn--vhquv",
                               L"xn--vuq861b",
                               L"xn--w4r85el8fhu5dnra",
                               L"xn--w4rs40l",
                               L"xn--wgbh1c",
                               L"xn--wgbl6a",
                               L"xn--xhq521b",
                               L"xn--xkc2al3hye2a",
                               L"xn--xkc2dl3a5ee0h",
                               L"xn--y9a3aq",
                               L"xn--yfro4i67o",
                               L"xn--ygbi2ammx",
                               L"xn--zfr164b",
                               L"xperia",
                               L"xxx",
                               L"xyz",
                               L"yachts",
                               L"yahoo",
                               L"yamaxun",
                               L"yandex",
                               L"ye",
                               L"yodobashi",
                               L"yoga",
                               L"yokohama",
                               L"you",
                               L"youtube",
                               L"yt",
                               L"yun",
                               L"za",
                               L"zappos",
                               L"zara",
                               L"zero",
                               L"zip",
                               L"zippo",
                               L"zm",
                               L"zone",
                               L"zuerich",
                               L"zw"};

  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  for (auto tld : TLDs) {
    wstring URL = L"example." + tld;
    BaseTokenStreamTestCase::assertAnalyzesTo(analyzer, URL,
                                              std::deque<wstring>{URL},
                                              std::deque<wstring>{L"<URL>"});
  }
}

TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestUAX29URLEmailTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestUAX29URLEmailTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<UAX29URLEmailTokenizer>(
          BaseTokenStreamTestCase::newAttributeFactory()));
}
} // namespace org::apache::lucene::analysis::standard