using namespace std;

#include "MultiFacets.h"

namespace org::apache::lucene::facet
{

MultiFacets::MultiFacets(
    unordered_map<wstring, std::shared_ptr<Facets>> &dimToFacets)
    : MultiFacets(dimToFacets, nullptr)
{
}

MultiFacets::MultiFacets(
    unordered_map<wstring, std::shared_ptr<Facets>> &dimToFacets,
    shared_ptr<Facets> defaultFacets)
    : dimToFacets(dimToFacets), defaultFacets(defaultFacets)
{
}

shared_ptr<FacetResult>
MultiFacets::getTopChildren(int topN, const wstring &dim,
                            deque<wstring> &path) 
{
  shared_ptr<Facets> facets = dimToFacets[dim];
  if (facets == nullptr) {
    if (defaultFacets == nullptr) {
      throw invalid_argument(L"invalid dim \"" + dim + L"\"");
    }
    facets = defaultFacets;
  }
  return facets->getTopChildren(topN, dim, {path});
}

shared_ptr<Number>
MultiFacets::getSpecificValue(const wstring &dim,
                              deque<wstring> &path) 
{
  shared_ptr<Facets> facets = dimToFacets[dim];
  if (facets == nullptr) {
    if (defaultFacets == nullptr) {
      throw invalid_argument(L"invalid dim \"" + dim + L"\"");
    }
    facets = defaultFacets;
  }
  return facets->getSpecificValue(dim, {path});
}

deque<std::shared_ptr<FacetResult>>
MultiFacets::getAllDims(int topN) 
{

  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();

  // First add the specific dim's facets:
  for (auto ent : dimToFacets) {
    results.push_back(ent.second::getTopChildren(topN, ent.first));
  }

  if (defaultFacets != nullptr) {

    // Then add all default facets as long as we didn't
    // already add that dim:
    for (auto result : defaultFacets->getAllDims(topN)) {
      if (dimToFacets.find(result->dim) != dimToFacets.end() == false) {
        results.push_back(result);
      }
    }
  }

  return results;
}
} // namespace org::apache::lucene::facet