using namespace std;

#include "RandomFacetSource.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetField.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include "../utils/Config.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

void RandomFacetSource::getNextFacets(
    deque<std::shared_ptr<FacetField>> &facets) throw(NoMoreDataException,
                                                       IOException)
{
  facets.clear();
  int numFacets =
      1 + random->nextInt(maxDocFacets); // at least one facet to each doc
  for (int i = 0; i < numFacets; i++) {
    int depth;
    if (maxFacetDepth == 2) {
      depth = 2;
    } else {
      depth = 2 + random->nextInt(maxFacetDepth - 2); // depth < 2 is not useful
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring dim = Integer::toString(random->nextInt(maxDims));
    std::deque<wstring> components(depth - 1);
    for (int k = 0; k < depth - 1; k++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      components[k] = Integer::toString(random->nextInt(maxValue));
      addItem();
    }
    shared_ptr<FacetField> ff = make_shared<FacetField>(dim, components);
    facets.push_back(ff);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    addBytes(ff->toString()->length()); // very rough approximation
  }
}

void RandomFacetSource::configure(shared_ptr<FacetsConfig> config)
{
  for (int i = 0; i < maxDims; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    config->setHierarchical(Integer::toString(i), true);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    config->setMultiValued(Integer::toString(i), true);
  }
}

RandomFacetSource::~RandomFacetSource()
{
  // nothing to do here
}

void RandomFacetSource::setConfig(shared_ptr<Config> config)
{
  FacetSource::setConfig(config);
  random = make_shared<Random>(config->get(L"rand.seed", 13));
  maxDocFacets = config->get(L"max.doc.facets", 10);
  maxDims = config->get(L"max.doc.facets.dims", 5);
  maxFacetDepth = config->get(L"max.facet.depth", 3);
  if (maxFacetDepth < 2) {
    throw invalid_argument(L"max.facet.depth must be at least 2; got: " +
                           to_wstring(maxFacetDepth));
  }
  maxValue = maxDocFacets * maxFacetDepth;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds