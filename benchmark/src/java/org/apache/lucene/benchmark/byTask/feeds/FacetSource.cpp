using namespace std;

#include "FacetSource.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetField.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetsConfig.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

void FacetSource::resetInputs() 
{
  printStatistics(L"facets");
  // re-initiate since properties by round may have changed.
  setConfig(getConfig());
  ContentItemsSource::resetInputs();
}
} // namespace org::apache::lucene::benchmark::byTask::feeds