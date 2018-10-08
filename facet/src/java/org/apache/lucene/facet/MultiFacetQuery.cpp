using namespace std;

#include "MultiFacetQuery.h"

namespace org::apache::lucene::facet
{
using Query = org::apache::lucene::search::Query;
using TermInSetQuery = org::apache::lucene::search::TermInSetQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

MultiFacetQuery::MultiFacetQuery(shared_ptr<FacetsConfig> facetsConfig,
                                 const wstring &dimension,
                                 deque<wstring> &paths)
    : org::apache::lucene::search::TermInSetQuery(
          facetsConfig->getDimConfig(dimension)->indexFieldName,
          toTerms(dimension, paths))
{
}

MultiFacetQuery::MultiFacetQuery(const wstring &dimension,
                                 deque<wstring> &paths)
    : org::apache::lucene::search::TermInSetQuery(
          FacetsConfig::DEFAULT_DIM_CONFIG->indexFieldName,
          toTerms(dimension, paths))
{
}

shared_ptr<deque<std::shared_ptr<BytesRef>>>
MultiFacetQuery::toTerms(const wstring &dimension, deque<wstring> &paths)
{
  shared_ptr<deque<std::shared_ptr<BytesRef>>> *const terms =
      deque<std::shared_ptr<BytesRef>>(paths->length);
  for (wstring[] path : paths) {
    terms->add(
        make_shared<BytesRef>(FacetsConfig::pathToString(dimension, path)));
  }
  return terms;
}
} // namespace org::apache::lucene::facet