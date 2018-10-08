using namespace std;

#include "TestUAX29URLEmailTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestUAX29URLEmailTokenizerFactory::testUAX29URLEmailTokenizer() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Wha\u0301t's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Wha\u0301t's", L"this", L"thing", L"do"});
}

void TestUAX29URLEmailTokenizerFactory::testArabic() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(
      L"الفيلم الوثائقي الأول عن ويكيبيديا يسمى \"الحقيقة بالأرقام: قصة "
      L"ويكيبيديا\" (بالإنجليزية: Truth in Numbers: The Wikipedia Story)، سيتم "
      L"إطلاقه في 2008.");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"الفيلم",    L"الوثائقي",    L"الأول",
                                   L"عن",        L"ويكيبيديا",   L"يسمى",
                                   L"الحقيقة",   L"بالأرقام",    L"قصة",
                                   L"ويكيبيديا", L"بالإنجليزية", L"Truth",
                                   L"in",        L"Numbers",     L"The",
                                   L"Wikipedia", L"Story",       L"سيتم",
                                   L"إطلاقه",    L"في",          L"2008"});
}

void TestUAX29URLEmailTokenizerFactory::testChinese() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"我是中国人。 １２３４ Ｔｅｓｔｓ ");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"我", L"是", L"中", L"国", L"人",
                                   L"１２３４", L"Ｔｅｓｔｓ"});
}

void TestUAX29URLEmailTokenizerFactory::testKorean() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"안녕하세요 한글입니다");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestUAX29URLEmailTokenizerFactory::testHyphen() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"some-dashed-phrase");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"some", L"dashed", L"phrase"});
}

void TestUAX29URLEmailTokenizerFactory::testURLs() 
{
  wstring textWithURLs =
      wstring(L"http://johno.jsmf.net/knowhow/ngrams/"
              L"index.php?table=en-dickens-word-2gram&paragraphs=50&length=200&"
              L"no-ads=on\n") +
      L" some extra\nWords thrown in here. " +
      L"http://c5-3486.bisynxu.FR/aI.YnNms/" + L" samba Halta gamba " +
      L"ftp://119.220.152.185/JgJgdZ/31aW5c/viWlfQSTs5/1c8U5T/ih5rXx/YfUJ/"
      L"xBW1uHrQo6.R\n" +
      L"M19nq.0URV4A.Me.CC/mj0kgt6hue/dRXv8YVLOw9v/CIOqb\n" +
      L"Https://yu7v33rbt.vC6U3.XN--KPRW13D/y%4fMSzkGFlm/wbDF4m" +
      L" inter Locutio " + L"[c2d4::]/%471j5l/j3KFN%AAAn/Fip-NisKH/\n" +
      L"file:///aXvSZS34is/eIgM8s~U5dU4Ifd%c7" + L" blah Sirrah woof " +
      L"http://[a42:a7b6::]/qSmxSUU4z/%52qVl4\n";
  shared_ptr<Reader> reader = make_shared<StringReader>(textWithURLs);
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{
                  L"http://johno.jsmf.net/knowhow/ngrams/"
                  L"index.php?table=en-dickens-word-2gram&paragraphs=50&length="
                  L"200&no-ads=on",
                  L"some",
                  L"extra",
                  L"Words",
                  L"thrown",
                  L"in",
                  L"here",
                  L"http://c5-3486.bisynxu.FR/aI.YnNms/",
                  L"samba",
                  L"Halta",
                  L"gamba",
                  L"ftp://119.220.152.185/JgJgdZ/31aW5c/viWlfQSTs5/1c8U5T/"
                  L"ih5rXx/YfUJ/xBW1uHrQo6.R",
                  L"M19nq.0URV4A.Me.CC/mj0kgt6hue/dRXv8YVLOw9v/CIOqb",
                  L"Https://yu7v33rbt.vC6U3.XN--KPRW13D/y%4fMSzkGFlm/wbDF4m",
                  L"inter",
                  L"Locutio",
                  L"[c2d4::]/%471j5l/j3KFN%AAAn/Fip-NisKH/",
                  L"file:///aXvSZS34is/eIgM8s~U5dU4Ifd%c7",
                  L"blah",
                  L"Sirrah",
                  L"woof",
                  L"http://[a42:a7b6::]/qSmxSUU4z/%52qVl4"});
}

void TestUAX29URLEmailTokenizerFactory::testEmails() 
{
  wstring textWithEmails =
      wstring(L" some extra\nWords thrown in here. ") + L"dJ8ngFi@avz13m.CC\n" +
      L"kU-l6DS@[082.015.228.189]\n" + L"\"%U\u0012@?\\B\"@Fl2d.md" +
      L" samba Halta gamba " + L"Bvd#@tupjv.sn\n" +
      L"SBMm0Nm.oyk70.rMNdd8k.#ru3LI.gMMLBI.0dZRD4d.RVK2nY@au58t.B13albgy4u."
      L"mt\n" +
      L"~+Kdz@3mousnl.SE\n" + L" inter Locutio " +
      L"C'ts`@Vh4zk.uoafcft-dr753x4odt04q.UY\n" +
      L"}0tzWYDBuy@cSRQAABB9B.7c8xawf75-cyo.PM" + L" blah Sirrah woof " +
      L"lMahAA.j/5.RqUjS745.DtkcYdi@d2-4gb-l6.ae\n" +
      L"lv'p@tqk.vj5s0tgl.0dlu7su3iyiaz.dqso.494.3hb76.XN--MGBAAM7A8H\n";
  shared_ptr<Reader> reader = make_shared<StringReader>(textWithEmails);
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{
          L"some",
          L"extra",
          L"Words",
          L"thrown",
          L"in",
          L"here",
          L"dJ8ngFi@avz13m.CC",
          L"kU-l6DS@[082.015.228.189]",
          L"\"%U\u0012@?\\B\"@Fl2d.md",
          L"samba",
          L"Halta",
          L"gamba",
          L"Bvd#@tupjv.sn",
          L"SBMm0Nm.oyk70.rMNdd8k.#ru3LI.gMMLBI.0dZRD4d.RVK2nY@au58t."
          L"B13albgy4u.mt",
          L"~+Kdz@3mousnl.SE",
          L"inter",
          L"Locutio",
          L"C'ts`@Vh4zk.uoafcft-dr753x4odt04q.UY",
          L"}0tzWYDBuy@cSRQAABB9B.7c8xawf75-cyo.PM",
          L"blah",
          L"Sirrah",
          L"woof",
          L"lMahAA.j/5.RqUjS745.DtkcYdi@d2-4gb-l6.ae",
          L"lv'p@tqk.vj5s0tgl.0dlu7su3iyiaz.dqso.494.3hb76.XN--MGBAAM7A8H"});
}

void TestUAX29URLEmailTokenizerFactory::testMaxTokenLength() throw(
    runtime_error)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int i = 0; i < 100; ++i) {
    builder->append(L"abcdefg"); // 7 * 100 = 700 char "word"
  }
  wstring longWord = builder->toString();
  wstring content = L"one two three " + longWord + L" four five six";
  shared_ptr<Reader> reader = make_shared<StringReader>(content);
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"UAX29URLEmail", {L"maxTokenLength", L"1000"})
          ->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"one", L"two", L"three", longWord, L"four",
                                   L"five", L"six"});
}

void TestUAX29URLEmailTokenizerFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"UAX29URLEmail", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestUAX29URLEmailTokenizerFactory::testIllegalArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"UAX29URLEmail", {L"maxTokenLength", L"-1"})->create();
  });
  assertTrue(
      expected.what()->contains(L"maxTokenLength must be greater than zero"));
}
} // namespace org::apache::lucene::analysis::standard