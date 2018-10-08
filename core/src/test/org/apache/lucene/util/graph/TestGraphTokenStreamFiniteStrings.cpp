using namespace std;

#include "TestGraphTokenStreamFiniteStrings.h"

namespace org::apache::lucene::util::graph
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Term = org::apache::lucene::index::Term;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

shared_ptr<Token> TestGraphTokenStreamFiniteStrings::token(const wstring &term,
                                                           int posInc,
                                                           int posLength)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, 0, term.length());
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

void TestGraphTokenStreamFiniteStrings::assertTokenStream(
    shared_ptr<TokenStream> ts, std::deque<wstring> &terms,
    std::deque<int> &increments) 
{
  // verify no nulls and arrays same length
  assertNotNull(ts);
  assertNotNull(terms);
  assertNotNull(increments);
  assertEquals(terms.size(), increments.size());
  shared_ptr<BytesTermAttribute> termAtt =
      ts->getAttribute(BytesTermAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> incrAtt =
      ts->getAttribute(PositionIncrementAttribute::typeid);
  int offset = 0;
  while (ts->incrementToken()) {
    // verify term and increment
    assert(offset < terms.size());
    assertEquals(terms[offset], termAtt->getBytesRef()->utf8ToString());
    assertEquals(increments[offset], incrAtt->getPositionIncrement());
    offset++;
  }

  // make sure we processed all items
  assertEquals(offset, terms.size());
}

void TestGraphTokenStreamFiniteStrings::testIllegalState() 
{
  expectThrows(IllegalStateException::typeid, [&]() {
    shared_ptr<TokenStream> ts =
        make_shared<CannedTokenStream>(token(L"a", 0, 1), token(L"b", 1, 1));
    (make_shared<GraphTokenStreamFiniteStrings>(ts))->getFiniteStrings();
  });
}

void TestGraphTokenStreamFiniteStrings::testEmpty() 
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>();
  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);
  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertFalse(it->hasNext());
  assertArrayEquals(std::deque<int>(0), graph->articulationPoints());
}

void TestGraphTokenStreamFiniteStrings::testSingleGraph() 
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"fast", 1, 1), token(L"wi", 1, 1), token(L"wifi", 0, 2),
      token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 3});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 3);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(3));
  it = graph->getFiniteStrings(3, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 3);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testSingleGraphWithGap() throw(
    runtime_error)
{
  // "hey the fast wifi network", where "the" removed
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"hey", 1, 1), token(L"fast", 2, 1), token(L"wi", 1, 1),
      token(L"wifi", 0, 2), token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"hey", L"fast", L"wi", L"fi", L"network"},
      std::deque<int>{1, 2, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"hey", L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 2, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 2, 4});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"hey"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"hey")});

  assertFalse(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 2);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{2});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 1);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast")});

  assertTrue(graph->hasSidePath(2));
  it = graph->getFiniteStrings(2, 4);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(4));
  it = graph->getFiniteStrings(4, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 4);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testGraphAndGapSameToken() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"fast", 1, 1), token(L"wi", 2, 1), token(L"wifi", 0, 2),
      token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 2, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 2, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 3});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 3);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{2});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(3));
  it = graph->getFiniteStrings(3, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 3);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testGraphAndGapSameTokenTerm() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"a", 1, 1), token(L"b", 1, 1), token(L"c", 2, 1),
      token(L"a", 0, 2), token(L"d", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"a", L"b", L"c", L"d"},
                    std::deque<int>{1, 1, 2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"a", L"b", L"a"},
                    std::deque<int>{1, 1, 2});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 2});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"a"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"a")});

  assertFalse(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 2);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"b"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 1);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"b")});

  assertTrue(graph->hasSidePath(2));
  it = graph->getFiniteStrings(2, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"c", L"d"},
                    std::deque<int>{2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"a"},
                    std::deque<int>{2});
  assertFalse(it->hasNext());
}

void TestGraphTokenStreamFiniteStrings::testStackedGraph() 
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"fast", 1, 1), token(L"wi", 1, 1), token(L"wifi", 0, 2),
      token(L"wireless", 0, 2), token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wireless", L"network"},
                    std::deque<int>{1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 3});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 3);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wireless"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(3));
  it = graph->getFiniteStrings(3, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 3);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testStackedGraphWithGap() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"fast", 1, 1), token(L"wi", 2, 1), token(L"wifi", 0, 2),
      token(L"wireless", 0, 2), token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 2, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wireless", L"network"},
                    std::deque<int>{1, 2, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 3});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 3);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{2});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wireless"},
                    std::deque<int>{2});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(3));
  it = graph->getFiniteStrings(3, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 3);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testStackedGraphWithRepeat() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"ny", 1, 4), token(L"new", 0, 1), token(L"new", 0, 3),
      token(L"york", 1, 1), token(L"city", 1, 2), token(L"york", 1, 1),
      token(L"is", 1, 1), token(L"great", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"ny", L"is", L"great"},
                    std::deque<int>{1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"new", L"york", L"city", L"is", L"great"},
      std::deque<int>{1, 1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"new", L"york", L"is", L"great"},
                    std::deque<int>{1, 1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{4, 5});

  assertTrue(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 4);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"ny"},
                    std::deque<int>{1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"new", L"york", L"city"},
                    std::deque<int>{1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"new", L"york"},
                    std::deque<int>{1, 1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(4));
  it = graph->getFiniteStrings(4, 5);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"is"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 4);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"is")});

  assertFalse(graph->hasSidePath(5));
  it = graph->getFiniteStrings(5, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"great"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 5);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"great")});
}

