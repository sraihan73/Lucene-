using namespace std;

#include "CreateTaxonomyIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyWriter.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;

CreateTaxonomyIndexTask::CreateTaxonomyIndexTask(
    shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CreateTaxonomyIndexTask::doLogic() 
{
  shared_ptr<PerfRunData> runData = getRunData();
  runData->setTaxonomyWriter(make_shared<DirectoryTaxonomyWriter>(
      runData->getTaxonomyDir(), OpenMode::CREATE));
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks