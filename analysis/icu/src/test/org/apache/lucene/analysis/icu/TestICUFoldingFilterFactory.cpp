using namespace std;

#include "TestICUFoldingFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUFoldingFilterFactory.h"

namespace org::apache::lucene::analysis::icu
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestICUFoldingFilterFactory::test() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"Résumé");
  shared_ptr<ICUFoldingFilterFactory> factory =
      make_shared<ICUFoldingFilterFactory>(unordered_map<wstring, wstring>());
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"resume"});
}

void TestICUFoldingFilterFactory::testFilter() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"filter", L"[^ö]");
  shared_ptr<ICUFoldingFilterFactory> factory =
      make_shared<ICUFoldingFilterFactory>(args);

  shared_ptr<Reader> reader = make_shared<StringReader>(L"Résumé");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"resume"});

  reader = make_shared<StringReader>(L"Fönster");
  stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"fönster"});
}

void TestICUFoldingFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ICUFoldingFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestICUFoldingFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestICUFoldingFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::icu