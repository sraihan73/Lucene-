using namespace std;

#include "SimpleQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include "../tasks/NewAnalyzerTask.h"
#include "DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;

std::deque<std::shared_ptr<Query>>
SimpleQueryMaker::prepareQueries() 
{
  // analyzer (default is standard analyzer)
  shared_ptr<Analyzer> anlzr = NewAnalyzerTask::createAnalyzer(config->get(
      L"analyzer", L"org.apache.lucene.analysis.standard.StandardAnalyzer"));

  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(DocMaker::BODY_FIELD, anlzr);
  deque<std::shared_ptr<Query>> qq = deque<std::shared_ptr<Query>>();
  shared_ptr<Query> q1 =
      make_shared<TermQuery>(make_shared<Term>(DocMaker::ID_FIELD, L"doc2"));
  qq.push_back(q1);
  shared_ptr<Query> q2 = make_shared<TermQuery>(
      make_shared<Term>(DocMaker::BODY_FIELD, L"simple"));
  qq.push_back(q2);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(q1, Occur::MUST);
  bq->add(q2, Occur::MUST);
  qq.push_back(bq->build());
  qq.push_back(qp->parse(L"synthetic body"));
  qq.push_back(qp->parse(L"\"synthetic body\""));
  qq.push_back(qp->parse(L"synthetic text"));
  qq.push_back(qp->parse(L"\"synthetic text\""));
  qq.push_back(qp->parse(L"\"synthetic text\"~3"));
  qq.push_back(qp->parse(L"zoom*"));
  qq.push_back(qp->parse(L"synth*"));
  return qq.toArray(std::deque<std::shared_ptr<Query>>(0));
}
} // namespace org::apache::lucene::benchmark::byTask::feeds