using namespace std;

#include "TestSolrSynonymParser.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/en/EnglishAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SolrSynonymParser.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using EnglishAnalyzer = org::apache::lucene::analysis::en::EnglishAnalyzer;

void TestSolrSynonymParser::testSimple() 
{
  wstring testFile = wstring(L"i-pod, ipod, ipoooood\n") + L"foo => foo bar\n" +
                     L"foo => baz\n" + L"this test, that testing";

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(testFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete analyzer;

  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj);

  assertAnalyzesTo(analyzer, L"ball", std::deque<wstring>{L"ball"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"i-pod",
                   std::deque<wstring>{L"i-pod", L"ipod", L"ipoooood"},
                   std::deque<int>{1, 0, 0});

  assertAnalyzesTo(analyzer, L"foo",
                   std::deque<wstring>{L"foo", L"baz", L"bar"},
                   std::deque<int>{1, 0, 1});

  assertAnalyzesTo(analyzer, L"this test",
                   std::deque<wstring>{L"this", L"that", L"test", L"testing"},
                   std::deque<int>{1, 0, 1, 0});
  delete analyzer;
}

TestSolrSynonymParser::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSolrSynonymParser> outerInstance,
    shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSolrSynonymParser::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSolrSynonymParser::testInvalidDoubleMap() 
{
  wstring testFile = L"a => b => c";
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(make_shared<StringReader>(testFile)); });
  delete analyzer;
}

void TestSolrSynonymParser::testInvalidAnalyzesToNothingOutput() throw(
    runtime_error)
{
  wstring testFile = L"a => 1";
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, false);
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(make_shared<StringReader>(testFile)); });
  delete analyzer;
}

void TestSolrSynonymParser::testInvalidAnalyzesToNothingInput() throw(
    runtime_error)
{
  wstring testFile = L"1 => a";
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, false);
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(make_shared<StringReader>(testFile)); });
  delete analyzer;
}

void TestSolrSynonymParser::testInvalidPositionsInput() 
{
  wstring testFile = L"testola => the test";
  shared_ptr<Analyzer> analyzer = make_shared<EnglishAnalyzer>();
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(make_shared<StringReader>(testFile)); });
  delete analyzer;
}

void TestSolrSynonymParser::testInvalidPositionsOutput() 
{
  wstring testFile = L"the test => testola";
  shared_ptr<Analyzer> analyzer = make_shared<EnglishAnalyzer>();
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(make_shared<StringReader>(testFile)); });
  delete analyzer;
}

void TestSolrSynonymParser::testEscapedStuff() 
{
  wstring testFile = wstring(L"a\\=>a => b\\=>b\n") + L"a\\,a => b\\,b";
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(testFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete analyzer;
  analyzer = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), map_obj);

  assertAnalyzesTo(analyzer, L"ball", std::deque<wstring>{L"ball"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"a=>a", std::deque<wstring>{L"b=>b"},
                   std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"a,a", std::deque<wstring>{L"b,b"},
                   std::deque<int>{1});
  delete analyzer;
}

TestSolrSynonymParser::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSolrSynonymParser> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSolrSynonymParser::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, false));
}

void TestSolrSynonymParser::testPositionLengthAndTypeSimple() throw(
    runtime_error)
{
  wstring testFile = L"spider man, spiderman";

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(testFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete analyzer;

  analyzer = make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), map_obj);

  assertAnalyzesToPositions(
      analyzer, L"spider man",
      std::deque<wstring>{L"spider", L"spiderman", L"man"},
      std::deque<wstring>{L"word", L"SYNONYM", L"word"},
      std::deque<int>{1, 0, 1}, std::deque<int>{1, 2, 1});
}

TestSolrSynonymParser::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestSolrSynonymParser> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSolrSynonymParser::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSolrSynonymParser::testParseSimple() 
{
  wstring testFile = wstring(L"spider man, spiderman\n") +
                     L"usa,united states,u s a,united states of america\n" +
                     L"mystyped, mistyped => mistyped\n" + L"foo => foo bar\n" +
                     L"foo => baz";

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(testFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete analyzer;

  assertEntryEquals(map_obj, L"spiderman", true, L"spider man");
  assertEntryEquals(map_obj, L"spider man", true, L"spiderman");

  assertEntryEquals(map_obj, L"usa", true,
                    std::deque<wstring>{L"united states", L"u s a",
                                         L"united states of america"});
  assertEntryEquals(
      map_obj, L"united states", true,
      std::deque<wstring>{L"usa", L"u s a", L"united states of america"});
  assertEntryEquals(map_obj, L"u s a", true,
                    std::deque<wstring>{L"usa", L"united states",
                                         L"united states of america"});
  assertEntryEquals(map_obj, L"united states of america", true,
                    std::deque<wstring>{L"usa", L"u s a", L"united states"});

  assertEntryEquals(map_obj, L"mistyped", false, L"mistyped");
  assertEntryEquals(map_obj, L"mystyped", false, L"mistyped");

  assertEntryEquals(map_obj, L"foo", false,
                    std::deque<wstring>{L"foo bar", L"baz"});
  assertEntryAbsent(map_obj, L"baz");
  assertEntryAbsent(map_obj, L"bar");
}
} // namespace org::apache::lucene::analysis::synonym