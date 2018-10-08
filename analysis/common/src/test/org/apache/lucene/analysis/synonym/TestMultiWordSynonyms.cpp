using namespace std;

#include "TestMultiWordSynonyms.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../util/StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::synonym
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using Version = org::apache::lucene::util::Version;

void TestMultiWordSynonyms::testMultiWordSynonyms() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"a e");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Synonym", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L"a b c,d"),
                              L"synonyms", L"synonyms.txt")
               .create(stream);
  // This fails because ["e","e"] is the value of the token stream
  assertTokenStreamContents(stream, std::deque<wstring>{L"a", L"e"});
}
} // namespace org::apache::lucene::analysis::synonym