using namespace std;

#include "FacetQuery.h"

namespace org::apache::lucene::facet
{
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

FacetQuery::FacetQuery(shared_ptr<FacetsConfig> facetsConfig,
                       const wstring &dimension, deque<wstring> &path)
    : org::apache::lucene::search::TermQuery(
          toTerm(facetsConfig->getDimConfig(dimension), dimension, path))
{
}

FacetQuery::FacetQuery(const wstring &dimension, deque<wstring> &path)
    : org::apache::lucene::search::TermQuery(
          toTerm(FacetsConfig::DEFAULT_DIM_CONFIG, dimension, path))
{
}

shared_ptr<Term>
FacetQuery::toTerm(shared_ptr<FacetsConfig::DimConfig> dimConfig,
                   const wstring &dimension, deque<wstring> &path)
{
  return make_shared<Term>(dimConfig->indexFieldName,
                           FacetsConfig::pathToString(dimension, path));
}
} // namespace org::apache::lucene::facet