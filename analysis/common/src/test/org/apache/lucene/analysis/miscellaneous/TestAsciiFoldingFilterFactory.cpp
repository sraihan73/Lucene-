using namespace std;

#include "TestAsciiFoldingFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

void TestAsciiFoldingFilterFactory::testMultiTermAnalysis() 
{
  shared_ptr<TokenFilterFactory> factory =
      make_shared<ASCIIFoldingFilterFactory>(Collections::emptyMap());
  shared_ptr<TokenStream> stream =
      make_shared<CannedTokenStream>(make_shared<Token>(L"Été", 0, 3));
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Ete"});

  factory = std::static_pointer_cast<TokenFilterFactory>(
      (std::static_pointer_cast<MultiTermAwareComponent>(factory))
          ->getMultiTermComponent());
  stream = make_shared<CannedTokenStream>(make_shared<Token>(L"Été", 0, 3));
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Ete"});

  factory = make_shared<ASCIIFoldingFilterFactory>(
      unordered_map<>(Collections::singletonMap(L"preserveOriginal", L"true")));
  stream = make_shared<CannedTokenStream>(make_shared<Token>(L"Été", 0, 3));
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Ete", L"Été"});

  factory = std::static_pointer_cast<TokenFilterFactory>(
      (std::static_pointer_cast<MultiTermAwareComponent>(factory))
          ->getMultiTermComponent());
  stream = make_shared<CannedTokenStream>(make_shared<Token>(L"Été", 0, 3));
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Ete"});
}
} // namespace org::apache::lucene::analysis::miscellaneous