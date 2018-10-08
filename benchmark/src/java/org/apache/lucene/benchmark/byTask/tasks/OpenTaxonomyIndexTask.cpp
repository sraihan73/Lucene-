using namespace std;

#include "OpenTaxonomyIndexTask.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyWriter.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;

OpenTaxonomyIndexTask::OpenTaxonomyIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int OpenTaxonomyIndexTask::doLogic() 
{
  shared_ptr<PerfRunData> runData = getRunData();
  runData->setTaxonomyWriter(
      make_shared<DirectoryTaxonomyWriter>(runData->getTaxonomyDir()));
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks