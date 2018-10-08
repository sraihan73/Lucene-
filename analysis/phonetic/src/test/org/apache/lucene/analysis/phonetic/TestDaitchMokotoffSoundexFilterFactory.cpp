using namespace std;

#include "TestDaitchMokotoffSoundexFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DaitchMokotoffSoundexFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DaitchMokotoffSoundexFilterFactory.h"

namespace org::apache::lucene::analysis::phonetic
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestDaitchMokotoffSoundexFilterFactory::testDefaults() 
{
  shared_ptr<DaitchMokotoffSoundexFilterFactory> factory =
      make_shared<DaitchMokotoffSoundexFilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<Tokenizer> inputStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  inputStream->setReader(make_shared<StringReader>(L"international"));

  shared_ptr<TokenStream> filteredStream = factory->create(inputStream);
  assertEquals(DaitchMokotoffSoundexFilter::typeid, filteredStream->getClass());
  assertTokenStreamContents(filteredStream,
                            std::deque<wstring>{L"international", L"063963"});
}

void TestDaitchMokotoffSoundexFilterFactory::testSettingInject() throw(
    runtime_error)
{
  unordered_map<wstring, wstring> parameters =
      unordered_map<wstring, wstring>();
  parameters.emplace(L"inject", L"false");
  shared_ptr<DaitchMokotoffSoundexFilterFactory> factory =
      make_shared<DaitchMokotoffSoundexFilterFactory>(parameters);

  shared_ptr<Tokenizer> inputStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  inputStream->setReader(make_shared<StringReader>(L"international"));

  shared_ptr<TokenStream> filteredStream = factory->create(inputStream);
  assertEquals(DaitchMokotoffSoundexFilter::typeid, filteredStream->getClass());
  assertTokenStreamContents(filteredStream, std::deque<wstring>{L"063963"});
}

void TestDaitchMokotoffSoundexFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<DaitchMokotoffSoundexFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestDaitchMokotoffSoundexFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestDaitchMokotoffSoundexFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::phonetic