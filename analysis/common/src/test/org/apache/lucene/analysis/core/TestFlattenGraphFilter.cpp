using namespace std;

#include "TestFlattenGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/FlattenGraphFilter.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

shared_ptr<Token> TestFlattenGraphFilter::token(const wstring &term, int posInc,
                                                int posLength, int startOffset,
                                                int endOffset)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, startOffset, endOffset);
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

void TestFlattenGraphFilter::testSimpleMock() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(a, L"wtf happened",
                   std::deque<wstring>{L"wtf", L"happened"},
                   std::deque<int>{0, 4}, std::deque<int>{3, 12}, nullptr,
                   std::deque<int>{1, 1}, std::deque<int>{1, 1}, true);
}

TestFlattenGraphFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestFlattenGraphFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFlattenGraphFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  shared_ptr<TokenStream> ts = make_shared<FlattenGraphFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, ts);
}

void TestFlattenGraphFilter::testAlreadyFlatten() 
{
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 12,
      std::deque<std::shared_ptr<Token>>{
          token(L"wtf", 1, 1, 0, 3), token(L"what", 0, 1, 0, 3),
          token(L"wow", 0, 1, 0, 3), token(L"the", 1, 1, 0, 3),
          token(L"that's", 0, 1, 0, 3), token(L"fudge", 1, 1, 0, 3),
          token(L"funny", 0, 1, 0, 3), token(L"happened", 1, 1, 4, 12)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(out,
                            std::deque<wstring>{L"wtf", L"what", L"wow",
                                                 L"the", L"that's", L"fudge",
                                                 L"funny", L"happened"},
                            std::deque<int>{0, 0, 0, 0, 0, 0, 0, 4},
                            std::deque<int>{3, 3, 3, 3, 3, 3, 3, 12},
                            std::deque<int>{1, 0, 0, 1, 0, 1, 0, 1},
                            std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1}, 12);
}

