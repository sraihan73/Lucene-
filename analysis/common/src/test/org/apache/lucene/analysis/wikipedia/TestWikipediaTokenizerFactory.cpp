using namespace std;

#include "TestWikipediaTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/wikipedia/WikipediaTokenizer.h"

namespace org::apache::lucene::analysis::wikipedia
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestWikipediaTokenizerFactory::testTokenizer() 
{
  wstring text = L"This is a [[Category:foo]]";
  shared_ptr<Tokenizer> tf =
      tokenizerFactory(WIKIPEDIA).create(newAttributeFactory());
  tf->setReader(make_shared<StringReader>(text));
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"This", L"is", L"a", L"foo"},
      std::deque<int>{0, 5, 8, 21}, std::deque<int>{4, 7, 9, 24},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY},
      std::deque<int>{1, 1, 1, 1}, text.length());
}

void TestWikipediaTokenizerFactory::testTokenizerTokensOnly() throw(
    runtime_error)
{
  wstring text = L"This is a [[Category:foo]]";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Tokenizer> tf =
      tokenizerFactory(
          WIKIPEDIA,
          {TOKEN_OUTPUT,
           (optional<int>(WikipediaTokenizer::TOKENS_ONLY))->toString()})
          ->create(newAttributeFactory());
  tf->setReader(make_shared<StringReader>(text));
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"This", L"is", L"a", L"foo"},
      std::deque<int>{0, 5, 8, 21}, std::deque<int>{4, 7, 9, 24},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY},
      std::deque<int>{1, 1, 1, 1}, text.length());
}

void TestWikipediaTokenizerFactory::testTokenizerUntokenizedOnly() throw(
    runtime_error)
{
  wstring test =
      L"[[Category:a b c d]] [[Category:e f g]] [[link here]] [[link there]] "
      L"''italics here'' something ''more italics'' [[Category:h   i   j]]";
  shared_ptr<Set<wstring>> untoks = unordered_set<wstring>();
  untoks->add(WikipediaTokenizer::CATEGORY);
  untoks->add(WikipediaTokenizer::ITALICS);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Tokenizer> tf =
      tokenizerFactory(
          WIKIPEDIA,
          {TOKEN_OUTPUT,
           (optional<int>(WikipediaTokenizer::UNTOKENIZED_ONLY))->toString(),
           UNTOKENIZED_TYPES,
           WikipediaTokenizer::CATEGORY + L", " + WikipediaTokenizer::ITALICS})
          ->create(newAttributeFactory());
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"a b c d", L"e f g", L"link", L"here", L"link",
                           L"there", L"italics here", L"something",
                           L"more italics", L"h   i   j"},
      std::deque<int>{11, 32, 42, 47, 56, 61, 71, 86, 98, 124},
      std::deque<int>{18, 37, 46, 51, 60, 66, 83, 95, 110, 133},
      std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWikipediaTokenizerFactory::testTokenizerBoth() 
{
  wstring test =
      L"[[Category:a b c d]] [[Category:e f g]] [[link here]] [[link there]] "
      L"''italics here'' something ''more italics'' [[Category:h   i   j]]";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Tokenizer> tf =
      tokenizerFactory(
          WIKIPEDIA,
          {TOKEN_OUTPUT, (optional<int>(WikipediaTokenizer::BOTH))->toString(),
           UNTOKENIZED_TYPES,
           WikipediaTokenizer::CATEGORY + L", " + WikipediaTokenizer::ITALICS})
          ->create(newAttributeFactory());
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"a b c d",   L"a",
                               L"b",         L"c",
                               L"d",         L"e f g",
                               L"e",         L"f",
                               L"g",         L"link",
                               L"here",      L"link",
                               L"there",     L"italics here",
                               L"italics",   L"here",
                               L"something", L"more italics",
                               L"more",      L"italics",
                               L"h   i   j", L"h",
                               L"i",         L"j"},
      std::deque<int>{11, 11, 13, 15, 17, 32, 32, 34,  36,  42,  47,  56,
                       61, 71, 71, 79, 86, 98, 98, 103, 124, 124, 128, 132},
      std::deque<int>{18, 12, 14, 16, 18, 37,  33,  35,  37,  46,  51,  60,
                       66, 83, 78, 83, 95, 110, 102, 110, 133, 125, 129, 133},
      std::deque<int>{1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                       1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1});
}

void TestWikipediaTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(WIKIPEDIA, {L"bogusArg", L"bogusValue"})
        ->create(newAttributeFactory());
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestWikipediaTokenizerFactory::testIllegalArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<Tokenizer> tf =
        tokenizerFactory(WIKIPEDIA, {TOKEN_OUTPUT, L"-1"})
            ->create(newAttributeFactory());
  });
  assertTrue(expected.what()->contains(
      L"tokenOutput must be TOKENS_ONLY, UNTOKENIZED_ONLY or BOTH"));
}
} // namespace org::apache::lucene::analysis::wikipedia