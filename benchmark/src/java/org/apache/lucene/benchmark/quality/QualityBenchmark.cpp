using namespace std;

#include "QualityBenchmark.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "Judge.h"
#include "QualityQuery.h"
#include "QualityQueryParser.h"
#include "QualityStats.h"
#include "utils/DocNameExtractor.h"
#include "utils/SubmissionReport.h"

namespace org::apache::lucene::benchmark::quality
{
using DocNameExtractor =
    org::apache::lucene::benchmark::quality::utils::DocNameExtractor;
using SubmissionReport =
    org::apache::lucene::benchmark::quality::utils::SubmissionReport;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;

QualityBenchmark::QualityBenchmark(
    std::deque<std::shared_ptr<QualityQuery>> &qqs,
    shared_ptr<QualityQueryParser> qqParser, shared_ptr<IndexSearcher> searcher,
    const wstring &docNameField)
{
  this->qualityQueries = qqs;
  this->qqParser = qqParser;
  this->searcher = searcher;
  this->docNameField = docNameField;
}

std::deque<std::shared_ptr<QualityStats>> QualityBenchmark::execute(
    shared_ptr<Judge> judge, shared_ptr<SubmissionReport> submitRep,
    shared_ptr<PrintWriter> qualityLog) 
{
  int nQueries = min(maxQueries, qualityQueries.size());
  std::deque<std::shared_ptr<QualityStats>> stats(nQueries);
  for (int i = 0; i < nQueries; i++) {
    shared_ptr<QualityQuery> qq = qualityQueries[i];
    // generate query
    shared_ptr<Query> q = qqParser->parse(qq);
    // search with this query
    int64_t t1 = System::currentTimeMillis();
    shared_ptr<TopDocs> td = searcher->search(q, maxResults);
    int64_t searchTime = System::currentTimeMillis() - t1;
    // most likely we either submit or judge, but check both
    if (judge != nullptr) {
      stats[i] = analyzeQueryResults(qq, q, td, judge, qualityLog, searchTime);
    }
    if (submitRep != nullptr) {
      submitRep->report(qq, td, docNameField, searcher);
    }
  }
  if (submitRep != nullptr) {
    submitRep->flush();
  }
  return stats;
}

shared_ptr<QualityStats> QualityBenchmark::analyzeQueryResults(
    shared_ptr<QualityQuery> qq, shared_ptr<Query> q, shared_ptr<TopDocs> td,
    shared_ptr<Judge> judge, shared_ptr<PrintWriter> logger,
    int64_t searchTime) 
{
  shared_ptr<QualityStats> stts =
      make_shared<QualityStats>(judge->maxRecall(qq), searchTime);
  std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
  int64_t t1 =
      System::currentTimeMillis(); // extraction of first doc name we measure
                                   // also construction of doc name extractor,
                                   // just in case.
  shared_ptr<DocNameExtractor> xt = make_shared<DocNameExtractor>(docNameField);
  for (int i = 0; i < sd.size(); i++) {
    wstring docName = xt->docName(searcher, sd[i]->doc);
    int64_t docNameExtractTime = System::currentTimeMillis() - t1;
    t1 = System::currentTimeMillis();
    bool isRelevant = judge->isRelevant(docName, qq);
    stts->addResult(i + 1, isRelevant, docNameExtractTime);
  }
  if (logger != nullptr) {
    logger->println(qq->getQueryID() + L"  -  " + q);
    stts->log(qq->getQueryID() + L" Stats:", 1, logger, L"  ");
  }
  return stts;
}

int QualityBenchmark::getMaxQueries() { return maxQueries; }

void QualityBenchmark::setMaxQueries(int maxQueries)
{
  this->maxQueries = maxQueries;
}

int QualityBenchmark::getMaxResults() { return maxResults; }

void QualityBenchmark::setMaxResults(int maxResults)
{
  this->maxResults = maxResults;
}
} // namespace org::apache::lucene::benchmark::quality