void TestFlattenGraphFilter::testWTF1() 
{

  // "wow that's funny" and "what the fudge" are separate side paths, in
  // parallel with "wtf", on input:
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 12,
      std::deque<std::shared_ptr<Token>>{
          token(L"wtf", 1, 5, 0, 3), token(L"what", 0, 1, 0, 3),
          token(L"wow", 0, 3, 0, 3), token(L"the", 1, 1, 0, 3),
          token(L"fudge", 1, 3, 0, 3), token(L"that's", 1, 1, 0, 3),
          token(L"funny", 1, 1, 0, 3), token(L"happened", 1, 1, 4, 12)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(out,
                            std::deque<wstring>{L"wtf", L"what", L"wow",
                                                 L"the", L"that's", L"fudge",
                                                 L"funny", L"happened"},
                            std::deque<int>{0, 0, 0, 0, 0, 0, 0, 4},
                            std::deque<int>{3, 3, 3, 3, 3, 3, 3, 12},
                            std::deque<int>{1, 0, 0, 1, 0, 1, 0, 1},
                            std::deque<int>{3, 1, 1, 1, 1, 1, 1, 1}, 12);
}

void TestFlattenGraphFilter::testWTF2() 
{

  // "wow that's funny" and "what the fudge" are separate side paths, in
  // parallel with "wtf", on input:
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 12,
      std::deque<std::shared_ptr<Token>>{
          token(L"what", 1, 1, 0, 3), token(L"wow", 0, 3, 0, 3),
          token(L"wtf", 0, 5, 0, 3), token(L"the", 1, 1, 0, 3),
          token(L"fudge", 1, 3, 0, 3), token(L"that's", 1, 1, 0, 3),
          token(L"funny", 1, 1, 0, 3), token(L"happened", 1, 1, 4, 12)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(out,
                            std::deque<wstring>{L"what", L"wow", L"wtf",
                                                 L"the", L"that's", L"fudge",
                                                 L"funny", L"happened"},
                            std::deque<int>{0, 0, 0, 0, 0, 0, 0, 4},
                            std::deque<int>{3, 3, 3, 3, 3, 3, 3, 12},
                            std::deque<int>{1, 0, 0, 1, 0, 1, 0, 1},
                            std::deque<int>{1, 1, 3, 1, 1, 1, 1, 1}, 12);
}

void TestFlattenGraphFilter::testNonGreedySynonyms() 
{
  // This is just "hypothetical" for Lucene today, because SynFilter is
  // greedy: when two syn rules match on overlapping tokens, only one
  // (greedily) wins.  This test pretends all syn matches could match:

  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 20,
      std::deque<std::shared_ptr<Token>>{
          token(L"wizard", 1, 1, 0, 6), token(L"wizard_of_oz", 0, 3, 0, 12),
          token(L"of", 1, 1, 7, 9), token(L"oz", 1, 1, 10, 12),
          token(L"oz_screams", 0, 2, 10, 20), token(L"screams", 1, 1, 13, 20)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(out,
                            std::deque<wstring>{L"wizard", L"wizard_of_oz",
                                                 L"of", L"oz", L"oz_screams",
                                                 L"screams"},
                            std::deque<int>{0, 0, 7, 10, 10, 13},
                            std::deque<int>{6, 12, 9, 12, 20, 20},
                            std::deque<int>{1, 0, 1, 1, 0, 1},
                            std::deque<int>{1, 3, 1, 1, 2, 1}, 20);
}

void TestFlattenGraphFilter::testNonGraph() 
{
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 22,
      std::deque<std::shared_ptr<Token>>{
          token(L"hello", 1, 1, 0, 5), token(L"pseudo", 1, 1, 6, 12),
          token(L"world", 1, 1, 13, 18), token(L"fun", 1, 1, 19, 22)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(
      out, std::deque<wstring>{L"hello", L"pseudo", L"world", L"fun"},
      std::deque<int>{0, 6, 13, 19}, std::deque<int>{5, 12, 18, 22},
      std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1}, 22);
}

void TestFlattenGraphFilter::testSimpleHole() 
{
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 13,
      std::deque<std::shared_ptr<Token>>{token(L"hello", 1, 1, 0, 5),
                                          token(L"hole", 2, 1, 6, 10),
                                          token(L"fun", 1, 1, 11, 13)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... but on output, it's flattened to wtf/what/wow that's/the fudge/funny
  // happened:
  assertTokenStreamContents(
      out, std::deque<wstring>{L"hello", L"hole", L"fun"},
      std::deque<int>{0, 6, 11}, std::deque<int>{5, 10, 13},
      std::deque<int>{1, 2, 1}, std::deque<int>{1, 1, 1}, 13);
}

void TestFlattenGraphFilter::testHoleUnderSyn() 
{
  // Tests a StopFilter after SynFilter where a stopword in a syn is removed
  //
  //   wizard of oz -> woz syn, but then "of" becomes a hole

  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 12,
      std::deque<std::shared_ptr<Token>>{token(L"wizard", 1, 1, 0, 6),
                                          token(L"woz", 0, 3, 0, 12),
                                          token(L"oz", 2, 1, 10, 12)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  assertTokenStreamContents(
      out, std::deque<wstring>{L"wizard", L"woz", L"oz"},
      std::deque<int>{0, 0, 10}, std::deque<int>{6, 12, 12},
      std::deque<int>{1, 0, 2}, std::deque<int>{1, 3, 1}, 12);
}

void TestFlattenGraphFilter::testStrangelyNumberedNodes() 
{

  // Uses only nodes 0, 2, 3, i.e. 1 is just never used (it is not a hole!!)
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 27,
      std::deque<std::shared_ptr<Token>>{token(L"dog", 1, 3, 0, 5),
                                          token(L"puppy", 0, 3, 0, 5),
                                          token(L"flies", 3, 1, 6, 11)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  assertTokenStreamContents(
      out, std::deque<wstring>{L"dog", L"puppy", L"flies"},
      std::deque<int>{0, 0, 6}, std::deque<int>{5, 5, 11},
      std::deque<int>{1, 0, 1}, std::deque<int>{1, 1, 1}, 27);
}

void TestFlattenGraphFilter::testTwoLongParallelPaths() 
{

  // "a a a a a a" in parallel with "b b b b b b"
  shared_ptr<TokenStream> in_ = make_shared<CannedTokenStream>(
      0, 11,
      std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1, 0, 1), token(L"b", 0, 2, 0, 1),
          token(L"a", 1, 2, 2, 3), token(L"b", 1, 2, 2, 3),
          token(L"a", 1, 2, 4, 5), token(L"b", 1, 2, 4, 5),
          token(L"a", 1, 2, 6, 7), token(L"b", 1, 2, 6, 7),
          token(L"a", 1, 2, 8, 9), token(L"b", 1, 2, 8, 9),
          token(L"a", 1, 2, 10, 11), token(L"b", 1, 2, 10, 11)});

  shared_ptr<TokenStream> out = make_shared<FlattenGraphFilter>(in_);

  // ... becomes flattened to a single path with overlapping a/b token between
  // each node:
  assertTokenStreamContents(
      out,
      std::deque<wstring>{L"a", L"b", L"a", L"b", L"a", L"b", L"a", L"b", L"a",
                           L"b", L"a", L"b"},
      std::deque<int>{0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10},
      std::deque<int>{1, 1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11},
      std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
      std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 11);
}
} // namespace org::apache::lucene::analysis::core