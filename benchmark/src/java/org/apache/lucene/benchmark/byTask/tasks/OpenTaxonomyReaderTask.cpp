using namespace std;

#include "OpenTaxonomyReaderTask.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyReader.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;

OpenTaxonomyReaderTask::OpenTaxonomyReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int OpenTaxonomyReaderTask::doLogic() 
{
  shared_ptr<PerfRunData> runData = getRunData();
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(runData->getTaxonomyDir());
  runData->setTaxonomyReader(taxoReader);
  // We transfer reference to the run data
  taxoReader->decRef();
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks