using namespace std;

#include "TestStemmerOverrideFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../util/StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using Version = org::apache::lucene::util::Version;

void TestStemmerOverrideFilterFactory::testKeywords() 
{
  // our stemdict stems dogs to 'cat'
  shared_ptr<Reader> reader = make_shared<StringReader>(L"testing dogs");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"StemmerOverride", Version::LATEST,
                         make_shared<StringMockResourceLoader>(L"dogs\tcat"),
                         L"dictionary", L"stemdict.txt")
          .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"test", L"cat"});
}

void TestStemmerOverrideFilterFactory::testKeywordsCaseInsensitive() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"testing DoGs");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"StemmerOverride", Version::LATEST,
                         make_shared<StringMockResourceLoader>(L"dogs\tcat"),
                         L"dictionary", L"stemdict.txt", L"ignoreCase", L"true")
          .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"test", L"cat"});
}

void TestStemmerOverrideFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"StemmerOverride", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous