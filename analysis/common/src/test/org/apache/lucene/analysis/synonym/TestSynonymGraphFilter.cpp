using namespace std;

#include "TestSynonymGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Operations.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/TooComplexToDeterminizeException.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockGraphTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/automaton/AutomatonTestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/FlattenGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SolrSynonymParser.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using FlattenGraphFilter =
    org::apache::lucene::analysis::core::FlattenGraphFilter;
using namespace org::apache::lucene::analysis::tokenattributes;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using Operations = org::apache::lucene::util::automaton::Operations;
using TooComplexToDeterminizeException =
    org::apache::lucene::util::automaton::TooComplexToDeterminizeException;
using Transition = org::apache::lucene::util::automaton::Transition;
using Util = org::apache::lucene::util::fst::Util;

void TestSynonymGraphFilter::testBasicKeepOrigOneOutput() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"c a b", std::deque<wstring>{L"c", L"x", L"a", L"b"},
                   std::deque<int>{0, 2, 2, 4}, std::deque<int>{1, 5, 3, 5},
                   std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word"},
                   std::deque<int>{1, 1, 0, 1}, std::deque<int>{1, 2, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testMixedKeepOrig() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);
  add(b, L"e f", L"y", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a b c e f g",
      std::deque<wstring>{L"c", L"x", L"a", L"b", L"c", L"y", L"g"},
      std::deque<int>{0, 2, 2, 4, 6, 8, 12},
      std::deque<int>{1, 5, 3, 5, 7, 11, 13},
      std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word", L"word",
                           L"SYNONYM", L"word"},
      std::deque<int>{1, 1, 0, 1, 1, 1, 1},
      std::deque<int>{1, 2, 1, 1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testNoParseAfterBuffer() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"b a", L"x", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"b b b", std::deque<wstring>{L"b", L"b", L"b"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{1, 3, 5},
                   std::deque<wstring>{L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testOneInputMultipleOutputKeepOrig() throw(
    runtime_error)
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);
  add(b, L"a b", L"y", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a b c", std::deque<wstring>{L"c", L"x", L"y", L"a", L"b", L"c"},
      std::deque<int>{0, 2, 2, 2, 4, 6}, std::deque<int>{1, 5, 5, 3, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"word", L"word",
                           L"word"},
      std::deque<int>{1, 1, 0, 0, 1, 1, 1, 1},
      std::deque<int>{1, 2, 2, 1, 1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testPositionLengthAndTypeSimple() throw(
    runtime_error)
{
  wstring testFile = L"spider man, spiderman";

  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesToPositions(
      analyzer, L"spider man",
      std::deque<wstring>{L"spiderman", L"spider", L"man"},
      std::deque<wstring>{L"SYNONYM", L"word", L"word"},
      std::deque<int>{1, 0, 1}, std::deque<int>{2, 1, 1});
}

void TestSynonymGraphFilter::testEscapedStuff() 
{
  wstring testFile = wstring(L"a\\=>a => b\\=>b\n") + L"a\\,a => b\\,b";
  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesTo(analyzer, L"ball", std::deque<wstring>{L"ball"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"a=>a", std::deque<wstring>{L"b=>b"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"a,a", std::deque<wstring>{L"b,b"},
                   std::deque<int>{1});
  delete analyzer;
}

void TestSynonymGraphFilter::testInvalidAnalyzesToNothingOutput() throw(
    runtime_error)
{
  wstring testFile = L"a => 1";
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, false);
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  try {
    parser->parse(make_shared<StringReader>(testFile));
    fail(L"didn't get expected exception");
  } catch (const ParseException &expected) {
    // expected exc
  }
  delete analyzer;
}

void TestSynonymGraphFilter::testInvalidDoubleMap() 
{
  wstring testFile = L"a => b => c";
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  try {
    parser->parse(make_shared<StringReader>(testFile));
    fail(L"didn't get expected exception");
  } catch (const ParseException &expected) {
    // expected exc
  }
  delete analyzer;
}

void TestSynonymGraphFilter::testSimple() 
{
  wstring testFile = wstring(L"i-pod, ipod, ipoooood\n") + L"foo => foo bar\n" +
                     L"foo => baz\n" + L"this test, that testing";

  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesTo(analyzer, L"ball", std::deque<wstring>{L"ball"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"i-pod",
                   std::deque<wstring>{L"ipod", L"ipoooood", L"i-pod"},
                   std::deque<int>{1, 0, 0});

  assertAnalyzesTo(analyzer, L"foo",
                   std::deque<wstring>{L"foo", L"baz", L"bar"},
                   std::deque<int>{1, 0, 1});

  assertAnalyzesTo(analyzer, L"this test",
                   std::deque<wstring>{L"that", L"this", L"testing", L"test"},
                   std::deque<int>{1, 0, 1, 0});
  delete analyzer;
}

void TestSynonymGraphFilter::testBufferLength() 
{
  wstring testFile = wstring(L"c => 8 2 5 6 7\n") + L"f c e d f, 1\n" +
                     L"c g a f d, 6 5 5\n" + L"e c => 4\n" + L"g => 5\n" +
                     L"a g b f e => 5 0 7 7\n" + L"b => 1";
  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  wstring doc = L"b c g a f b d";
  std::deque<wstring> expected = {L"1", L"8", L"2", L"5", L"6", L"7",
                                   L"5", L"a", L"f", L"1", L"d"};
  assertAnalyzesTo(analyzer, doc, expected);
}

shared_ptr<Analyzer> TestSynonymGraphFilter::solrSynsToAnalyzer(
    const wstring &syns) 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(syns));
  delete analyzer;
  return getFlattenAnalyzer(parser, true);
}

void TestSynonymGraphFilter::testMoreThanOneLookAhead() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b c d", L"x", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"a b c e", std::deque<wstring>{L"a", L"b", L"c", L"e"},
                   std::deque<int>{0, 2, 4, 6}, std::deque<int>{1, 3, 5, 7},
                   std::deque<wstring>{L"word", L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testLookaheadAfterParse() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"b b", L"x", true);
  add(b, L"b", L"y", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(
      a, L"b a b b", std::deque<wstring>{L"y", L"b", L"a", L"x", L"b", L"b"},
      std::deque<int>{0, 0, 2, 4, 4, 6}, std::deque<int>{1, 1, 3, 7, 5, 7},
      nullptr, std::deque<int>{1, 0, 1, 1, 0, 1},
      std::deque<int>{1, 1, 1, 2, 1, 1}, true);
}

void TestSynonymGraphFilter::testLookaheadSecondParse() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"b b b", L"x", true);
  add(b, L"b", L"y", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"b b", std::deque<wstring>{L"y", L"b", L"y", L"b"},
                   std::deque<int>{0, 0, 2, 2}, std::deque<int>{1, 1, 3, 3},
                   nullptr, std::deque<int>{1, 0, 1, 0},
                   std::deque<int>{1, 1, 1, 1}, true);
}

void TestSynonymGraphFilter::testOneInputMultipleOutputNoKeepOrig() throw(
    runtime_error)
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", false);
  add(b, L"a b", L"y", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a b c", std::deque<wstring>{L"c", L"x", L"y", L"c"},
      std::deque<int>{0, 2, 2, 6}, std::deque<int>{1, 5, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"word"},
      std::deque<int>{1, 1, 0, 1}, std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testOneInputMultipleOutputMixedKeepOrig() throw(
    runtime_error)
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);
  add(b, L"a b", L"y", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a b c", std::deque<wstring>{L"c", L"x", L"y", L"a", L"b", L"c"},
      std::deque<int>{0, 2, 2, 2, 4, 6}, std::deque<int>{1, 5, 5, 3, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"word", L"word",
                           L"word"},
      std::deque<int>{1, 1, 0, 0, 1, 1, 1, 1},
      std::deque<int>{1, 2, 2, 1, 1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testSynAtEnd() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c d e a b", std::deque<wstring>{L"c", L"d", L"e", L"x", L"a", L"b"},
      std::deque<int>{0, 2, 4, 6, 6, 8}, std::deque<int>{1, 3, 5, 9, 7, 9},
      std::deque<wstring>{L"word", L"word", L"word", L"SYNONYM", L"word",
                           L"word"},
      std::deque<int>{1, 1, 1, 1, 0, 1}, std::deque<int>{1, 1, 1, 2, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testTwoSynsInARow() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a", L"x", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a a b", std::deque<wstring>{L"c", L"x", L"x", L"b"},
      std::deque<int>{0, 2, 4, 6}, std::deque<int>{1, 3, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"word"},
      std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBasicKeepOrigTwoOutputs() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x y", true);
  add(b, L"a b", L"m n o", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"c a b d",
                   std::deque<wstring>{L"c", L"x", L"m", L"a", L"y", L"n",
                                        L"o", L"b", L"d"},
                   std::deque<int>{0, 2, 2, 2, 2, 2, 2, 4, 6},
                   std::deque<int>{1, 5, 5, 3, 5, 5, 5, 5, 7},
                   std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM",
                                        L"word", L"SYNONYM", L"SYNONYM",
                                        L"SYNONYM", L"word", L"word"},
                   std::deque<int>{1, 1, 0, 0, 1, 1, 1, 1, 1},
                   std::deque<int>{1, 1, 2, 4, 4, 1, 2, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testNoCaptureIfNoMatch() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x y", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"c d d", std::deque<wstring>{L"c", L"d", L"d"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{1, 3, 5},
                   std::deque<wstring>{L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});
  TestUtil::assertEquals(0, synFilter->getCaptureCount());
  delete a;
}

void TestSynonymGraphFilter::testBasicNotKeepOrigOneOutput() throw(
    runtime_error)
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"c a b", std::deque<wstring>{L"c", L"x"},
                   std::deque<int>{0, 2}, std::deque<int>{1, 5},
                   std::deque<wstring>{L"word", L"SYNONYM"},
                   std::deque<int>{1, 1}, std::deque<int>{1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBasicNoKeepOrigTwoOutputs() throw(
    runtime_error)
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x y", false);
  add(b, L"a b", L"m n o", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c a b d",
      std::deque<wstring>{L"c", L"x", L"m", L"y", L"n", L"o", L"d"},
      std::deque<int>{0, 2, 2, 2, 2, 2, 6},
      std::deque<int>{1, 5, 5, 5, 5, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM",
                           L"SYNONYM", L"SYNONYM", L"word"},
      std::deque<int>{1, 1, 0, 1, 1, 1, 1},
      std::deque<int>{1, 1, 2, 3, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testIgnoreCase() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x y", false);
  add(b, L"a b", L"m n o", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"c A B D",
      std::deque<wstring>{L"c", L"x", L"m", L"y", L"n", L"o", L"D"},
      std::deque<int>{0, 2, 2, 2, 2, 2, 6},
      std::deque<int>{1, 5, 5, 5, 5, 5, 7},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM",
                           L"SYNONYM", L"SYNONYM", L"word"},
      std::deque<int>{1, 1, 0, 1, 1, 1, 1},
      std::deque<int>{1, 1, 2, 3, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testDoNotIgnoreCase() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x y", false);
  add(b, L"a b", L"m n o", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, false);
  assertAnalyzesTo(a, L"c A B D", std::deque<wstring>{L"c", L"A", L"B", L"D"},
                   std::deque<int>{0, 2, 4, 6}, std::deque<int>{1, 3, 5, 7},
                   std::deque<wstring>{L"word", L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBufferedFinish1() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b c", L"m n o", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"c a b", std::deque<wstring>{L"c", L"a", L"b"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{1, 3, 5},
                   std::deque<wstring>{L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBufferedFinish2() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"m n o", false);
  add(b, L"d e", L"m n o", false);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"c a d", std::deque<wstring>{L"c", L"a", L"d"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{1, 3, 5},
                   std::deque<wstring>{L"word", L"word", L"word"},
                   std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testCanReuse() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b", L"x", true);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  for (int i = 0; i < 10; i++) {
    assertAnalyzesTo(
        a, L"c a b", std::deque<wstring>{L"c", L"x", L"a", L"b"},
        std::deque<int>{0, 2, 2, 4}, std::deque<int>{1, 5, 3, 5},
        std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word"},
        std::deque<int>{1, 1, 0, 1}, std::deque<int>{1, 2, 1, 1});
  }
  delete a;
}

void TestSynonymGraphFilter::testManyToOne() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b c", L"z", true);

  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(
      a, L"a b c d", std::deque<wstring>{L"z", L"a", L"b", L"c", L"d"},
      std::deque<int>{0, 0, 2, 4, 6}, std::deque<int>{5, 1, 3, 5, 7},
      std::deque<wstring>{L"SYNONYM", L"word", L"word", L"word", L"word"},
      std::deque<int>{1, 0, 1, 1, 1}, std::deque<int>{3, 1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBufferAfterMatch() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"a b c d", L"x", true);
  add(b, L"a b", L"y", false);

  // The 'c' token has to be buffered because SynGraphFilter
  // needs to know whether a b c d -> x matches:
  shared_ptr<Analyzer> a = getAnalyzer(b, true);
  assertAnalyzesTo(a, L"f a b c e",
                   std::deque<wstring>{L"f", L"y", L"c", L"e"},
                   std::deque<int>{0, 2, 6, 8}, std::deque<int>{1, 5, 7, 9},
                   std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word"},
                   std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testZeroSyns() 
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"aa bb"));
  try {
    make_shared<SynonymGraphFilter>(
        tokenizer, (make_shared<SynonymMap::Builder>(true))->build(), true);
    fail(L"did not hit expected exception");
  } catch (const invalid_argument &iae) {
    // expected
    TestUtil::assertEquals(L"fst must be non-null", iae.what());
  }
}

void TestSynonymGraphFilter::testOutputHangsOffEnd() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  // b hangs off the end (no input token under it):
  add(b, L"a", L"a b", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"a", L"b"},
                   std::deque<int>{0, 0}, std::deque<int>{1, 1}, nullptr,
                   std::deque<int>{1, 1}, std::deque<int>{1, 1}, true);
  delete a;
}

void TestSynonymGraphFilter::testDedup() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab"}, std::deque<int>{1});
  delete a;
}

void TestSynonymGraphFilter::testNoDedup() 
{
  // dedup is false:
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(false);
  constexpr bool keepOrig = false;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab", L"ab", L"ab"},
                   std::deque<int>{1, 0, 0});
  delete a;
}

void TestSynonymGraphFilter::testMatching() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a c", L"ac", keepOrig);
  add(b, L"a", L"aa", keepOrig);
  add(b, L"b", L"bb", keepOrig);
  add(b, L"z x c v", L"zxcv", keepOrig);
  add(b, L"x c", L"xc", keepOrig);

  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  checkOneTerm(a, L"$", L"$");
  checkOneTerm(a, L"a", L"aa");
  checkOneTerm(a, L"b", L"bb");

  assertAnalyzesTo(a, L"a $", std::deque<wstring>{L"aa", L"$"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"$ a", std::deque<wstring>{L"$", L"aa"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"a a", std::deque<wstring>{L"aa", L"aa"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"z x c v", std::deque<wstring>{L"zxcv"},
                   std::deque<int>{1});

  assertAnalyzesTo(a, L"z x c $", std::deque<wstring>{L"z", L"xc", L"$"},
                   std::deque<int>{1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testBasic1() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  add(b, L"a", L"foo", true);
  add(b, L"a b", L"bar fee", true);
  add(b, L"b c", L"dog collar", true);
  add(b, L"c d", L"dog harness holder extras", true);
  add(b, L"m c e", L"dog barks loudly", false);
  add(b, L"i j k", L"feep", true);

  add(b, L"e f", L"foo bar", false);
  add(b, L"e f", L"baz bee", false);

  add(b, L"z", L"boo", false);
  add(b, L"y", L"bee", true);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(a, L"a b c",
                   std::deque<wstring>{L"bar", L"a", L"fee", L"b", L"c"},
                   std::deque<int>{1, 0, 1, 0, 1});

  assertAnalyzesTo(a, L"x a b c d",
                   std::deque<wstring>{L"x", L"bar", L"a", L"fee", L"b",
                                        L"dog", L"c", L"harness", L"d",
                                        L"holder", L"extras"},
                   std::deque<int>{1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1});

  assertAnalyzesTo(
      a, L"a b a",
      std::deque<wstring>{L"bar", L"a", L"fee", L"b", L"foo", L"a"},
      std::deque<int>{1, 0, 1, 0, 1, 0});

  // outputs no longer add to one another:
  assertAnalyzesTo(a, L"c d c d",
                   std::deque<wstring>{L"dog", L"c", L"harness", L"d",
                                        L"holder", L"extras", L"dog", L"c",
                                        L"harness", L"d", L"holder", L"extras"},
                   std::deque<int>{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1});

  // two outputs for same input
  assertAnalyzesTo(a, L"e f",
                   std::deque<wstring>{L"foo", L"baz", L"bar", L"bee"},
                   std::deque<int>{1, 0, 1, 0});

  // verify multi-word / single-output offsets:
  assertAnalyzesTo(a, L"g i j k g",
                   std::deque<wstring>{L"g", L"feep", L"i", L"j", L"k", L"g"},
                   std::deque<int>{1, 1, 0, 1, 1, 1});

  // mixed keepOrig true/false:
  assertAnalyzesTo(
      a, L"a m c e x",
      std::deque<wstring>{L"foo", L"a", L"dog", L"barks", L"loudly", L"x"},
      std::deque<int>{1, 0, 1, 1, 1, 1});
  assertAnalyzesTo(a, L"c d m c e x",
                   std::deque<wstring>{L"dog", L"c", L"harness", L"d",
                                        L"holder", L"extras", L"dog", L"barks",
                                        L"loudly", L"x"},
                   std::deque<int>{1, 0, 1, 0, 1, 1, 1, 1, 1, 1});
  assertTrue(synFilter->getCaptureCount() > 0);

  // no captureStates when no syns matched
  assertAnalyzesTo(a, L"p q r s t",
                   std::deque<wstring>{L"p", L"q", L"r", L"s", L"t"},
                   std::deque<int>{1, 1, 1, 1, 1});
  TestUtil::assertEquals(0, synFilter->getCaptureCount());

  // captureStates are necessary for the single-token syn case:
  assertAnalyzesTo(a, L"p q z y t",
                   std::deque<wstring>{L"p", L"q", L"boo", L"bee", L"y", L"t"},
                   std::deque<int>{1, 1, 1, 1, 0, 1});
  assertTrue(synFilter->getCaptureCount() > 0);
}

void TestSynonymGraphFilter::testBasic2() 
{
  bool keepOrig = true;
  do {
    keepOrig = !keepOrig;

    shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
    add(b, L"aaa", L"aaaa1 aaaa2 aaaa3", keepOrig);
    add(b, L"bbb", L"bbbb1 bbbb2", keepOrig);
    shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

    if (keepOrig) {
      assertAnalyzesTo(a, L"xyzzy bbb pot of gold",
                       std::deque<wstring>{L"xyzzy", L"bbbb1", L"bbb",
                                            L"bbbb2", L"pot", L"of", L"gold"},
                       std::deque<int>{1, 1, 0, 1, 1, 1, 1});
      assertAnalyzesTo(a, L"xyzzy aaa pot of gold",
                       std::deque<wstring>{L"xyzzy", L"aaaa1", L"aaa",
                                            L"aaaa2", L"aaaa2", L"pot", L"of",
                                            L"gold"},
                       std::deque<int>{1, 1, 0, 1, 1, 1, 1, 1});
    } else {
      assertAnalyzesTo(a, L"xyzzy bbb pot of gold",
                       std::deque<wstring>{L"xyzzy", L"bbbb1", L"bbbb2",
                                            L"pot", L"of", L"gold"},
                       std::deque<int>{1, 1, 1, 1, 1, 1});
      assertAnalyzesTo(a, L"xyzzy aaa pot of gold",
                       std::deque<wstring>{L"xyzzy", L"aaaa1", L"aaaa2",
                                            L"aaaa3", L"pot", L"of", L"gold"},
                       std::deque<int>{1, 1, 1, 1, 1, 1, 1});
    }
  } while (keepOrig);
}

void TestSynonymGraphFilter::testFlattenedGraph() 
{

  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>();
  add(b, L"wtf", L"what the fudge", true);

  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(
      a, L"wtf happened",
      std::deque<wstring>{L"what", L"wtf", L"the", L"fudge", L"happened"},
      std::deque<int>{0, 0, 0, 0, 4}, std::deque<int>{3, 3, 3, 3, 12},
      nullptr, std::deque<int>{1, 0, 1, 1, 1}, std::deque<int>{1, 3, 1, 1, 1},
      true);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"wtf happened", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  // Good (this should not match, and doesn't):
  TestUtil::assertEquals(
      0, s->count(make_shared<PhraseQuery>(L"field", L"what", L"happened")));

  // Bad (this should match, but doesn't):
  TestUtil::assertEquals(
      0, s->count(make_shared<PhraseQuery>(L"field", L"wtf", L"happened")));

  // Good (this should match, and does):
  TestUtil::assertEquals(
      1, s->count(make_shared<PhraseQuery>(L"field", L"what", L"the", L"fudge",
                                           L"happened")));

  // Bad (this should not match, but does):
  TestUtil::assertEquals(
      1, s->count(make_shared<PhraseQuery>(L"field", L"wtf", L"the")));

  IOUtils::close({r, dir});
}

wstring TestSynonymGraphFilter::randomNonEmptyString()
{
  while (true) {
    wstring s = StringHelper::trim(TestUtil::randomUnicodeString(random()));
    // std::wstring s = TestUtil.randomSimpleString(random()).trim();
    if (s.length() != 0 && s.find(L'\u0000') == wstring::npos) {
      return s;
    }
  }
}

void TestSynonymGraphFilter::testRandomGraphAfter() 
{
  constexpr int numIters = atLeast(3);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random()->nextBoolean();
    constexpr bool doFlatten = random()->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj,
                                                 ignoreCase, doFlatten);

    checkRandomData(random(), analyzer, 100);
    delete analyzer;
  }
}

TestSynonymGraphFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSynonymGraphFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase, bool doFlatten)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
  this->doFlatten = doFlatten;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymGraphFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  shared_ptr<TokenStream> syns =
      make_shared<SynonymGraphFilter>(tokenizer, map_obj, ignoreCase);
  shared_ptr<TokenStream> graph =
      make_shared<MockGraphTokenFilter>(random(), syns);
  if (doFlatten) {
    graph = make_shared<FlattenGraphFilter>(graph);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, graph);
}

void TestSynonymGraphFilter::testEmptyStringInput() 
{
  constexpr int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    constexpr bool ignoreCase = random()->nextBoolean();

    shared_ptr<Analyzer> analyzer = getAnalyzer(b, ignoreCase);

    checkAnalysisConsistency(random(), analyzer, random()->nextBoolean(), L"");
    delete analyzer;
  }
}

void TestSynonymGraphFilter::testRandom2() 
{
  constexpr int numIters = atLeast(3);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    constexpr bool ignoreCase = random()->nextBoolean();
    constexpr bool doFlatten = random()->nextBoolean();

    shared_ptr<Analyzer> analyzer;
    if (doFlatten) {
      analyzer = getFlattenAnalyzer(b, ignoreCase);
    } else {
      analyzer = getAnalyzer(b, ignoreCase);
    }

    checkRandomData(random(), analyzer, 100);
    delete analyzer;
  }
}

void TestSynonymGraphFilter::testRandomHuge() 
{
  constexpr int numIters = atLeast(3);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    if (VERBOSE) {
      wcout << L"TEST: iter=" << i << L" numEntries=" << numEntries << endl;
    }
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    constexpr bool ignoreCase = random()->nextBoolean();
    constexpr bool doFlatten = random()->nextBoolean();

    shared_ptr<Analyzer> analyzer;
    if (doFlatten) {
      analyzer = getFlattenAnalyzer(b, ignoreCase);
    } else {
      analyzer = getAnalyzer(b, ignoreCase);
    }

    checkRandomData(random(), analyzer, 100, 1024);
    delete analyzer;
  }
}

void TestSynonymGraphFilter::testEmptyTerm() 
{
  constexpr int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    constexpr bool ignoreCase = random()->nextBoolean();

    shared_ptr<Analyzer> *const analyzer = getAnalyzer(b, ignoreCase);

    checkAnalysisConsistency(random(), analyzer, random()->nextBoolean(), L"");
    delete analyzer;
  }
}

void TestSynonymGraphFilter::testVanishingTermsNoFlatten() 
{
  wstring testFile =
      wstring(L"aaa => aaaa1 aaaa2 aaaa3\n") + L"bbb => bbbb1 bbbb2\n";
  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesTo(analyzer, L"xyzzy bbb pot of gold",
                   std::deque<wstring>{L"xyzzy", L"bbbb1", L"bbbb2", L"pot",
                                        L"of", L"gold"});

  // xyzzy aaa pot of gold -> xyzzy aaaa1 aaaa2 aaaa3 gold
  assertAnalyzesTo(analyzer, L"xyzzy aaa pot of gold",
                   std::deque<wstring>{L"xyzzy", L"aaaa1", L"aaaa2", L"aaaa3",
                                        L"pot", L"of", L"gold"});
  delete analyzer;
}

void TestSynonymGraphFilter::testVanishingTermsWithFlatten() throw(
    runtime_error)
{
  wstring testFile =
      wstring(L"aaa => aaaa1 aaaa2 aaaa3\n") + L"bbb => bbbb1 bbbb2\n";

  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesTo(analyzer, L"xyzzy bbb pot of gold",
                   std::deque<wstring>{L"xyzzy", L"bbbb1", L"bbbb2", L"pot",
                                        L"of", L"gold"});

  // xyzzy aaa pot of gold -> xyzzy aaaa1 aaaa2 aaaa3 gold
  assertAnalyzesTo(analyzer, L"xyzzy aaa pot of gold",
                   std::deque<wstring>{L"xyzzy", L"aaaa1", L"aaaa2", L"aaaa3",
                                        L"pot", L"of", L"gold"});
  delete analyzer;
}

