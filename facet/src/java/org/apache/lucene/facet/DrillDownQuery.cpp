using namespace std;

#include "DrillDownQuery.h"

namespace org::apache::lucene::facet
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

shared_ptr<Term> DrillDownQuery::term(const wstring &field, const wstring &dim,
                                      deque<wstring> &path)
{
  return make_shared<Term>(field, FacetsConfig::pathToString(dim, path));
}

DrillDownQuery::DrillDownQuery(
    shared_ptr<FacetsConfig> config, shared_ptr<Query> baseQuery,
    deque<std::shared_ptr<BooleanQuery::Builder>> &dimQueries,
    unordered_map<wstring, int> &drillDownDims)
    : config(config), baseQuery(baseQuery)
{
  this->dimQueries.insert(this->dimQueries.end(), dimQueries.begin(),
                          dimQueries.end());
  this->drillDownDims.putAll(drillDownDims);
}

DrillDownQuery::DrillDownQuery(shared_ptr<FacetsConfig> config,
                               shared_ptr<Query> filter,
                               shared_ptr<DrillDownQuery> other)
    : config(config), baseQuery((make_shared<BooleanQuery::Builder>())
                                    ->add(other->baseQuery == nullptr
                                              ? make_shared<MatchAllDocsQuery>()
                                              : other->baseQuery,
                                          Occur::MUST)
                                    ->add(filter, Occur::FILTER)
                                    ->build())
{
  this->dimQueries.insert(this->dimQueries.end(), other->dimQueries.begin(),
                          other->dimQueries.end());
  this->drillDownDims.putAll(other->drillDownDims);
}

DrillDownQuery::DrillDownQuery(shared_ptr<FacetsConfig> config)
    : DrillDownQuery(config, nullptr)
{
}

DrillDownQuery::DrillDownQuery(shared_ptr<FacetsConfig> config,
                               shared_ptr<Query> baseQuery)
    : config(config), baseQuery(baseQuery)
{
}

void DrillDownQuery::add(const wstring &dim, deque<wstring> &path)
{
  wstring indexedField = config->getDimConfig(dim)->indexFieldName;
  add(dim, make_shared<TermQuery>(term(indexedField, dim, {path})));
}

void DrillDownQuery::add(const wstring &dim, shared_ptr<Query> subQuery)
{
  assert(drillDownDims.size() == dimQueries.size());
  if (drillDownDims.find(dim) != drillDownDims.end() == false) {
    drillDownDims.emplace(dim, drillDownDims.size());
    shared_ptr<BooleanQuery::Builder> builder =
        make_shared<BooleanQuery::Builder>();
    dimQueries.push_back(builder);
  }
  constexpr int index = drillDownDims[dim];
  dimQueries[index]->add(subQuery, Occur::SHOULD);
}

shared_ptr<DrillDownQuery> DrillDownQuery::clone()
{
  return make_shared<DrillDownQuery>(config, baseQuery, dimQueries,
                                     drillDownDims);
}

int DrillDownQuery::hashCode()
{
  return classHash() + Objects::hash(baseQuery, dimQueries);
}

bool DrillDownQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DrillDownQuery::equalsTo(shared_ptr<DrillDownQuery> other)
{
  return Objects::equals(baseQuery, other->baseQuery) &&
         dimQueries.equals(other->dimQueries);
}

shared_ptr<Query>
DrillDownQuery::rewrite(shared_ptr<IndexReader> r) 
{
  shared_ptr<BooleanQuery> rewritten = getBooleanQuery();
  if (rewritten->clauses().empty()) {
    return make_shared<MatchAllDocsQuery>();
  }
  return rewritten;
}

wstring DrillDownQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getBooleanQuery()->toString(field);
}

shared_ptr<BooleanQuery> DrillDownQuery::getBooleanQuery()
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  if (baseQuery != nullptr) {
    bq->add(baseQuery, Occur::MUST);
  }
  for (auto builder : dimQueries) {
    bq->add(builder->build(), Occur::FILTER);
  }
  return bq->build();
}

shared_ptr<Query> DrillDownQuery::getBaseQuery() { return baseQuery; }

std::deque<std::shared_ptr<Query>> DrillDownQuery::getDrillDownQueries()
{
  std::deque<std::shared_ptr<Query>> dimQueries(this->dimQueries.size());
  for (int i = 0; i < dimQueries.size(); ++i) {
    dimQueries[i] = this->dimQueries[i]->build();
  }
  return dimQueries;
}

unordered_map<wstring, int> DrillDownQuery::getDims() { return drillDownDims; }
} // namespace org::apache::lucene::facet