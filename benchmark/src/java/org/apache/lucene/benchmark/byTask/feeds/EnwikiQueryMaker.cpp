using namespace std;

#include "EnwikiQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/MultiTermQuery.h"
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
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Term = org::apache::lucene::index::Term;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;
std::deque<wstring> EnwikiQueryMaker::STANDARD_QUERIES = {
    L"Images catbox gif",
    L"Imunisasi haram",
    L"Favicon ico",
    L"Michael jackson",
    L"Unknown artist",
    L"Lily Thai",
    L"Neda",
    L"The Last Song",
    L"Metallica",
    L"Nicola Tesla",
    L"Max B",
    L"Skil Corporation",
    L"\"The 100 Greatest Artists of All Time\"",
    L"\"Top 100 Global Universities\"",
    L"Pink floyd",
    L"Bolton Sullivan",
    L"Frank Lucas Jr",
    L"Drake Woods",
    L"Radiohead",
    L"George Freeman",
    L"Oksana Grigorieva",
    L"The Elder Scrolls V",
    L"Deadpool",
    L"Green day",
    L"\"Red hot chili peppers\"",
    L"Jennifer Bini Taylor",
    L"The Paradiso Girls",
    L"Queen",
    L"3Me4Ph",
    L"Paloma Jimenez",
    L"AUDI A4",
    L"Edith Bouvier Beale: A Life In Pictures",
    L"\"Skylar James Deleon\"",
    L"Simple Explanation",
    L"Juxtaposition",
    L"The Woody Show",
    L"London WITHER",
    L"In A Dark Place",
    L"George Freeman",
    L"LuAnn de Lesseps",
    L"Muhammad.",
    L"U2",
    L"List of countries by GDP",
    L"Dean Martin Discography",
    L"Web 3.0",
    L"List of American actors",
    L"The Expendables",
    L"\"100 Greatest Guitarists of All Time\"",
    L"Vince Offer.",
    L"\"List of ZIP Codes in the United States\"",
    L"Blood type diet",
    L"Jennifer Gimenez",
    L"List of hobbies",
    L"The beatles",
    L"Acdc",
    L"Nightwish",
    L"Iron maiden",
    L"Murder Was the Case",
    L"Pelvic hernia",
    L"Naruto Shippuuden",
    L"campaign",
    L"Enthesopathy of hip region",
    L"operating system",
    L"mouse",
    L"List of Xbox 360 games without region encoding",
    L"Shakepearian sonnet",
    L"\"The Monday Night Miracle\"",
    L"India",
    L"Dad's Army",
    L"Solanum melanocerasum",
    L"\"List of PlayStation Portable Wi-Fi games\"",
    L"Little Pixie Geldof",
    L"Planes, Trains & Automobiles",
    L"Freddy Ingalls",
    L"The Return of Chef",
    L"Nehalem",
    L"Turtle",
    L"Calculus",
    L"Superman-Prime",
    L"\"The Losers\"",
    L"pen-pal",
    L"Audio stream input output",
    L"lifehouse",
    L"50 greatest gunners",
    L"Polyfecalia",
    L"freeloader",
    L"The Filthy Youth"};

std::deque<std::shared_ptr<Query>>
EnwikiQueryMaker::getPrebuiltQueries(const wstring &field)
{
  shared_ptr<WildcardQuery> wcq =
      make_shared<WildcardQuery>(make_shared<Term>(field, L"fo*"));
  wcq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  // be wary of unanalyzed text
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
      wcq};
}

std::deque<std::shared_ptr<Query>>
EnwikiQueryMaker::createQueries(deque<any> &qs, shared_ptr<Analyzer> a)
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
EnwikiQueryMaker::prepareQueries() 
{
  // analyzer (default is standard analyzer)
  shared_ptr<Analyzer> anlzr = NewAnalyzerTask::createAnalyzer(
      config->get(L"analyzer", StandardAnalyzer::typeid->getName()));

  deque<any> queryList = deque<any>(20);
  queryList.addAll(Arrays::asList(STANDARD_QUERIES));
  if (!config->get(L"enwikiQueryMaker.disableSpanQueries", false)) {
    queryList.addAll(Arrays::asList(getPrebuiltQueries(DocMaker::BODY_FIELD)));
  }
  return createQueries(queryList, anlzr);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds