using namespace std;

#include "RangeFacetCounts.h"

namespace org::apache::lucene::facet::range
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using Query = org::apache::lucene::search::Query;

RangeFacetCounts::RangeFacetCounts(
    const wstring &field, std::deque<std::shared_ptr<Range>> &ranges,
    shared_ptr<Query> fastMatchQuery) 
    : ranges(ranges), counts(std::deque<int>(ranges.size())),
      fastMatchQuery(fastMatchQuery), field(field)
{
}

shared_ptr<FacetResult> RangeFacetCounts::getTopChildren(int topN,
                                                         const wstring &dim,
                                                         deque<wstring> &path)
{
  if (dim == field == false) {
    throw invalid_argument(L"invalid dim \"" + dim + L"\"; should be \"" +
                           field + L"\"");
  }
  if (path->length != 0) {
    throw invalid_argument(L"path.length should be 0");
  }
  std::deque<std::shared_ptr<LabelAndValue>> labelValues(counts.size());
  for (int i = 0; i < counts.size(); i++) {
    labelValues[i] = make_shared<LabelAndValue>(ranges[i]->label, counts[i]);
  }
  return make_shared<FacetResult>(dim, path, totCount, labelValues,
                                  labelValues.size());
}

shared_ptr<Number>
RangeFacetCounts::getSpecificValue(const wstring &dim,
                                   deque<wstring> &path) 
{
  // TODO: should we impl this?
  throw make_shared<UnsupportedOperationException>();
}

deque<std::shared_ptr<FacetResult>>
RangeFacetCounts::getAllDims(int topN) 
{
  return Collections::singletonList(getTopChildren(topN, field));
}

wstring RangeFacetCounts::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"RangeFacetCounts totCount=");
  b->append(totCount);
  b->append(L":\n");
  for (int i = 0; i < ranges.size(); i++) {
    b->append(L"  ");
    b->append(ranges[i]->label);
    b->append(L" -> count=");
    b->append(counts[i]);
    b->append(L'\n');
  }
  return b->toString();
}
} // namespace org::apache::lucene::facet::range