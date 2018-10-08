using namespace std;

#include "TestConcatenatingTokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ConcatenatingTokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/EmptyTokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

void TestConcatenatingTokenStream::testBasic() 
{

  shared_ptr<AttributeFactory> factory = newAttributeFactory();

  shared_ptr<MockTokenizer> *const first =
      make_shared<MockTokenizer>(factory, MockTokenizer::WHITESPACE, false);
  first->setReader(make_shared<StringReader>(L"first words "));
  shared_ptr<MockTokenizer> *const second =
      make_shared<MockTokenizer>(factory, MockTokenizer::WHITESPACE, false);
  second->setReader(make_shared<StringReader>(L"second words"));
  shared_ptr<MockTokenizer> *const third =
      make_shared<MockTokenizer>(factory, MockTokenizer::WHITESPACE, false);
  third->setReader(make_shared<StringReader>(L" third words"));

  shared_ptr<TokenStream> ts = make_shared<ConcatenatingTokenStream>(
      first, second, make_shared<EmptyTokenStream>(), third);
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"first", L"words", L"second",
                                                 L"words", L"third", L"words"},
                            std::deque<int>{0, 6, 12, 19, 25, 31},
                            std::deque<int>{5, 11, 18, 24, 30, 36});
}

void TestConcatenatingTokenStream::testInconsistentAttributes() throw(
    IOException)
{

  shared_ptr<AttributeFactory> factory = newAttributeFactory();

  shared_ptr<MockTokenizer> *const first =
      make_shared<MockTokenizer>(factory, MockTokenizer::WHITESPACE, false);
  first->setReader(make_shared<StringReader>(L"first words "));
  first->addAttribute(PayloadAttribute::typeid);
  shared_ptr<MockTokenizer> *const second =
      make_shared<MockTokenizer>(factory, MockTokenizer::WHITESPACE, false);
  second->setReader(make_shared<StringReader>(L"second words"));
  second->addAttribute(FlagsAttribute::typeid);

  shared_ptr<TokenStream> ts =
      make_shared<ConcatenatingTokenStream>(first, second);
  assertTrue(ts->hasAttribute(FlagsAttribute::typeid));
  assertTrue(ts->hasAttribute(PayloadAttribute::typeid));

  assertTokenStreamContents(
      ts, std::deque<wstring>{L"first", L"words", L"second", L"words"},
      std::deque<int>{0, 6, 12, 19}, std::deque<int>{5, 11, 18, 24});
}

void TestConcatenatingTokenStream::testInconsistentAttributeFactories() throw(
    IOException)
{

  shared_ptr<MockTokenizer> *const first =
      make_shared<MockTokenizer>(AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY,
                                 MockTokenizer::WHITESPACE, true);
  shared_ptr<MockTokenizer> *const second =
      make_shared<MockTokenizer>(TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                 MockTokenizer::WHITESPACE, true);

  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<ConcatenatingTokenStream>(first, second); });
}
} // namespace org::apache::lucene::analysis::miscellaneous