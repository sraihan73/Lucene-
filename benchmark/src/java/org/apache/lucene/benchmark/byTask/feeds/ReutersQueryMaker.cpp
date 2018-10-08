using namespace std;

#include "ReutersQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/WildcardQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/spans/SpanFirstQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/spans/SpanNearQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/spans/SpanTermQuery.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include "../tasks/NewAnalyzerTask.h"
#include "DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;
std::deque<wstring> ReutersQueryMaker::STANDARD_QUERIES = {
    L"Salomon",
    L"Comex",
    L"night trading",
    L"Japan Sony",
    L"\"Sony Japan\"",
    L"\"food needs\"~3",
    L"\"World Bank\"^2 AND Nigeria",
    L"\"World Bank\" -Nigeria",
    L"\"Ford Credit\"~5",
    L"airline Europe Canada destination",
    wstring(L"Long term pressure by trade ") +
        L"ministers is necessary if the current Uruguay round of talks on " +
        L"the General Agreement on Trade and Tariffs (GATT) is to " +
        L"succeed"};

std::deque<std::shared_ptr<Query>>
ReutersQueryMaker::getPrebuiltQueries(const wstring &field)
{
  //  be wary of unanalyzed text
  return std::deque<std::shared_ptr<Query>>{
      make_shared<SpanFirstQuery>(
          make_shared<SpanTermQuery>(make_shared<Term>(field, L"ford")), 5),
      make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              make_shared<SpanTermQuery>(make_shared<Term>(field, L"night")),
              make_shared<SpanTermQuery>(make_shared<Term>(field, L"trading"))},
          4, false),
      make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              make_shared<SpanFirstQuery>(
                  make_shared<SpanTermQuery>(make_shared<Term>(field, L"ford")),
                  10),
              make_shared<SpanTermQuery>(make_shared<Term>(field, L"credit"))},
          10, false),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"fo*"))};
}

std::deque<std::shared_ptr<Query>>
ReutersQueryMaker::createQueries(deque<any> &qs, shared_ptr<Analyzer> a)
{
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(DocMaker::BODY_FIELD, a);
  deque<any> queries = deque<any>();
  for (int i = 0; i < qs.size(); i++) {
    try {

      any query = qs[i];
      shared_ptr<Query> q = nullptr;
      if (dynamic_cast<wstring>(query) != nullptr) {
        q = qp->parse(any_cast<wstring>(query));

      } else if (std::dynamic_pointer_cast<Query>(query) != nullptr) {
        q = any_cast<std::shared_ptr<Query>>(query);

      } else {
        System::err::println(L"Unsupported Query Type: " + query);
      }

      if (q != nullptr) {
        queries.push_back(q);
      }

    } catch (const runtime_error &e) {
      e.printStackTrace();
    }
  }

  return queries.toArray(std::deque<std::shared_ptr<Query>>(0));
}

std::deque<std::shared_ptr<Query>>
ReutersQueryMaker::prepareQueries() 
{
  // analyzer (default is standard analyzer)
  shared_ptr<Analyzer> anlzr = NewAnalyzerTask::createAnalyzer(config->get(
      L"analyzer", L"org.apache.lucene.analysis.standard.StandardAnalyzer"));

  deque<any> queryList = deque<any>(20);
  queryList.addAll(Arrays::asList(STANDARD_QUERIES));
  queryList.addAll(Arrays::asList(getPrebuiltQueries(DocMaker::BODY_FIELD)));
  return createQueries(queryList, anlzr);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds