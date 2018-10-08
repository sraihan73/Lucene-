using namespace std;

#include "TestICUTokenizerFactory.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizerFactory.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;

void TestICUTokenizerFactory::testMixedText() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"การที่ได้ต้องแสดงว่างานดี  This is a test ກວ່າດອກ");
  shared_ptr<ICUTokenizerFactory> factory =
      make_shared<ICUTokenizerFactory>(unordered_map<wstring, wstring>());
  factory->inform(make_shared<ClasspathResourceLoader>(getClass()));
  shared_ptr<Tokenizer> stream = factory->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง", L"ว่า",
                                   L"งาน", L"ดี", L"This", L"is", L"a", L"test",
                                   L"ກວ່າ", L"ດອກ"});
}

void TestICUTokenizerFactory::testTokenizeLatinOnWhitespaceOnly() throw(
    runtime_error)
{
  // " U+201C LEFT DOUBLE QUOTATION MARK; " U+201D RIGHT DOUBLE QUOTATION MARK
  shared_ptr<Reader> reader = make_shared<StringReader>(
      L"  Don't,break.at?/(punct)!  \u201Cnice\u201D\r\n\r\n85_At:all; "
      L"`really\" +2=3$5,&813 !@#%$^)(*@#$   ");
  const unordered_map<wstring, wstring> args =
      unordered_map<wstring, wstring>();
  args.emplace(ICUTokenizerFactory::RULEFILES,
               L"Latn:Latin-break-only-on-whitespace.rbbi");
  shared_ptr<ICUTokenizerFactory> factory =
      make_shared<ICUTokenizerFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(this->getClass()));
  shared_ptr<Tokenizer> stream = factory->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Don't,break.at?/(punct)!", L"\u201Cnice\u201D",
                           L"85_At:all;", L"`really\"", L"+2=3$5,&813",
                           L"!@#%$^)(*@#$"},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<NUM>", L"<OTHER>"});
}

void TestICUTokenizerFactory::testTokenizeLatinDontBreakOnHyphens() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"One-two punch.  Brang-, not brung-it.  This "
                                L"one--not that one--is the right one, -ish.");
  const unordered_map<wstring, wstring> args =
      unordered_map<wstring, wstring>();
  args.emplace(ICUTokenizerFactory::RULEFILES,
               L"Latn:Latin-dont-break-on-hyphens.rbbi");
  shared_ptr<ICUTokenizerFactory> factory =
      make_shared<ICUTokenizerFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(getClass()));
  shared_ptr<Tokenizer> stream = factory->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"One-two", L"punch", L"Brang", L"not", L"brung-it",
                           L"This", L"one", L"not", L"that", L"one", L"is",
                           L"the", L"right", L"one", L"ish"});
}

void TestICUTokenizerFactory::testKeywordTokenizeCyrillicAndThai() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Some English.  Немного русский.  "
                                L"ข้อความภาษาไทยเล็ก ๆ น้อย ๆ  More English.");
  const unordered_map<wstring, wstring> args =
      unordered_map<wstring, wstring>();
  args.emplace(ICUTokenizerFactory::RULEFILES,
               L"Cyrl:KeywordTokenizer.rbbi,Thai:KeywordTokenizer.rbbi");
  shared_ptr<ICUTokenizerFactory> factory =
      make_shared<ICUTokenizerFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(getClass()));
  shared_ptr<Tokenizer> stream = factory->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Some", L"English", L"Немного русский.  ",
                           L"ข้อความภาษาไทยเล็ก ๆ น้อย ๆ  ", L"More", L"English"});
}

void TestICUTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ICUTokenizerFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestICUTokenizerFactory::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestICUTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::icu::segmentation