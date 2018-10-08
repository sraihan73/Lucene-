using namespace std;

#include "CommitTaxonomyIndexTask.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;

CommitTaxonomyIndexTask::CommitTaxonomyIndexTask(
    shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CommitTaxonomyIndexTask::doLogic() 
{
  shared_ptr<TaxonomyWriter> taxonomyWriter = getRunData()->getTaxonomyWriter();
  if (taxonomyWriter != nullptr) {
    taxonomyWriter->commit();
  } else {
    throw make_shared<IllegalStateException>(
        L"TaxonomyWriter is not currently open");
  }

  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks