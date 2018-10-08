using namespace std;

#include "TaxonomyFacets.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using DimConfig = org::apache::lucene::facet::FacetsConfig::DimConfig;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
const shared_ptr<java::util::Comparator<
    std::shared_ptr<org::apache::lucene::facet::FacetResult>>>
    TaxonomyFacets::BY_VALUE_THEN_DIM =
        make_shared<ComparatorAnonymousInnerClass>();

TaxonomyFacets::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int TaxonomyFacets::ComparatorAnonymousInnerClass::compare(
    shared_ptr<FacetResult> a, shared_ptr<FacetResult> b)
{
  if (a->value->doubleValue() > b->value->doubleValue()) {
    return -1;
  } else if (b->value->doubleValue() > a->value->doubleValue()) {
    return 1;
  } else {
    return a->dim.compare(b->dim);
  }
}

TaxonomyFacets::TaxonomyFacets(
    const wstring &indexFieldName, shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config) 
    : indexFieldName(indexFieldName), taxoReader(taxoReader), config(config),
      parents(taxoReader->getParallelTaxonomyArrays()->parents())
{
}

std::deque<int> TaxonomyFacets::getChildren() 
{
  if (children.empty()) {
    children = taxoReader->getParallelTaxonomyArrays()->children();
  }
  return children;
}

std::deque<int> TaxonomyFacets::getSiblings() 
{
  if (siblings.empty()) {
    siblings = taxoReader->getParallelTaxonomyArrays()->siblings();
  }
  return siblings;
}

bool TaxonomyFacets::childrenLoaded() { return children.size() > 0; }

bool TaxonomyFacets::siblingsLoaded() { return children.size() > 0; }

shared_ptr<DimConfig> TaxonomyFacets::verifyDim(const wstring &dim)
{
  shared_ptr<DimConfig> dimConfig = config->getDimConfig(dim);
  if (dimConfig->indexFieldName != indexFieldName) {
    throw invalid_argument(L"dimension \"" + dim +
                           L"\" was not indexed into field \"" +
                           indexFieldName + L"\"");
  }
  return dimConfig;
}

deque<std::shared_ptr<FacetResult>>
TaxonomyFacets::getAllDims(int topN) 
{
  std::deque<int> children = getChildren();
  std::deque<int> siblings = getSiblings();
  int ord = children[TaxonomyReader::ROOT_ORDINAL];
  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();
  while (ord != TaxonomyReader::INVALID_ORDINAL) {
    wstring dim = taxoReader->getPath(ord)->components[0];
    shared_ptr<DimConfig> dimConfig = config->getDimConfig(dim);
    if (dimConfig->indexFieldName == indexFieldName) {
      shared_ptr<FacetResult> result = getTopChildren(topN, dim);
      if (result != nullptr) {
        results.push_back(result);
      }
    }
    ord = siblings[ord];
  }

  // Sort by highest value, tie break by dim:
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(results, BY_VALUE_THEN_DIM);
  sort(results.begin(), results.end(), BY_VALUE_THEN_DIM);
  return results;
}
} // namespace org::apache::lucene::facet::taxonomy