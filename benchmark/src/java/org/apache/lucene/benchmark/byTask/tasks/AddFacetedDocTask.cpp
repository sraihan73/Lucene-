using namespace std;

#include "AddFacetedDocTask.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetField.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include "../PerfRunData.h"
#include "../feeds/FacetSource.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using FacetSource = org::apache::lucene::benchmark::byTask::feeds::FacetSource;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

AddFacetedDocTask::AddFacetedDocTask(shared_ptr<PerfRunData> runData)
    : AddDocTask(runData)
{
}

void AddFacetedDocTask::setup() 
{
  AddDocTask::setup();
  if (config == nullptr) {
    bool withFacets = getRunData()->getConfig()->get(L"with.facets", true);
    if (withFacets) {
      shared_ptr<FacetSource> facetsSource = getRunData()->getFacetSource();
      config = make_shared<FacetsConfig>();
      facetsSource->configure(config);
    }
  }
}

wstring AddFacetedDocTask::getLogMessage(int recsCount)
{
  if (config == nullptr) {
    return AddDocTask::getLogMessage(recsCount);
  }
  return AddDocTask::getLogMessage(recsCount) + L" with facets";
}

int AddFacetedDocTask::doLogic() 
{
  if (config != nullptr) {
    deque<std::shared_ptr<FacetField>> facets =
        deque<std::shared_ptr<FacetField>>();
    getRunData()->getFacetSource()->getNextFacets(facets);
    for (auto ff : facets) {
      doc->push_back(ff);
    }
    doc = config->build(getRunData()->getTaxonomyWriter(), doc);
  }
  return AddDocTask::doLogic();
}
} // namespace org::apache::lucene::benchmark::byTask::tasks