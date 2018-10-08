using namespace std;

#include "TestSnowballPorterFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/tartarus/snowball/ext/EnglishStemmer.h"
#include "../util/StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::snowball
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using Version = org::apache::lucene::util::Version;
using EnglishStemmer = org::tartarus::snowball::ext::EnglishStemmer;

void TestSnowballPorterFilterFactory::test() 
{
  wstring text = L"The fledgling banks were counting on a big boom in banking";
  shared_ptr<EnglishStemmer> stemmer = make_shared<EnglishStemmer>();
  std::deque<wstring> test = text.split(L"\\s");
  std::deque<wstring> gold(test.size());
  for (int i = 0; i < test.size(); i++) {
    stemmer->setCurrent(test[i]);
    stemmer->stem();
    gold[i] = stemmer->getCurrent();
  }

  shared_ptr<Reader> reader = make_shared<StringReader>(text);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"SnowballPorter", {L"language", L"English"})
               ->create(stream);
  assertTokenStreamContents(stream, gold);
}

void TestSnowballPorterFilterFactory::testProtected() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"ridding of some stemming");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"SnowballPorter", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L"ridding"),
                              L"protected", L"protwords.txt", L"language",
                              L"English")
               .create(stream);

  assertTokenStreamContents(
      stream, std::deque<wstring>{L"ridding", L"of", L"some", L"stem"});
}

void TestSnowballPorterFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"SnowballPorter", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::snowball