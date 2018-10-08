using namespace std;

#include "TestICUNormalizer2CharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUNormalizer2CharFilterFactory.h"

namespace org::apache::lucene::analysis::icu
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestICUNormalizer2CharFilterFactory::testDefaults() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"This is a Ｔｅｓｔ");
  shared_ptr<ICUNormalizer2CharFilterFactory> factory =
      make_shared<ICUNormalizer2CharFilterFactory>(
          unordered_map<wstring, wstring>());
  reader = factory->create(reader);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this", L"is", L"a", L"test"});
}

void TestICUNormalizer2CharFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ICUNormalizer2CharFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestICUNormalizer2CharFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestICUNormalizer2CharFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::icu