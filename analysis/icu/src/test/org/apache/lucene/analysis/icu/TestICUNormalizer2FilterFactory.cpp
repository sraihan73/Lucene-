using namespace std;

#include "TestICUNormalizer2FilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUNormalizer2FilterFactory.h"

namespace org::apache::lucene::analysis::icu
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestICUNormalizer2FilterFactory::testDefaults() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"This is a Ｔｅｓｔ");
  shared_ptr<ICUNormalizer2FilterFactory> factory =
      make_shared<ICUNormalizer2FilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this", L"is", L"a", L"test"});
}

void TestICUNormalizer2FilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ICUNormalizer2FilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestICUNormalizer2FilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestICUNormalizer2FilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::icu