void TestGraphTokenStreamFiniteStrings::testGraphWithRegularSynonym() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"fast", 1, 1), token(L"speedy", 0, 1), token(L"wi", 1, 1),
      token(L"wifi", 0, 2), token(L"fi", 1, 1), token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"speedy", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"speedy", L"wifi", L"network"},
                    std::deque<int>{1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 3});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"speedy"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"fast"),
                               make_shared<Term>(L"field", L"speedy")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 3);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(3));
  it = graph->getFiniteStrings(3, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 3);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testMultiGraph() 
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"turbo", 1, 1), token(L"fast", 0, 2), token(L"charged", 1, 1),
      token(L"wi", 1, 1), token(L"wifi", 0, 2), token(L"fi", 1, 1),
      token(L"network", 1, 1));

  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"turbo", L"charged", L"wi", L"fi", L"network"},
      std::deque<int>{1, 1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"turbo", L"charged", L"wifi", L"network"},
      std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wi", L"fi", L"network"},
                    std::deque<int>{1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"fast", L"wifi", L"network"},
                    std::deque<int>{1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{2, 4});

  assertTrue(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 2);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"turbo", L"charged"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"fast"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertTrue(graph->hasSidePath(2));
  it = graph->getFiniteStrings(2, 4);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wi", L"fi"},
                    std::deque<int>{1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"wifi"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(4));
  it = graph->getFiniteStrings(4, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 4);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}

void TestGraphTokenStreamFiniteStrings::testMultipleSidePaths() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      token(L"the", 1, 1), token(L"ny", 1, 4), token(L"new", 0, 1),
      token(L"york", 1, 1), token(L"wifi", 1, 4), token(L"wi", 0, 1),
      token(L"fi", 1, 3), token(L"wifi", 2, 2), token(L"wi", 0, 1),
      token(L"fi", 1, 1), token(L"network", 1, 1));
  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(ts);

  Iterator<std::shared_ptr<TokenStream>> it = graph->getFiniteStrings();
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"the", L"ny", L"wifi", L"network"},
                    std::deque<int>{1, 1, 2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(), std::deque<wstring>{L"the", L"ny", L"wi", L"fi", L"network"},
      std::deque<int>{1, 1, 2, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"the", L"new", L"york", L"wifi", L"network"},
      std::deque<int>{1, 1, 1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(
      it->next(),
      std::deque<wstring>{L"the", L"new", L"york", L"wi", L"fi", L"network"},
      std::deque<int>{1, 1, 1, 1, 1, 1});
  assertFalse(it->hasNext());

  std::deque<int> points = graph->articulationPoints();
  assertArrayEquals(points, std::deque<int>{1, 7});

  assertFalse(graph->hasSidePath(0));
  it = graph->getFiniteStrings(0, 1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"the"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  std::deque<std::shared_ptr<Term>> terms = graph->getTerms(L"field", 0);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"the")});

  assertTrue(graph->hasSidePath(1));
  it = graph->getFiniteStrings(1, 7);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"ny", L"wifi"},
                    std::deque<int>{1, 2});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"ny", L"wi", L"fi"},
                    std::deque<int>{1, 2, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"new", L"york", L"wifi"},
                    std::deque<int>{1, 1, 1});
  assertTrue(it->hasNext());
  assertTokenStream(it->next(),
                    std::deque<wstring>{L"new", L"york", L"wi", L"fi"},
                    std::deque<int>{1, 1, 1, 1});
  assertFalse(it->hasNext());

  assertFalse(graph->hasSidePath(7));
  it = graph->getFiniteStrings(7, -1);
  assertTrue(it->hasNext());
  assertTokenStream(it->next(), std::deque<wstring>{L"network"},
                    std::deque<int>{1});
  assertFalse(it->hasNext());
  terms = graph->getTerms(L"field", 7);
  assertArrayEquals(terms, std::deque<std::shared_ptr<Term>>{
                               make_shared<Term>(L"field", L"network")});
}
} // namespace org::apache::lucene::util::graph