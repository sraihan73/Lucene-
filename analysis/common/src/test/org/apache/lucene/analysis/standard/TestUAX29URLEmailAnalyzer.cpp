using namespace std;

#include "TestUAX29URLEmailAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/standard/UAX29URLEmailAnalyzer.h"

namespace org::apache::lucene::analysis::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using UAX29URLEmailAnalyzer =
    org::apache::lucene::analysis::standard::UAX29URLEmailAnalyzer;

void TestUAX29URLEmailAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<UAX29URLEmailAnalyzer>();
}

void TestUAX29URLEmailAnalyzer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestUAX29URLEmailAnalyzer::testHugeDoc() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wchar_t> whitespace(4094);
  Arrays::fill(whitespace, L' ');
  sb->append(whitespace);
  sb->append(L"testing 1234");
  wstring input = sb->toString();
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, input, std::deque<wstring>{L"testing", L"1234"});
}

void TestUAX29URLEmailAnalyzer::testArmenian() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"Վիքիպեդիայի 13 միլիոն հոդվածները (4,600` հայերեն վիքիպեդիայում) գրվել "
      L"են կամավորների կողմից ու համարյա բոլոր հոդվածները կարող է խմբագրել "
      L"ցանկաց մարդ ով կարող է բացել Վիքիպեդիայի կայքը։",
      std::deque<wstring>{L"վիքիպեդիայի",   L"13",     L"միլիոն",
                           L"հոդվածները",    L"4,600",  L"հայերեն",
                           L"վիքիպեդիայում", L"գրվել",  L"են",
                           L"կամավորների",   L"կողմից", L"ու",
                           L"համարյա",       L"բոլոր",  L"հոդվածները",
                           L"կարող",         L"է",      L"խմբագրել",
                           L"ցանկաց",        L"մարդ",   L"ով",
                           L"կարող",         L"է",      L"բացել",
                           L"վիքիպեդիայի",   L"կայքը"});
}

void TestUAX29URLEmailAnalyzer::testAmharic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ዊኪፔድያ የባለ ብዙ ቋንቋ የተሟላ ትክክለኛና ነጻ መዝገበ ዕውቀት (ኢንሳይክሎፒዲያ) ነው። ማንኛውም",
      std::deque<wstring>{L"ዊኪፔድያ", L"የባለ", L"ብዙ", L"ቋንቋ", L"የተሟላ", L"ትክክለኛና",
                           L"ነጻ", L"መዝገበ", L"ዕውቀት", L"ኢንሳይክሎፒዲያ", L"ነው",
                           L"ማንኛውም"});
}

void TestUAX29URLEmailAnalyzer::testArabic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"الفيلم الوثائقي الأول عن ويكيبيديا يسمى \"الحقيقة بالأرقام: قصة "
      L"ويكيبيديا\" (بالإنجليزية: Truth in Numbers: The Wikipedia Story)، سيتم "
      L"إطلاقه في 2008.",
      std::deque<wstring>{L"الفيلم", L"الوثائقي", L"الأول", L"عن",
                           L"ويكيبيديا", L"يسمى", L"الحقيقة", L"بالأرقام",
                           L"قصة", L"ويكيبيديا", L"بالإنجليزية", L"truth",
                           L"numbers", L"wikipedia", L"story", L"سيتم",
                           L"إطلاقه", L"في", L"2008"});
}

void TestUAX29URLEmailAnalyzer::testAramaic() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"ܘܝܩܝܦܕܝܐ (ܐܢܓܠܝܐ: Wikipedia) ܗܘ ܐܝܢܣܩܠܘܦܕܝܐ ܚܐܪܬܐ ܕܐܢܛܪܢܛ ܒܠܫܢ̈ܐ ܣܓܝܐ̈ܐ܂ "
      L"ܫܡܗ ܐܬܐ ܡܢ ܡ̈ܠܬܐ ܕ\"ܘܝܩܝ\" ܘ\"ܐܝܢܣܩܠܘܦܕܝܐ\"܀",
      std::deque<wstring>{L"ܘܝܩܝܦܕܝܐ", L"ܐܢܓܠܝܐ", L"wikipedia", L"ܗܘ",
                           L"ܐܝܢܣܩܠܘܦܕܝܐ", L"ܚܐܪܬܐ", L"ܕܐܢܛܪܢܛ", L"ܒܠܫܢ̈ܐ",
                           L"ܣܓܝܐ̈ܐ", L"ܫܡܗ", L"ܐܬܐ", L"ܡܢ", L"ܡ̈ܠܬܐ", L"ܕ",
                           L"ܘܝܩܝ", L"ܘ", L"ܐܝܢܣܩܠܘܦܕܝܐ"});
}

void TestUAX29URLEmailAnalyzer::testBengali() 
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

void TestUAX29URLEmailAnalyzer::testFarsi() 
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

void TestUAX29URLEmailAnalyzer::testGreek() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"Γράφεται σε συνεργασία από εθελοντές με το λογισμικό wiki, κάτι που "
      L"σημαίνει ότι άρθρα μπορεί να προστεθούν ή να αλλάξουν από τον καθένα.",
      std::deque<wstring>{L"γράφεται",   L"σε",    L"συνεργασία", L"από",
                           L"εθελοντές",  L"με",    L"το",         L"λογισμικό",
                           L"wiki",       L"κάτι",  L"που",        L"σημαίνει",
                           L"ότι",        L"άρθρα", L"μπορεί",     L"να",
                           L"προστεθούν", L"ή",     L"να",         L"αλλάξουν",
                           L"από",        L"τον",   L"καθένα"});
}

void TestUAX29URLEmailAnalyzer::testThai() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"การที่ได้ต้องแสดงว่างานดี. แล้วเธอจะไปไหน? ๑๒๓๔",
      std::deque<wstring>{L"การที่ได้ต้องแสดงว่างานดี", L"แล้วเธอจะไปไหน", L"๑๒๓๔"});
}

void TestUAX29URLEmailAnalyzer::testLao() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"ສາທາລະນະລັດ ປະຊາທິປະໄຕ ປະຊາຊົນລາວ",
      std::deque<wstring>{L"ສາທາລະນະລັດ", L"ປະຊາທິປະໄຕ", L"ປະຊາຊົນລາວ"});
}

void TestUAX29URLEmailAnalyzer::testTibetan() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །",
      std::deque<wstring>{L"སྣོན", L"མཛོད", L"དང", L"ལས",  L"འདིས", L"བོད",  L"ཡིག",
                           L"མི",  L"ཉམས", L"གོང", L"འཕེལ", L"དུ",   L"གཏོང", L"བར",
                           L"ཧ",  L"ཅང",  L"དགེ", L"མཚན", L"མཆིས", L"སོ"});
}

void TestUAX29URLEmailAnalyzer::testChinese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"我是中国人。 １２３４ Ｔｅｓｔｓ ",
      std::deque<wstring>{L"我", L"是", L"中", L"国", L"人", L"１２３４",
                           L"ｔｅｓｔｓ"});
}

void TestUAX29URLEmailAnalyzer::testEmpty() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L".", std::deque<wstring>());
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L" ", std::deque<wstring>());
}

void TestUAX29URLEmailAnalyzer::testLUCENE1545() 
{
  /*
   * Standard analyzer does not correctly tokenize combining character U+0364
   * COMBINING LATIN SMALL LETTER E. The word "moͤchte" is incorrectly tokenized
   * into "mo" "chte", the combining character is lost. Expected result is only
   * one token "moͤchte".
   */
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"moͤchte",
                                            std::deque<wstring>{L"moͤchte"});
}

void TestUAX29URLEmailAnalyzer::testAlphanumericSA() 
{
  // alphanumeric tokens
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"B2B",
                                            std::deque<wstring>{L"b2b"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"2B",
                                            std::deque<wstring>{L"2b"});
}

void TestUAX29URLEmailAnalyzer::testDelimitersSA() 
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