void TestSynonymGraphFilter::testBuilderDedup() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab"}, std::deque<int>{1});
  delete a;
}

void TestSynonymGraphFilter::testBuilderNoDedup() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(false);
  constexpr bool keepOrig = false;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a b", L"ab", keepOrig);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab", L"ab", L"ab"},
                   std::deque<int>{1, 0, 0});
  delete a;
}

void TestSynonymGraphFilter::testRecursion1() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(b, L"zoo", L"zoo", keepOrig);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"$", L"zoo"},
                   std::deque<int>{1, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testRecursion2() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(b, L"zoo", L"zoo", keepOrig);
  add(b, L"zoo", L"zoo zoo", keepOrig);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  // verify("zoo zoo $ zoo", "zoo/zoo zoo/zoo/zoo $/zoo zoo/zoo zoo");
  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"zoo", L"zoo",
                                        L"zoo", L"$", L"zoo", L"zoo", L"zoo"},
                   std::deque<int>{1, 0, 1, 1, 0, 1, 1, 1, 0, 1});
  delete a;
}

void TestSynonymGraphFilter::testRecursion3() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(b, L"zoo zoo", L"zoo", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"$", L"zoo"},
                   std::deque<int>{1, 0, 1, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testRecursion4() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(b, L"zoo zoo", L"zoo", keepOrig);
  add(b, L"zoo", L"zoo zoo", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);
  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"$", L"zoo",
                                        L"zoo", L"zoo"},
                   std::deque<int>{1, 0, 1, 1, 1, 0, 1});
  delete a;
}

