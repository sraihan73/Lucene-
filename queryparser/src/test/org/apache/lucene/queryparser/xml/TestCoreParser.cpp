using namespace std;

#include "TestCoreParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::xml::sax::SAXException;
const wstring TestCoreParser::defaultField = L"contents";
shared_ptr<org::apache::lucene::analysis::Analyzer> TestCoreParser::analyzer;
shared_ptr<CoreParser> TestCoreParser::coreParser;
shared_ptr<CoreParserTestIndexData> TestCoreParser::indexData;

shared_ptr<Analyzer> TestCoreParser::newAnalyzer()
{
  // TODO: rewrite test (this needs to set QueryParser.enablePositionIncrements,
  // too, for work with CURRENT):
  return make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                   MockTokenFilter::ENGLISH_STOPSET);
}

shared_ptr<CoreParser>
TestCoreParser::newCoreParser(const wstring &defaultField,
                              shared_ptr<Analyzer> analyzer)
{
  return make_shared<CoreParser>(defaultField, analyzer);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestCoreParser::afterClass() 
{
  if (indexData_ != nullptr) {
    delete indexData_;
    indexData_.reset();
  }
  coreParser_.reset();
  analyzer_.reset();
}

void TestCoreParser::testTermQueryXML() 
{
  shared_ptr<Query> q = parse(L"TermQuery.xml");
  dumpResults(L"TermQuery", q, 5);
}

void TestCoreParser::test_DOCTYPE_TermQueryXML() throw(ParserException,
                                                       IOException)
{
  shared_ptr<SAXException> saxe = LuceneTestCase::expectThrows(
      ParserException::typeid, SAXException::typeid,
      [&]() { parse(L"DOCTYPE_TermQuery.xml"); });
  assertTrue(saxe->getMessage()->startsWith(
      L"External Entity resolving unsupported:"));
}

void TestCoreParser::test_ENTITY_TermQueryXML() throw(ParserException,
                                                      IOException)
{
  shared_ptr<SAXException> saxe = LuceneTestCase::expectThrows(
      ParserException::typeid, SAXException::typeid,
      [&]() { parse(L"ENTITY_TermQuery.xml"); });
  assertTrue(saxe->getMessage()->startsWith(
      L"External Entity resolving unsupported:"));
}

void TestCoreParser::testTermQueryEmptyXML() 
{
  parseShouldFail(L"TermQueryEmpty.xml", L"TermQuery has no text");
}

void TestCoreParser::testTermsQueryXML() 
{
  shared_ptr<Query> q = parse(L"TermsQuery.xml");
  dumpResults(L"TermsQuery", q, 5);
}

void TestCoreParser::testBooleanQueryXML() 
{
  shared_ptr<Query> q = parse(L"BooleanQuery.xml");
  dumpResults(L"BooleanQuery", q, 5);
}

void TestCoreParser::testDisjunctionMaxQueryXML() throw(ParserException,
                                                        IOException)
{
  shared_ptr<Query> q = parse(L"DisjunctionMaxQuery.xml");
  assertTrue(std::dynamic_pointer_cast<DisjunctionMaxQuery>(q) != nullptr);
  shared_ptr<DisjunctionMaxQuery> d =
      std::static_pointer_cast<DisjunctionMaxQuery>(q);
  assertEquals(0.0f, d->getTieBreakerMultiplier(), 0.0001f);
  assertEquals(2, d->getDisjuncts().size());
  shared_ptr<DisjunctionMaxQuery> ndq =
      std::static_pointer_cast<DisjunctionMaxQuery>(d->getDisjuncts()[1]);
  assertEquals(1.2f, ndq->getTieBreakerMultiplier(), 0.0001f);
  assertEquals(1, ndq->getDisjuncts().size());
}

void TestCoreParser::testRangeQueryXML() 
{
  shared_ptr<Query> q = parse(L"RangeQuery.xml");
  dumpResults(L"RangeQuery", q, 5);
}

void TestCoreParser::testUserQueryXML() 
{
  shared_ptr<Query> q = parse(L"UserInputQuery.xml");
  dumpResults(L"UserInput with Filter", q, 5);
}

void TestCoreParser::testCustomFieldUserQueryXML() throw(ParserException,
                                                         IOException)
{
  shared_ptr<Query> q = parse(L"UserInputQueryCustomField.xml");
  int64_t h = searcher()->search(q, 1000)->totalHits;
  assertEquals(L"UserInputQueryCustomField should produce 0 result ", 0, h);
}

void TestCoreParser::testBoostingTermQueryXML() 
{
  shared_ptr<Query> q = parse(L"BoostingTermQuery.xml");
  dumpResults(L"BoostingTermQuery", q, 5);
}

void TestCoreParser::testSpanTermXML() 
{
  shared_ptr<Query> q = parse(L"SpanQuery.xml");
  dumpResults(L"Span Query", q, 5);
  shared_ptr<SpanQuery> sq = parseAsSpan(L"SpanQuery.xml");
  dumpResults(L"Span Query", sq, 5);
  assertEquals(q, sq);
}

void TestCoreParser::testConstantScoreQueryXML() 
{
  shared_ptr<Query> q = parse(L"ConstantScoreQuery.xml");
  dumpResults(L"ConstantScoreQuery", q, 5);
}

void TestCoreParser::testMatchAllDocsPlusFilterXML() throw(ParserException,
                                                           IOException)
{
  shared_ptr<Query> q = parse(L"MatchAllDocsQuery.xml");
  dumpResults(L"MatchAllDocsQuery with range filter", q, 5);
}

void TestCoreParser::testNestedBooleanQuery() throw(ParserException,
                                                    IOException)
{
  shared_ptr<Query> q = parse(L"NestedBooleanQuery.xml");
  dumpResults(L"Nested Boolean query", q, 5);
}

void TestCoreParser::testPointRangeQuery() 
{
  shared_ptr<Query> q = parse(L"PointRangeQuery.xml");
  dumpResults(L"PointRangeQuery", q, 5);
}

void TestCoreParser::testPointRangeQueryWithoutLowerTerm() throw(
    ParserException, IOException)
{
  shared_ptr<Query> q = parse(L"PointRangeQueryWithoutLowerTerm.xml");
  dumpResults(L"PointRangeQueryWithoutLowerTerm", q, 5);
}

void TestCoreParser::testPointRangeQueryWithoutUpperTerm() throw(
    ParserException, IOException)
{
  shared_ptr<Query> q = parse(L"PointRangeQueryWithoutUpperTerm.xml");
  dumpResults(L"PointRangeQueryWithoutUpperTerm", q, 5);
}

void TestCoreParser::testPointRangeQueryWithoutRange() throw(ParserException,
                                                             IOException)
{
  shared_ptr<Query> q = parse(L"PointRangeQueryWithoutRange.xml");
  dumpResults(L"PointRangeQueryWithoutRange", q, 5);
}

wstring TestCoreParser::defaultField() { return defaultField_; }

shared_ptr<Analyzer> TestCoreParser::analyzer()
{
  if (analyzer_ == nullptr) {
    analyzer_ = newAnalyzer();
  }
  return analyzer_;
}

shared_ptr<CoreParser> TestCoreParser::coreParser()
{
  if (coreParser_ == nullptr) {
    coreParser_ = newCoreParser(defaultField_, analyzer());
  }
  return coreParser_;
}

shared_ptr<CoreParserTestIndexData> TestCoreParser::indexData()
{
  if (indexData_ == nullptr) {
    try {
      indexData_ = make_shared<CoreParserTestIndexData>(analyzer());
    } catch (const runtime_error &e) {
      fail(L"caught Exception " + e);
    }
  }
  return indexData_;
}

shared_ptr<IndexReader> TestCoreParser::reader() { return indexData()->reader; }

shared_ptr<IndexSearcher> TestCoreParser::searcher()
{
  return indexData()->searcher;
}

void TestCoreParser::parseShouldFail(
    const wstring &xmlFileName,
    const wstring &expectedParserExceptionMessage) 
{
  shared_ptr<Query> q = nullptr;
  shared_ptr<ParserException> pe = nullptr;
  try {
    q = parse(xmlFileName);
  } catch (const ParserException &e) {
    pe = e;
  }
  assertNull(L"for " + xmlFileName + L" unexpectedly got " + q, q);
  assertNotNull(L"expected a ParserException for " + xmlFileName, pe);
  assertEquals(L"expected different ParserException for " + xmlFileName,
               expectedParserExceptionMessage, pe->what());
}

shared_ptr<Query>
TestCoreParser::parse(const wstring &xmlFileName) throw(ParserException,
                                                        IOException)
{
  return implParse(xmlFileName, false);
}

shared_ptr<SpanQuery>
TestCoreParser::parseAsSpan(const wstring &xmlFileName) throw(ParserException,
                                                              IOException)
{
  return std::static_pointer_cast<SpanQuery>(implParse(xmlFileName, true));
}

shared_ptr<Query> TestCoreParser::implParse(const wstring &xmlFileName,
                                            bool span) throw(ParserException,
                                                             IOException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream xmlStream =
  // TestCoreParser.class.getResourceAsStream(xmlFileName))
  {
    java::io::InputStream xmlStream =
        TestCoreParser::typeid->getResourceAsStream(xmlFileName);
    assertNotNull(L"Test XML file " + xmlFileName + L" cannot be found",
                  xmlStream);
    if (span) {
      return coreParser()->parseAsSpanQuery(xmlStream);
    } else {
      return coreParser()->parse(xmlStream);
    }
  }
}

shared_ptr<Query>
TestCoreParser::rewrite(shared_ptr<Query> q) 
{
  return q->rewrite(reader());
}

void TestCoreParser::dumpResults(const wstring &qType, shared_ptr<Query> q,
                                 int numDocs) 
{
  if (VERBOSE) {
    wcout << L"TEST: qType=" << qType << L" numDocs=" << numDocs << L" "
          << q->getClass().getCanonicalName() << L" query=" << q << endl;
  }
  shared_ptr<IndexSearcher> *const searcher = this->searcher();
  shared_ptr<TopDocs> hits = searcher->search(q, numDocs);
  constexpr bool producedResults = (hits->totalHits > 0);
  if (!producedResults) {
    wcout << L"TEST: qType=" << qType << L" numDocs=" << numDocs << L" "
          << q->getClass().getCanonicalName() << L" query=" << q << endl;
  }
  if (VERBOSE) {
    std::deque<std::shared_ptr<ScoreDoc>> scoreDocs = hits->scoreDocs;
    for (int i = 0; i < min(numDocs, hits->totalHits); i++) {
      shared_ptr<Document> ldoc = searcher->doc(scoreDocs[i]->doc);
      wcout << L"[" << ldoc[L"date"] << L"]" << ldoc[L"contents"] << endl;
    }
    wcout << endl;
  }
  assertTrue(qType + L" produced no results", producedResults);
}
} // namespace org::apache::lucene::queryparser::xml