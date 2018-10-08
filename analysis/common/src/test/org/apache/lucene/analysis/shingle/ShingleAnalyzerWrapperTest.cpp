using namespace std;

#include "ShingleAnalyzerWrapperTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleAnalyzerWrapper.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;

void ShingleAnalyzerWrapperTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false), 2);
  directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, make_shared<IndexWriterConfig>(analyzer));

  shared_ptr<Document> doc;
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"please divide this sentence into shingles",
      Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"just another test sentence", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"a sentence which contains no test", Field::Store::YES));
  writer->addDocument(doc);

  delete writer;

  reader = DirectoryReader::open(directory);
  searcher = newSearcher(reader);
}

void ShingleAnalyzerWrapperTest::tearDown() 
{
  delete reader;
  delete directory;
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void ShingleAnalyzerWrapperTest::compareRanks(
    std::deque<std::shared_ptr<ScoreDoc>> &hits,
    std::deque<int> &ranks) 
{
  assertEquals(ranks.size(), hits.size());
  for (int i = 0; i < ranks.size(); i++) {
    assertEquals(ranks[i], hits[i]->doc);
  }
}

void ShingleAnalyzerWrapperTest::testShingleAnalyzerWrapperPhraseQuery() throw(
    runtime_error)
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("content", "this sentence"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"content", L"this sentence");
    int j = -1;

    shared_ptr<PositionIncrementAttribute> posIncrAtt =
        ts->addAttribute(PositionIncrementAttribute::typeid);
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);

    ts->reset();
    while (ts->incrementToken()) {
      j += posIncrAtt->getPositionIncrement();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring termText = termAtt->toString();
      builder->add(make_shared<Term>(L"content", termText), j);
    }
    ts->end();
  }

  shared_ptr<PhraseQuery> q = builder->build();
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q, 1000)->scoreDocs;
  std::deque<int> ranks = {0};
  compareRanks(hits, ranks);
}

void ShingleAnalyzerWrapperTest::testShingleAnalyzerWrapperBooleanQuery() throw(
    runtime_error)
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("content", "test sentence"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"content", L"test sentence");
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);

    ts->reset();
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring termText = termAtt->toString();
      q->add(make_shared<TermQuery>(make_shared<Term>(L"content", termText)),
             BooleanClause::Occur::SHOULD);
    }
    ts->end();
  }

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q->build(), 1000)->scoreDocs;
  std::deque<int> ranks = {1, 2, 0};
  compareRanks(hits, ranks);
}

void ShingleAnalyzerWrapperTest::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false), 2);
  assertAnalyzesTo(a, L"please divide into shingles",
                   std::deque<wstring>{L"please", L"please divide", L"divide",
                                        L"divide into", L"into",
                                        L"into shingles", L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19},
                   std::deque<int>{6, 13, 13, 18, 18, 27, 27},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  assertAnalyzesTo(a, L"divide me up again",
                   std::deque<wstring>{L"divide", L"divide me", L"me",
                                        L"me up", L"up", L"up again", L"again"},
                   std::deque<int>{0, 0, 7, 7, 10, 10, 13},
                   std::deque<int>{6, 9, 9, 12, 12, 18, 18},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete a;
}

void ShingleAnalyzerWrapperTest::testNonDefaultMinShingleSize() throw(
    runtime_error)
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          3, 4);
  assertAnalyzesTo(
      analyzer, L"please divide this sentence into shingles",
      std::deque<wstring>{
          L"please", L"please divide this", L"please divide this sentence",
          L"divide", L"divide this sentence", L"divide this sentence into",
          L"this", L"this sentence into", L"this sentence into shingles",
          L"sentence", L"sentence into shingles", L"into", L"shingles"},
      std::deque<int>{0, 0, 0, 7, 7, 7, 14, 14, 14, 19, 19, 28, 33},
      std::deque<int>{6, 18, 27, 13, 27, 32, 18, 32, 41, 27, 41, 32, 41},
      std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1});
  delete analyzer;

  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false), 3,
      4, ShingleFilter::DEFAULT_TOKEN_SEPARATOR, false, false,
      ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please divide this sentence into shingles",
                   std::deque<wstring>{
                       L"please divide this", L"please divide this sentence",
                       L"divide this sentence", L"divide this sentence into",
                       L"this sentence into", L"this sentence into shingles",
                       L"sentence into shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19},
                   std::deque<int>{18, 27, 27, 32, 32, 41, 41},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete analyzer;
}

void ShingleAnalyzerWrapperTest::testNonDefaultMinAndSameMaxShingleSize() throw(
    runtime_error)
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          3, 3);
  assertAnalyzesTo(analyzer, L"please divide this sentence into shingles",
                   std::deque<wstring>{L"please", L"please divide this",
                                        L"divide", L"divide this sentence",
                                        L"this", L"this sentence into",
                                        L"sentence", L"sentence into shingles",
                                        L"into", L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19, 19, 28, 33},
                   std::deque<int>{6, 18, 13, 27, 18, 32, 27, 41, 32, 41},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 1});
  delete analyzer;

  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false), 3,
      3, ShingleFilter::DEFAULT_TOKEN_SEPARATOR, false, false,
      ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(
      analyzer, L"please divide this sentence into shingles",
      std::deque<wstring>{L"please divide this", L"divide this sentence",
                           L"this sentence into", L"sentence into shingles"},
      std::deque<int>{0, 7, 14, 19}, std::deque<int>{18, 27, 32, 41},
      std::deque<int>{1, 1, 1, 1});
  delete analyzer;
}

void ShingleAnalyzerWrapperTest::testNoTokenSeparator() 
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"", true, false,
          ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please divide into shingles",
                   std::deque<wstring>{L"please", L"pleasedivide", L"divide",
                                        L"divideinto", L"into", L"intoshingles",
                                        L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19},
                   std::deque<int>{6, 13, 13, 18, 18, 27, 27},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete analyzer;

  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"", false, false,
      ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(
      analyzer, L"please divide into shingles",
      std::deque<wstring>{L"pleasedivide", L"divideinto", L"intoshingles"},
      std::deque<int>{0, 7, 14}, std::deque<int>{13, 18, 27},
      std::deque<int>{1, 1, 1});
  delete analyzer;
}

void ShingleAnalyzerWrapperTest::testNullTokenSeparator() 
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, nullptr, true, false,
          ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please divide into shingles",
                   std::deque<wstring>{L"please", L"pleasedivide", L"divide",
                                        L"divideinto", L"into", L"intoshingles",
                                        L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19},
                   std::deque<int>{6, 13, 13, 18, 18, 27, 27},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete analyzer;

  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"", false, false,
      ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(
      analyzer, L"please divide into shingles",
      std::deque<wstring>{L"pleasedivide", L"divideinto", L"intoshingles"},
      std::deque<int>{0, 7, 14}, std::deque<int>{13, 18, 27},
      std::deque<int>{1, 1, 1});
  delete analyzer;
}

void ShingleAnalyzerWrapperTest::testAltTokenSeparator() 
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"<SEP>", true, false,
          ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please divide into shingles",
                   std::deque<wstring>{L"please", L"please<SEP>divide",
                                        L"divide", L"divide<SEP>into", L"into",
                                        L"into<SEP>shingles", L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 14, 14, 19},
                   std::deque<int>{6, 13, 13, 18, 18, 27, 27},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete analyzer;

  analyzer = make_shared<ShingleAnalyzerWrapper>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"<SEP>", false, false,
      ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please divide into shingles",
                   std::deque<wstring>{L"please<SEP>divide",
                                        L"divide<SEP>into",
                                        L"into<SEP>shingles"},
                   std::deque<int>{0, 7, 14}, std::deque<int>{13, 18, 27},
                   std::deque<int>{1, 1, 1});
  delete analyzer;
}

void ShingleAnalyzerWrapperTest::testAltFillerToken() 
{
  shared_ptr<Analyzer> delegate_ =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          delegate_, ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_TOKEN_SEPARATOR, true, false, L"--");
  assertAnalyzesTo(analyzer, L"please divide into shingles",
                   std::deque<wstring>{L"please", L"please divide", L"divide",
                                        L"divide --", L"-- shingles",
                                        L"shingles"},
                   std::deque<int>{0, 0, 7, 7, 19, 19},
                   std::deque<int>{6, 13, 13, 19, 27, 27},
                   std::deque<int>{1, 0, 1, 0, 1, 1});
  delete analyzer;

  delegate_ = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  analyzer = make_shared<ShingleAnalyzerWrapper>(
      delegate_, ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_TOKEN_SEPARATOR, false, false, nullptr);
  assertAnalyzesTo(
      analyzer, L"please divide into shingles",
      std::deque<wstring>{L"please divide", L"divide ", L" shingles"},
      std::deque<int>{0, 7, 19}, std::deque<int>{13, 19, 27},
      std::deque<int>{1, 1, 1});
  delete analyzer;

  delegate_ = make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  analyzer = make_shared<ShingleAnalyzerWrapper>(
      delegate_, ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE,
      ShingleFilter::DEFAULT_TOKEN_SEPARATOR, false, false, L"");
  assertAnalyzesTo(
      analyzer, L"please divide into shingles",
      std::deque<wstring>{L"please divide", L"divide ", L" shingles"},
      std::deque<int>{0, 7, 19}, std::deque<int>{13, 19, 27},
      std::deque<int>{1, 1, 1});
  delete analyzer;
}

ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<ShingleAnalyzerWrapperTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"into"});
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter = make_shared<StopFilter>(tokenizer, stopSet);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<ShingleAnalyzerWrapperTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"into"});
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter = make_shared<StopFilter>(tokenizer, stopSet);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<ShingleAnalyzerWrapperTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleAnalyzerWrapperTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"into"});
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter = make_shared<StopFilter>(tokenizer, stopSet);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void ShingleAnalyzerWrapperTest::
    testOutputUnigramsIfNoShinglesSingleToken() 
{
  shared_ptr<ShingleAnalyzerWrapper> analyzer =
      make_shared<ShingleAnalyzerWrapper>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
          ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE, L"", false, true,
          ShingleFilter::DEFAULT_FILLER_TOKEN);
  assertAnalyzesTo(analyzer, L"please", std::deque<wstring>{L"please"},
                   std::deque<int>{0}, std::deque<int>{6},
                   std::deque<int>{1});
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::shingle