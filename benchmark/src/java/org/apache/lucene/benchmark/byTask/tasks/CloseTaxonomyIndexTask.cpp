using namespace std;

#include "CloseTaxonomyIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IOUtils = org::apache::lucene::util::IOUtils;

CloseTaxonomyIndexTask::CloseTaxonomyIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CloseTaxonomyIndexTask::doLogic() 
{
  IOUtils::close({getRunData()->getTaxonomyWriter()});
  getRunData()->setTaxonomyWriter(nullptr);

  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks