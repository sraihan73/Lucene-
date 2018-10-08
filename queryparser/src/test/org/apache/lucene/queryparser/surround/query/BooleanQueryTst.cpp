using namespace std;

#include "BooleanQueryTst.h"

namespace org::apache::lucene::queryparser::surround::query
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Scorer = org::apache::lucene::search::Scorer;
using Query = org::apache::lucene::search::Query;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using QueryParser =
    org::apache::lucene::queryparser::surround::parser::QueryParser;
using org::junit::Assert;

BooleanQueryTst::BooleanQueryTst(const wstring &queryText,
                                 std::deque<int> &expectedDocNrs,
                                 shared_ptr<SingleFieldTestDb> dBase,
                                 const wstring &fieldName,
                                 shared_ptr<Assert> testCase,
                                 shared_ptr<BasicQueryFactory> qf)
    : expectedDocNrs(expectedDocNrs)
{
  this->queryText = queryText;
  this->dBase = dBase;
  this->fieldName = fieldName;
  this->testCase = testCase;
  this->qf = qf;
}

void BooleanQueryTst::setVerbose(bool verbose) { this->verbose = verbose; }

BooleanQueryTst::TestCollector::TestCollector(
    shared_ptr<BooleanQueryTst> outerInstance)
    : outerInstance(outerInstance)
{
  totalMatched = 0;
  encountered = std::deque<bool>(outerInstance->expectedDocNrs.size());
}

void BooleanQueryTst::TestCollector::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
  this->scorer = scorer;
}

void BooleanQueryTst::TestCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void BooleanQueryTst::TestCollector::collect(int docNr) 
{
  float score = scorer->score();
  docNr += docBase;
  /* System.out.println(docNr + " '" + dBase.getDocs()[docNr] + "': " + score);
   */
  Assert::assertTrue(outerInstance->queryText + L": positive score",
                     score > 0.0);
  Assert::assertTrue(outerInstance->queryText + L": too many hits",
                     totalMatched < outerInstance->expectedDocNrs.size());
  int i;
  for (i = 0; i < outerInstance->expectedDocNrs.size(); i++) {
    if ((!encountered[i]) && (outerInstance->expectedDocNrs[i] == docNr)) {
      encountered[i] = true;
      break;
    }
  }
  if (i == outerInstance->expectedDocNrs.size()) {
    Assert::assertTrue(outerInstance->queryText +
                           L": doc nr for hit not expected: " +
                           to_wstring(docNr),
                       false);
  }
  totalMatched++;
}

bool BooleanQueryTst::TestCollector::needsScores() { return true; }

void BooleanQueryTst::TestCollector::checkNrHits()
{
  Assert::assertEquals(outerInstance->queryText + L": nr of hits",
                       outerInstance->expectedDocNrs.size(), totalMatched);
}

void BooleanQueryTst::doTest() 
{

  if (verbose) {
    wcout << L"" << endl;
    wcout << L"Query: " << queryText << endl;
  }

  shared_ptr<SrndQuery> lq = QueryParser::parse(queryText);

  /* if (verbose) System.out.println("Srnd: " + lq.toString()); */

  shared_ptr<Query> query = lq->makeLuceneQueryField(fieldName, qf);
  /* if (verbose) System.out.println("Lucene: " + query.toString()); */

  shared_ptr<TestCollector> tc = make_shared<TestCollector>(shared_from_this());
  shared_ptr<IndexReader> reader = DirectoryReader::open(dBase->getDb());
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  try {
    searcher->search(query, tc);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
  }
  tc->checkNrHits();
}
} // namespace org::apache::lucene::queryparser::surround::query