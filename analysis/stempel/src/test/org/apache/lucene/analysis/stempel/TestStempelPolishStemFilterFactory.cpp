using namespace std;

#include "TestStempelPolishStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/stempel/StempelPolishStemFilterFactory.h"

namespace org::apache::lucene::analysis::stempel
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestStempelPolishStemFilterFactory::testBasics() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"studenta studenci");
  shared_ptr<StempelPolishStemFilterFactory> factory =
      make_shared<StempelPolishStemFilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"student", L"student"});
}

void TestStempelPolishStemFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<StempelPolishStemFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestStempelPolishStemFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestStempelPolishStemFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::stempel