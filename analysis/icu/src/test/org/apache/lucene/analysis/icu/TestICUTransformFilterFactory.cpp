using namespace std;

#include "TestICUTransformFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUTransformFilterFactory.h"

namespace org::apache::lucene::analysis::icu
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestICUTransformFilterFactory::test() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"簡化字");
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"id", L"Traditional-Simplified");
  shared_ptr<ICUTransformFilterFactory> factory =
      make_shared<ICUTransformFilterFactory>(args);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"简化字"});
}

void TestICUTransformFilterFactory::testForwardDirection() 
{
  // forward
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Российская Федерация");
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"id", L"Cyrillic-Latin");
  shared_ptr<ICUTransformFilterFactory> factory =
      make_shared<ICUTransformFilterFactory>(args);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Rossijskaâ", L"Federaciâ"});
}

void TestICUTransformFilterFactory::testReverseDirection() 
{
  // backward (invokes Latin-Cyrillic)
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Rossijskaâ Federaciâ");
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"id", L"Cyrillic-Latin");
  args.emplace(L"direction", L"reverse");
  shared_ptr<ICUTransformFilterFactory> factory =
      make_shared<ICUTransformFilterFactory>(args);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Российская", L"Федерация"});
}

void TestICUTransformFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ICUTransformFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestICUTransformFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestICUTransformFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"id", L"Null");
  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::icu