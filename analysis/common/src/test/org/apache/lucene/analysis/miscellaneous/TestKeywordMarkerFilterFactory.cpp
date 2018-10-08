using namespace std;

#include "TestKeywordMarkerFilterFactory.h"
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

void TestKeywordMarkerFilterFactory::testKeywords() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dogs cats");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"KeywordMarker", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L"cats"),
                              L"protected", L"protwords.txt")
               .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"dog", L"cats"});
}

void TestKeywordMarkerFilterFactory::testKeywords2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dogs cats");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"KeywordMarker", {L"pattern", L"cats|Dogs"})
               ->create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"dog", L"cats"});
}

void TestKeywordMarkerFilterFactory::testKeywordsMixed() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dogs cats birds");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"KeywordMarker", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L"cats"),
                              L"protected", L"protwords.txt", L"pattern",
                              L"birds|Dogs")
               .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"dog", L"cats", L"birds"});
}

void TestKeywordMarkerFilterFactory::testKeywordsCaseInsensitive() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dogs cats Cats");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"KeywordMarker", Version::LATEST,
                         make_shared<StringMockResourceLoader>(L"cats"),
                         L"protected", L"protwords.txt", L"ignoreCase", L"true")
          .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"dog", L"cats", L"Cats"});
}

void TestKeywordMarkerFilterFactory::testKeywordsCaseInsensitive2() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dogs cats Cats");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"KeywordMarker",
                              {L"pattern", L"Cats", L"ignoreCase", L"true"})
               ->create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"dog", L"cats", L"Cats"});
}

void TestKeywordMarkerFilterFactory::testKeywordsCaseInsensitiveMixed() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"dogs cats Cats Birds birds");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"KeywordMarker", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L"cats"),
                              L"protected", L"protwords.txt", L"pattern",
                              L"birds", L"ignoreCase", L"true")
               .create(stream);
  stream = tokenFilterFactory(L"PorterStem").create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"dog", L"cats", L"Cats", L"Birds", L"birds"});
}

void TestKeywordMarkerFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"KeywordMarker", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous