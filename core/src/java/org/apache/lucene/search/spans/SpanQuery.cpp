using namespace std;

#include "SpanQuery.h"

namespace org::apache::lucene::search::spans
{
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
SpanQuery::getTermContexts(deque<SpanWeight> &weights)
{
  unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> terms =
      map_obj<std::shared_ptr<Term>, std::shared_ptr<TermContext>>();
  for (shared_ptr<SpanWeight> w : weights) {
    w->extractTermContexts(terms);
  }
  return terms;
}

unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
SpanQuery::getTermContexts(
    shared_ptr<deque<std::shared_ptr<SpanWeight>>> weights)
{
  unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> terms =
      map_obj<std::shared_ptr<Term>, std::shared_ptr<TermContext>>();
  for (auto w : weights) {
    w->extractTermContexts(terms);
  }
  return terms;
}
} // namespace org::apache::lucene::search::spans