void TestUAX29URLEmailAnalyzer::testApostrophesSA() 
{
  // internal apostrophes: O'Reilly, you're, O'Reilly's
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"O'Reilly",
                                            std::deque<wstring>{L"o'reilly"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"you're",
                                            std::deque<wstring>{L"you're"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"she's",
                                            std::deque<wstring>{L"she's"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"Jim's",
                                            std::deque<wstring>{L"jim's"});
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"don't",
                                            std::deque<wstring>{L"don't"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"O'Reilly's", std::deque<wstring>{L"o'reilly's"});
}

void TestUAX29URLEmailAnalyzer::testNumericSA() 
{
  // floating point, serial, model numbers, ip addresses, etc.
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"21.35",
                                            std::deque<wstring>{L"21.35"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"R2D2 C3PO", std::deque<wstring>{L"r2d2", L"c3po"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"216.239.63.104", std::deque<wstring>{L"216.239.63.104"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"216.239.63.104", std::deque<wstring>{L"216.239.63.104"});
}

void TestUAX29URLEmailAnalyzer::testTextWithNumbersSA() 
{
  // numbers
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"david", L"has", L"5000", L"bones"});
}

void TestUAX29URLEmailAnalyzer::testVariousTextSA() 
{
  // various
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"C embedded developers wanted",
      std::deque<wstring>{L"c", L"embedded", L"developers", L"wanted"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"foo bar FOO BAR",
      std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"foo      bar .  FOO <> BAR",
      std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"\"QUOTED\" word", std::deque<wstring>{L"quoted", L"word"});
}

void TestUAX29URLEmailAnalyzer::testKoreanSA() 
{
  // Korean words
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"안녕하세요 한글입니다",
      std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestUAX29URLEmailAnalyzer::testOffsets() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"David has 5000 bones",
      std::deque<wstring>{L"david", L"has", L"5000", L"bones"},
      std::deque<int>{0, 6, 10, 15}, std::deque<int>{5, 9, 14, 20});
}

void TestUAX29URLEmailAnalyzer::testTypes() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"david has 5000 bones",
      std::deque<wstring>{L"david", L"has", L"5000", L"bones"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<NUM>",
                           L"<ALPHANUM>"});
}

void TestUAX29URLEmailAnalyzer::testSupplementary() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<IDEOGRAPHIC>"});
}

void TestUAX29URLEmailAnalyzer::testKorean() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"훈민정음",
                                            std::deque<wstring>{L"훈민정음"},
                                            std::deque<wstring>{L"<HANGUL>"});
}

void TestUAX29URLEmailAnalyzer::testJapanese() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"仮名遣い カタカナ",
      std::deque<wstring>{L"仮", L"名", L"遣", L"い", L"カタカナ"},
      std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>",
                           L"<HIRAGANA>", L"<KATAKANA>"});
}

void TestUAX29URLEmailAnalyzer::testCombiningMarks() 
{
  checkOneTerm(a, L"ざ", L"ざ"); // hiragana
  checkOneTerm(a, L"ザ", L"ザ"); // katakana
  checkOneTerm(a, L"壹゙", L"壹゙"); // ideographic
  checkOneTerm(a, L"아゙", L"아゙"); // hangul
}

void TestUAX29URLEmailAnalyzer::testBasicEmails() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"one test@example.com two three [A@example.CO.UK] "
      L"\"ArakaBanassaMassanaBakarA\" <info@Info.info>",
      std::deque<wstring>{L"one", L"test@example.com", L"two", L"three",
                           L"a@example.co.uk", L"arakabanassamassanabakara",
                           L"info@info.info"},
      std::deque<wstring>{L"<ALPHANUM>", L"<EMAIL>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<EMAIL>", L"<ALPHANUM>",
                           L"<EMAIL>"});
}

void TestUAX29URLEmailAnalyzer::testMailtoSchemeEmails() 
{
  // See LUCENE-3880
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"MAILTO:Test@Example.ORG",
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
      std::deque<wstring>{L"mailto", L"persona@example.com",
                           L",personb@example.com", L"?cc=personc@example.com",
                           L"subject", L"subjectivity", L"body", L"corpusivity",
                           L"20or", L"20something", L"20like", L"20that"},
      std::deque<wstring>{L"<ALPHANUM>", L"<EMAIL>", L"<EMAIL>", L"<EMAIL>",
                           L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<ALPHANUM>"});
}

void TestUAX29URLEmailAnalyzer::testBasicURLs() 
{
  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"a <HTTPs://example.net/omg/isnt/that/NICE?no=its&n%30t#mntl-E>b-D "
      L"ftp://www.example.com/ABC.txt file:///C:/path/to/a/FILE.txt C",
      std::deque<wstring>{
          L"https://example.net/omg/isnt/that/nice?no=its&n%30t#mntl-e", L"b",
          L"d", L"ftp://www.example.com/abc.txt",
          L"file:///c:/path/to/a/file.txt", L"c"},
      std::deque<wstring>{L"<URL>", L"<ALPHANUM>", L"<ALPHANUM>", L"<URL>",
                           L"<URL>", L"<ALPHANUM>"});
}

void TestUAX29URLEmailAnalyzer::testNoSchemeURLs() 
{
  // ".ph" is a Top Level Domain
  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"<index.ph>",
                                            std::deque<wstring>{L"index.ph"},
                                            std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(a, L"index.ph",
                                            std::deque<wstring>{L"index.ph"},
                                            std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"index.php", std::deque<wstring>{L"index.php"},
      std::deque<wstring>{L"<ALPHANUM>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"index.phα", std::deque<wstring>{L"index.phα"},
      std::deque<wstring>{L"<ALPHANUM>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"index-h.php", std::deque<wstring>{L"index", L"h.php"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"index2.php", std::deque<wstring>{L"index2", L"php"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"index2.ph９,", std::deque<wstring>{L"index2", L"ph９"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com,example.ph,index.php,index2.php,example2.ph",
      std::deque<wstring>{L"example.com", L"example.ph", L"index.php",
                           L"index2", L"php", L"example2.ph"},
      std::deque<wstring>{L"<URL>", L"<URL>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a,
      L"example.com:8080 example.com/path/here example.com?query=something "
      L"example.com#fragment",
      std::deque<wstring>{L"example.com:8080", L"example.com/path/here",
                           L"example.com?query=something",
                           L"example.com#fragment"},
      std::deque<wstring>{L"<URL>", L"<URL>", L"<URL>", L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080/path/here?query=something#fragment",
      std::deque<wstring>{
          L"example.com:8080/path/here?query=something#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080/path/here?query=something",
      std::deque<wstring>{L"example.com:8080/path/here?query=something"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080/path/here#fragment",
      std::deque<wstring>{L"example.com:8080/path/here#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080/path/here",
      std::deque<wstring>{L"example.com:8080/path/here"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080?query=something#fragment",
      std::deque<wstring>{L"example.com:8080?query=something#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080?query=something",
      std::deque<wstring>{L"example.com:8080?query=something"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com:8080#fragment",
      std::deque<wstring>{L"example.com:8080#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com/path/here?query=something#fragment",
      std::deque<wstring>{L"example.com/path/here?query=something#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com/path/here?query=something",
      std::deque<wstring>{L"example.com/path/here?query=something"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com/path/here#fragment",
      std::deque<wstring>{L"example.com/path/here#fragment"},
      std::deque<wstring>{L"<URL>"});

  BaseTokenStreamTestCase::assertAnalyzesTo(
      a, L"example.com?query=something#fragment",
      std::deque<wstring>{L"example.com?query=something#fragment"},
      std::deque<wstring>{L"<URL>"});
}

void TestUAX29URLEmailAnalyzer::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestUAX29URLEmailAnalyzer::testMaxTokenLengthDefault() 
{

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

void TestUAX29URLEmailAnalyzer::testMaxTokenLengthNonDefault() throw(
    runtime_error)
{
  shared_ptr<UAX29URLEmailAnalyzer> a = make_shared<UAX29URLEmailAnalyzer>();
  a->setMaxTokenLength(5);
  assertAnalyzesTo(
      a, L"ab cd toolong xy z",
      std::deque<wstring>{L"ab", L"cd", L"toolo", L"ng", L"xy", L"z"});
  delete a;
}
} // namespace org::apache::lucene::analysis::standard