void TestSynonymGraphFilter::testKeepOrig() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a c", L"ac", keepOrig);
  add(b, L"a", L"aa", keepOrig);
  add(b, L"b", L"bb", keepOrig);
  add(b, L"z x c v", L"zxcv", keepOrig);
  add(b, L"x c", L"xc", keepOrig);
  shared_ptr<Analyzer> a = getAnalyzer(b, true);

  assertAnalyzesTo(a, L"$", std::deque<wstring>{L"$"}, std::deque<int>{1});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"aa", L"a"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"aa", L"a"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"$ a", std::deque<wstring>{L"$", L"aa", L"a"},
                   std::deque<int>{1, 1, 0});
  assertAnalyzesTo(a, L"a $", std::deque<wstring>{L"aa", L"a", L"$"},
                   std::deque<int>{1, 0, 1});
  assertAnalyzesTo(a, L"$ a !", std::deque<wstring>{L"$", L"aa", L"a", L"!"},
                   std::deque<int>{1, 1, 0, 1});
  assertAnalyzesTo(a, L"a a", std::deque<wstring>{L"aa", L"a", L"aa", L"a"},
                   std::deque<int>{1, 0, 1, 0});
  assertAnalyzesTo(a, L"b", std::deque<wstring>{L"bb", L"b"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"z x c v",
                   std::deque<wstring>{L"zxcv", L"z", L"x", L"c", L"v"},
                   std::deque<int>{1, 0, 1, 1, 1});
  assertAnalyzesTo(a, L"z x c $",
                   std::deque<wstring>{L"z", L"xc", L"x", L"c", L"$"},
                   std::deque<int>{1, 1, 0, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testNonPreservingMultiwordSynonyms() throw(
    runtime_error)
{
  wstring testFile =
      wstring(L"aaa => two words\n") +
      L"bbb => one two, very many multiple words\n" +
      L"ee ff, gg, h i j k, h i => one\n" +
      L"cc dd => usa,united states,u s a,united states of america";

  shared_ptr<Analyzer> analyzer = solrSynsToAnalyzer(testFile);

  assertAnalyzesTo(analyzer, L"aaa", std::deque<wstring>{L"two", L"words"},
                   std::deque<int>{0, 0}, std::deque<int>{3, 3},
                   std::deque<wstring>{L"SYNONYM", L"SYNONYM"},
                   std::deque<int>{1, 1}, std::deque<int>{1, 1});

  assertAnalyzesToPositions(
      analyzer, L"amazing aaa",
      std::deque<wstring>{L"amazing", L"two", L"words"},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM"},
      std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});

  assertAnalyzesTo(analyzer, L"p bbb s",
                   std::deque<wstring>{L"p", L"one", L"very", L"two", L"many",
                                        L"multiple", L"words", L"s"},
                   std::deque<int>{0, 2, 2, 2, 2, 2, 2, 6},
                   std::deque<int>{1, 5, 5, 5, 5, 5, 5, 7},
                   std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM",
                                        L"SYNONYM", L"SYNONYM", L"SYNONYM",
                                        L"SYNONYM", L"word"},
                   std::deque<int>{1, 1, 0, 1, 0, 1, 1, 1},
                   std::deque<int>{1, 1, 1, 3, 1, 1, 1, 1});

  assertAnalyzesTo(analyzer, L"p ee ff s",
                   std::deque<wstring>{L"p", L"one", L"s"},
                   std::deque<int>{0, 2, 8}, std::deque<int>{1, 7, 9},
                   std::deque<wstring>{L"word", L"SYNONYM", L"word"},
                   std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1});

  assertAnalyzesTo(analyzer, L"p h i j s",
                   std::deque<wstring>{L"p", L"one", L"j", L"s"},
                   std::deque<int>{0, 2, 6, 8}, std::deque<int>{1, 5, 7, 9},
                   std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word"},
                   std::deque<int>{1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1});

  delete analyzer;
}

shared_ptr<Analyzer>
TestSynonymGraphFilter::getAnalyzer(shared_ptr<SynonymMap::Builder> b,
                                    bool const ignoreCase) 
{
  shared_ptr<SynonymMap> *const map_obj = b->build();
  return make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(),
                                                   ignoreCase, map_obj);
}

TestSynonymGraphFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSynonymGraphFilter> outerInstance, bool ignoreCase,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->ignoreCase = ignoreCase;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymGraphFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  // Make a local variable so testRandomHuge doesn't share it across threads!
  shared_ptr<SynonymGraphFilter> synFilter =
      make_shared<SynonymGraphFilter>(tokenizer, map_obj, ignoreCase);
  outerInstance->flattenFilter.reset();
  outerInstance->synFilter = synFilter;
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, synFilter);
}

shared_ptr<Analyzer>
TestSynonymGraphFilter::getFlattenAnalyzer(shared_ptr<SynonymMap::Builder> b,
                                           bool ignoreCase) 
{
  shared_ptr<SynonymMap> *const map_obj = b->build();
  return make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(),
                                                   ignoreCase, map_obj);
}

TestSynonymGraphFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestSynonymGraphFilter> outerInstance, bool ignoreCase,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->ignoreCase = ignoreCase;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymGraphFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  // Make a local variable so testRandomHuge doesn't share it across threads!
  shared_ptr<SynonymGraphFilter> synFilter =
      make_shared<SynonymGraphFilter>(tokenizer, map_obj, ignoreCase);
  shared_ptr<FlattenGraphFilter> flattenFilter =
      make_shared<FlattenGraphFilter>(synFilter);
  outerInstance->synFilter = synFilter;
  outerInstance->flattenFilter = flattenFilter;
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, flattenFilter);
}

void TestSynonymGraphFilter::add(shared_ptr<SynonymMap::Builder> b,
                                 const wstring &input, const wstring &output,
                                 bool keepOrig)
{
  if (VERBOSE) {
    // System.out.println("  add input=" + input + " output=" + output + "
    // keepOrig=" + keepOrig);
  }
  shared_ptr<CharsRefBuilder> inputCharsRef = make_shared<CharsRefBuilder>();
  SynonymMap::Builder::join(input.split(L" +"), inputCharsRef);

  shared_ptr<CharsRefBuilder> outputCharsRef = make_shared<CharsRefBuilder>();
  SynonymMap::Builder::join(output.split(L" +"), outputCharsRef);

  b->add(inputCharsRef->get(), outputCharsRef->get(), keepOrig);
}

std::deque<wchar_t> TestSynonymGraphFilter::randomBinaryChars(int minLen,
                                                               int maxLen,
                                                               double bias,
                                                               wchar_t base)
{
  int len = TestUtil::nextInt(random(), minLen, maxLen);
  std::deque<wchar_t> chars(len);
  for (int i = 0; i < len; i++) {
    wchar_t ch;
    if (random()->nextDouble() < bias) {
      ch = base;
    } else {
      ch = static_cast<wchar_t>(base + 1);
    }
    chars[i] = ch;
  }

  return chars;
}

wstring TestSynonymGraphFilter::toTokenString(std::deque<wchar_t> &chars)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (auto c : chars) {
    if (b->length() > 0) {
      b->append(L' ');
    }
    b->append(c);
  }
  return b->toString();
}

wstring TestSynonymGraphFilter::OneSyn::toString()
{
  return toTokenString(in_) + L" --> " + toTokenString(out) + L" (keepOrig=" +
         StringHelper::toString(keepOrig) + L")";
}

void TestSynonymGraphFilter::testRandomSyns() 
{
  int synCount = atLeast(10);
  double bias = random()->nextDouble();
  bool dedup = random()->nextBoolean();

  bool flatten = random()->nextBoolean();

  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(dedup);
  deque<std::shared_ptr<OneSyn>> syns = deque<std::shared_ptr<OneSyn>>();
  // Makes random syns from random a / b tokens, mapping to random x / y tokens
  if (VERBOSE) {
    wcout << L"TEST: make " << synCount << L" syns" << endl;
    wcout << L"  bias for a over b=" << bias << endl;
    wcout << L"  dedup=" << dedup << endl;
    wcout << L"  flatten=" << flatten << endl;
  }

  int maxSynLength = 0;

  for (int i = 0; i < synCount; i++) {
    shared_ptr<OneSyn> syn = make_shared<OneSyn>();
    syn->in_ = randomBinaryChars(1, 5, bias, L'a');
    syn->out = randomBinaryChars(1, 5, 0.5, L'x');
    syn->keepOrig = random()->nextBoolean();
    syns.push_back(syn);

    maxSynLength = max(maxSynLength, syn->in_.size());

    if (VERBOSE) {
      wcout << L"  " << syn << endl;
    }
    add(b, toTokenString(syn->in_), toTokenString(syn->out), syn->keepOrig);
  }

  // Compute max allowed lookahead for flatten filter:
  int maxFlattenLookahead = 0;
  if (flatten) {
    for (int i = 0; i < synCount; i++) {
      shared_ptr<OneSyn> syn1 = syns[i];
      int count = syn1->out.size();
      bool keepOrig = syn1->keepOrig;
      for (int j = 0; j < synCount; j++) {
        shared_ptr<OneSyn> syn2 = syns[i];
        keepOrig |= syn2->keepOrig;
        if (syn1->in_.equals(syn2->in_)) {
          count += syn2->out.size();
        }
      }

      if (keepOrig) {
        count += syn1->in_.size();
      }

      maxFlattenLookahead = max(maxFlattenLookahead, count);
    }
  }

  // Only used w/ VERBOSE:
  shared_ptr<Analyzer> aNoFlattened;
  if (VERBOSE) {
    aNoFlattened = getAnalyzer(b, true);
  } else {
    aNoFlattened.reset();
  }

  shared_ptr<Analyzer> a;
  if (flatten) {
    a = getFlattenAnalyzer(b, true);
  } else {
    a = getAnalyzer(b, true);
  }

  int iters = atLeast(20);
  for (int iter = 0; iter < iters; iter++) {

    wstring doc = toTokenString(randomBinaryChars(50, 100, bias, L'a'));
    // std::wstring doc = toTokenString(randomBinaryChars(10, 50, bias, 'a'));

    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << L" doc=" << doc << endl;
    }
    shared_ptr<Automaton> expected = slowSynFilter(doc, syns, flatten);
    if (VERBOSE) {
      wcout << L"  expected:\n" << expected->toDot() << endl;
      if (flatten) {
        shared_ptr<Automaton> unflattened =
            toAutomaton(aNoFlattened->tokenStream(
                L"field", make_shared<StringReader>(doc)));
        wcout << L"  actual unflattened:\n" << unflattened->toDot() << endl;
      }
    }
    shared_ptr<Automaton> actual =
        toAutomaton(a->tokenStream(L"field", make_shared<StringReader>(doc)));
    if (VERBOSE) {
      wcout << L"  actual:\n" << actual->toDot() << endl;
    }

    assertTrue(L"maxLookaheadUsed=" +
                   to_wstring(synFilter->getMaxLookaheadUsed()) +
                   L" maxSynLength=" + to_wstring(maxSynLength),
               synFilter->getMaxLookaheadUsed() <= maxSynLength);
    if (flatten) {
      assertTrue(L"flatten maxLookaheadUsed=" +
                     to_wstring(flattenFilter->getMaxLookaheadUsed()) +
                     L" maxFlattenLookahead=" + to_wstring(maxFlattenLookahead),
                 flattenFilter->getMaxLookaheadUsed() <= maxFlattenLookahead);
    }

    checkAnalysisConsistency(random(), a, random()->nextBoolean(), doc);
    // We can easily have a non-deterministic automaton at this point, e.g. if
    // more than one syn matched at given point, or if the syn mapped to an
    // output token that also happens to be in the input:
    try {
      actual = Operations::determinize(actual, 50000);
    } catch (const TooComplexToDeterminizeException &tctde) {
      // Unfortunately the syns can easily create difficult-to-determinize
      // graphs:
      assertTrue(approxEquals(actual, expected));
      continue;
    }

    try {
      expected = Operations::determinize(expected, 50000);
    } catch (const TooComplexToDeterminizeException &tctde) {
      // Unfortunately the syns can easily create difficult-to-determinize
      // graphs:
      assertTrue(approxEquals(actual, expected));
      continue;
    }

    assertTrue(approxEquals(actual, expected));
    assertTrue(Operations::sameLanguage(actual, expected));
  }

  delete a;
}

bool TestSynonymGraphFilter::approxEquals(shared_ptr<Automaton> actual,
                                          shared_ptr<Automaton> expected)
{
  // Don't collapse these into one line else the thread stack won't say which
  // direction failed!:
  bool b1 = approxSubsetOf(actual, expected);
  bool b2 = approxSubsetOf(expected, actual);
  return b1 && b2;
}

bool TestSynonymGraphFilter::approxSubsetOf(shared_ptr<Automaton> a1,
                                            shared_ptr<Automaton> a2)
{
  shared_ptr<AutomatonTestUtil::RandomAcceptedStrings> ras =
      make_shared<AutomatonTestUtil::RandomAcceptedStrings>(a1);
  for (int i = 0; i < 2000; i++) {
    std::deque<int> ints = ras->getRandomAcceptedString(random());
    shared_ptr<IntsRef> path = make_shared<IntsRef>(ints, 0, ints.size());
    if (accepts(a2, path) == false) {
      throw runtime_error(L"a2 does not accept " + path);
    }
  }

  // Presumed true
  return true;
}

bool TestSynonymGraphFilter::accepts(shared_ptr<Automaton> a,
                                     shared_ptr<IntsRef> path)
{
  shared_ptr<Set<int>> states = unordered_set<int>();
  states->add(0);
  shared_ptr<Transition> t = make_shared<Transition>();
  for (int i = 0; i < path->length; i++) {
    int digit = path->ints[path->offset + i];
    shared_ptr<Set<int>> nextStates = unordered_set<int>();
    for (auto state : states) {
      int count = a->initTransition(state, t);
      for (int j = 0; j < count; j++) {
        a->getNextTransition(t);
        if (digit >= t->min && digit <= t->max) {
          nextStates->add(t->dest);
        }
      }
    }
    states = nextStates;
    if (states->isEmpty()) {
      return false;
    }
  }

  for (auto state : states) {
    if (a->isAccept(state)) {
      return true;
    }
  }

  return false;
}

shared_ptr<Automaton> TestSynonymGraphFilter::slowSynFilter(
    const wstring &doc, deque<std::shared_ptr<OneSyn>> &syns, bool flatten)
{
  std::deque<wstring> tokens = doc.split(L" +");
  if (VERBOSE) {
    wcout << L"  doc has " << tokens.size() << L" tokens" << endl;
  }
  int i = 0;
  shared_ptr<Automaton::Builder> a = make_shared<Automaton::Builder>();
  int lastState = a->createState();
  while (i < tokens.size()) {
    // Consider all possible syn matches starting at this point:
    assert(tokens[i].length() == 1);
    if (VERBOSE) {
      wcout << L"    i=" << i << endl;
    }

    deque<std::shared_ptr<OneSyn>> matches = deque<std::shared_ptr<OneSyn>>();
    for (auto syn : syns) {
      if (i + syn->in_.size() <= tokens.size()) {
        bool match = true;
        for (int j = 0; j < syn->in_.size(); j++) {
          if (tokens[i + j][0] != syn->in_[j]) {
            match = false;
            break;
          }
        }

        if (match) {
          if (matches.empty() == false) {
            if (syn->in_.size() < matches[0]->in_->size()) {
              // Greedy matching: we already found longer syns matching here
              continue;
            } else if (syn->in_.size() > matches[0]->in_->size()) {
              // Greedy matching: all previous matches were shorter, so we drop
              // them
              matches.clear();
            } else {
              // Keep the current matches: we allow multiple synonyms matching
              // the same input string
            }
          }

          matches.push_back(syn);
        }
      }
    }

    int nextState = a->createState();

    if (matches.empty() == false) {
      // We have match(es) starting at this token
      if (VERBOSE) {
        wcout << L"  matches @ i=" << i << L": " << matches << endl;
      }
      // We keepOrig if any of the matches said to:
      bool keepOrig = false;
      for (auto syn : matches) {
        keepOrig |= syn->keepOrig;
      }

      deque<int> flatStates;
      if (flatten) {
        flatStates = deque<>();
      } else {
        flatStates.clear();
      }

      if (keepOrig) {
        // Add path for the original tokens
        addSidePath(a, lastState, nextState, matches[0]->in_, flatStates);
      }

      for (auto syn : matches) {
        addSidePath(a, lastState, nextState, syn->out, flatStates);
      }

      i += matches[0]->in_->size();
    } else {
      a->addTransition(lastState, nextState, tokens[i][0]);
      i++;
    }

    lastState = nextState;
  }

  a->setAccept(lastState, true);

  return topoSort(a->finish());
}

void TestSynonymGraphFilter::addSidePath(shared_ptr<Automaton::Builder> a,
                                         int startState, int endState,
                                         std::deque<wchar_t> &tokens,
                                         deque<int> &flatStates)
{
  int lastState = startState;
  for (int i = 0; i < tokens.size(); i++) {
    int nextState;
    if (i == tokens.size() - 1) {
      nextState = endState;
    } else if (flatStates.empty() || i >= flatStates.size()) {
      nextState = a->createState();
      if (flatStates.size() > 0) {
        assert(i == flatStates.size());
        flatStates.push_back(nextState);
      }
    } else {
      nextState = flatStates[i];
    }
    a->addTransition(lastState, nextState, tokens[i]);

    lastState = nextState;
  }
}

shared_ptr<Automaton> TestSynonymGraphFilter::toAutomaton(
    shared_ptr<TokenStream> ts) 
{
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      ts->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> posLenAtt =
      ts->addAttribute(PositionLengthAttribute::typeid);
  shared_ptr<CharTermAttribute> termAtt =
      ts->addAttribute(CharTermAttribute::typeid);
  ts->reset();
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int srcNode = -1;
  int destNode = -1;
  int state = a->createState();
  while (ts->incrementToken()) {
    assert(termAtt->length() == 1);
    wchar_t c = termAtt->charAt(0);
    int posInc = posIncAtt->getPositionIncrement();
    if (posInc != 0) {
      srcNode += posInc;
      while (state < srcNode) {
        state = a->createState();
      }
    }
    destNode = srcNode + posLenAtt->getPositionLength();
    while (state < destNode) {
      state = a->createState();
    }
    a->addTransition(srcNode, destNode, c);
  }
  ts->end();
  delete ts;
  a->finishState();
  a->setAccept(destNode, true);
  return a;
}

shared_ptr<Automaton>
TestSynonymGraphFilter::topoSort(shared_ptr<Automaton> in_)
{
  std::deque<int> newToOld = Operations::topoSortStates(in_);
  std::deque<int> oldToNew(newToOld.size());

  shared_ptr<Automaton::Builder> a = make_shared<Automaton::Builder>();
  // System.out.println("remap:");
  for (int i = 0; i < newToOld.size(); i++) {
    a->createState();
    oldToNew[newToOld[i]] = i;
    // System.out.println("  " + newToOld[i] + " -> " + i);
    if (in_->isAccept(newToOld[i])) {
      a->setAccept(i, true);
      // System.out.println("    **");
    }
  }

  shared_ptr<Transition> t = make_shared<Transition>();
  for (int i = 0; i < newToOld.size(); i++) {
    int count = in_->initTransition(newToOld[i], t);
    for (int j = 0; j < count; j++) {
      in_->getNextTransition(t);
      a->addTransition(i, oldToNew[t->dest], t->min, t->max);
    }
  }

  return a->finish();
}

void TestSynonymGraphFilter::testPositionLengthAndType() 
{
  wstring testFile = wstring(L"spider man, spiderman\n") +
                     L"usa,united states,u s a,united states of america";
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);

  parser->parse(make_shared<StringReader>(testFile));
  delete analyzer;

  shared_ptr<SynonymMap> map_obj = parser->build();
  analyzer = getFlattenAnalyzer(parser, true);

  shared_ptr<BytesRef> value =
      Util::get(map_obj->fst, Util::toUTF32(make_shared<CharsRef>(L"usa"),
                                        make_shared<IntsRefBuilder>()));
  shared_ptr<ByteArrayDataInput> bytesReader = make_shared<ByteArrayDataInput>(
      value->bytes, value->offset, value->length);
  constexpr int code = bytesReader->readVInt();
  constexpr int count = static_cast<int>(static_cast<unsigned int>(code) >> 1);

  const std::deque<int> synonymsIdxs = std::deque<int>(count);
  for (int i = 0; i < count; i++) {
    synonymsIdxs[i] = bytesReader->readVInt();
  }

  shared_ptr<BytesRef> scratchBytes = make_shared<BytesRef>();
  map_obj->words->get(synonymsIdxs[2], scratchBytes);

  int synonymLength = 1;
  for (int i = scratchBytes->offset;
       i < scratchBytes->offset + scratchBytes->length; i++) {
    if (scratchBytes->bytes[i] == SynonymMap::WORD_SEPARATOR) {
      synonymLength++;
    }
  }

  TestUtil::assertEquals(count, 3);
  TestUtil::assertEquals(synonymLength, 4);

  assertAnalyzesTo(analyzer, L"spider man",
                   std::deque<wstring>{L"spiderman", L"spider", L"man"},
                   std::deque<int>{0, 0, 7}, std::deque<int>{10, 6, 10},
                   std::deque<wstring>{L"SYNONYM", L"word", L"word"},
                   std::deque<int>{1, 0, 1}, std::deque<int>{2, 1, 1});

  assertAnalyzesToPositions(
      analyzer, L"amazing spider man",
      std::deque<wstring>{L"amazing", L"spiderman", L"spider", L"man"},
      std::deque<wstring>{L"word", L"SYNONYM", L"word", L"word"},
      std::deque<int>{1, 1, 0, 1}, std::deque<int>{1, 2, 1, 1});

  // System.out.println(toDot(getAnalyzer(parser, true).tokenStream("field", new
  // StringReader("the usa is wealthy"))));

  assertAnalyzesTo(
      analyzer, L"the united states of america is wealthy",
      std::deque<wstring>{L"the", L"usa", L"united", L"u", L"united",
                           L"states", L"s", L"states", L"a", L"of", L"america",
                           L"is", L"wealthy"},
      std::deque<int>{0, 4, 4, 4, 4, 11, 11, 11, 18, 18, 21, 29, 32},
      std::deque<int>{3, 28, 10, 10, 10, 28, 17, 17, 28, 20, 28, 31, 39},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM", L"word",
                           L"SYNONYM", L"SYNONYM", L"word", L"SYNONYM", L"word",
                           L"word", L"word", L"word"},
      std::deque<int>{1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
      std::deque<int>{1, 4, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 1});

  assertAnalyzesToPositions(
      analyzer, L"spiderman",
      std::deque<wstring>{L"spider", L"spiderman", L"man"},
      std::deque<wstring>{L"SYNONYM", L"word", L"SYNONYM"},
      std::deque<int>{1, 0, 1}, std::deque<int>{1, 2, 1});

  assertAnalyzesTo(
      analyzer, L"spiderman enemies",
      std::deque<wstring>{L"spider", L"spiderman", L"man", L"enemies"},
      std::deque<int>{0, 0, 0, 10}, std::deque<int>{9, 9, 9, 17},
      std::deque<wstring>{L"SYNONYM", L"word", L"SYNONYM", L"word"},
      std::deque<int>{1, 0, 1, 1}, std::deque<int>{1, 2, 1, 1});

  assertAnalyzesTo(
      analyzer, L"the usa is wealthy",
      std::deque<wstring>{L"the", L"united", L"u", L"united", L"usa",
                           L"states", L"s", L"states", L"a", L"of", L"america",
                           L"is", L"wealthy"},
      std::deque<int>{0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 11},
      std::deque<int>{3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 10, 18},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM", L"word",
                           L"SYNONYM", L"SYNONYM", L"SYNONYM", L"SYNONYM",
                           L"SYNONYM", L"SYNONYM", L"word", L"word"},
      std::deque<int>{1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
      std::deque<int>{1, 1, 1, 1, 4, 3, 1, 1, 2, 1, 1, 1, 1});

  assertGraphStrings(
      analyzer,
      {L"the usa is wealthy",
       std::deque<wstring>{
           L"the usa is wealthy", L"the united states is wealthy",
           L"the u s a is wealthy", L"the united states of america is wealthy",
           L"the u states is wealthy", L"the u states a is wealthy",
           L"the u s of america is wealthy",
           L"the u states of america is wealthy", L"the united s a is wealthy",
           L"the united states a is wealthy",
           L"the united s of america is wealthy"}});

  assertAnalyzesTo(
      analyzer, L"the united states is wealthy",
      std::deque<wstring>{L"the", L"usa", L"u", L"united", L"united", L"s",
                           L"states", L"states", L"a", L"of", L"america", L"is",
                           L"wealthy"},
      std::deque<int>{0, 4, 4, 4, 4, 11, 11, 11, 11, 11, 11, 18, 21},
      std::deque<int>{3, 17, 10, 10, 10, 17, 17, 17, 17, 17, 17, 20, 28},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM", L"word",
                           L"SYNONYM", L"SYNONYM", L"word", L"SYNONYM",
                           L"SYNONYM", L"SYNONYM", L"word", L"word"},
      std::deque<int>{1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
      std::deque<int>{1, 4, 1, 1, 1, 1, 1, 3, 2, 1, 1, 1, 1}, false);

  assertAnalyzesTo(
      analyzer, L"the united states of balance",
      std::deque<wstring>{L"the", L"usa", L"u", L"united", L"united", L"s",
                           L"states", L"states", L"a", L"of", L"america", L"of",
                           L"balance"},
      std::deque<int>{0, 4, 4, 4, 4, 11, 11, 11, 11, 11, 11, 18, 21},
      std::deque<int>{3, 17, 10, 10, 10, 17, 17, 17, 17, 17, 17, 20, 28},
      std::deque<wstring>{L"word", L"SYNONYM", L"SYNONYM", L"SYNONYM", L"word",
                           L"SYNONYM", L"SYNONYM", L"word", L"SYNONYM",
                           L"SYNONYM", L"SYNONYM", L"word", L"word"},
      std::deque<int>{1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
      std::deque<int>{1, 4, 1, 1, 1, 1, 1, 3, 2, 1, 1, 1, 1});

  delete analyzer;
}

void TestSynonymGraphFilter::testMultiwordOffsets() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(b, L"national hockey league", L"nhl", keepOrig);
  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(
      a, L"national hockey league",
      std::deque<wstring>{L"nhl", L"national", L"hockey", L"league"},
      std::deque<int>{0, 0, 9, 16}, std::deque<int>{22, 8, 15, 22},
      std::deque<int>{1, 0, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testIncludeOrig() 
{
  shared_ptr<SynonymMap::Builder> b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(b, L"a b", L"ab", keepOrig);
  add(b, L"a c", L"ac", keepOrig);
  add(b, L"a", L"aa", keepOrig);
  add(b, L"b", L"bb", keepOrig);
  add(b, L"z x c v", L"zxcv", keepOrig);
  add(b, L"x c", L"xc", keepOrig);

  shared_ptr<Analyzer> a = getFlattenAnalyzer(b, true);

  assertAnalyzesTo(a, L"$", std::deque<wstring>{L"$"}, std::deque<int>{1});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"aa", L"a"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"aa", L"a"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"$ a", std::deque<wstring>{L"$", L"aa", L"a"},
                   std::deque<int>{1, 1, 0});
  assertAnalyzesTo(a, L"a $", std::deque<wstring>{L"aa", L"a", L"$"},
                   std::deque<int>{1, 0, 1});
  assertAnalyzesTo(a, L"$ a !", std::deque<wstring>{L"$", L"aa", L"a", L"!"},
                   std::deque<int>{1, 1, 0, 1});
  assertAnalyzesTo(a, L"a a", std::deque<wstring>{L"aa", L"a", L"aa", L"a"},
                   std::deque<int>{1, 0, 1, 0});
  assertAnalyzesTo(a, L"b", std::deque<wstring>{L"bb", L"b"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"z x c v",
                   std::deque<wstring>{L"zxcv", L"z", L"x", L"c", L"v"},
                   std::deque<int>{1, 0, 1, 1, 1});
  assertAnalyzesTo(a, L"z x c $",
                   std::deque<wstring>{L"z", L"xc", L"x", L"c", L"$"},
                   std::deque<int>{1, 1, 0, 1, 1});
  delete a;
}

void TestSynonymGraphFilter::testUpperCase() 
{
  assertMapping(L"word", L"synonym");
  assertMapping((wstring(L"word")).toUpperCase(Locale::ROOT), L"synonym");
}

void TestSynonymGraphFilter::assertMapping(
    const wstring &inputString, const wstring &outputString) 
{
  shared_ptr<SynonymMap::Builder> builder =
      make_shared<SynonymMap::Builder>(false);
  // the rules must be lowercased up front, but the incoming tokens will be case
  // insensitive:
  shared_ptr<CharsRef> input = SynonymMap::Builder::join(
      inputString.toLowerCase(Locale::ROOT)->split(L" "),
      make_shared<CharsRefBuilder>());
  shared_ptr<CharsRef> output = SynonymMap::Builder::join(
      outputString.split(L" "), make_shared<CharsRefBuilder>());
  builder->add(input, output, true);
  shared_ptr<Analyzer> analyzer = make_shared<CustomAnalyzer>(builder->build());
  shared_ptr<TokenStream> tokenStream =
      analyzer->tokenStream(L"field", inputString);
  assertTokenStreamContents(tokenStream,
                            std::deque<wstring>{outputString, inputString});
}

TestSynonymGraphFilter::CustomAnalyzer::CustomAnalyzer(
    shared_ptr<SynonymMap> synonymMap)
{
  this->synonymMap = synonymMap;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymGraphFilter::CustomAnalyzer::createComponents(const wstring &s)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> tokenStream = make_shared<SynonymGraphFilter>(
      tokenizer, synonymMap, true); // Ignore case True
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenStream);
}
} // namespace org::apache::lucene::analysis::synonym