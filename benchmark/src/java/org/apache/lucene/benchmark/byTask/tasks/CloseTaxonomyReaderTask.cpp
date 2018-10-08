using namespace std;

#include "CloseTaxonomyReaderTask.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;

CloseTaxonomyReaderTask::CloseTaxonomyReaderTask(
    shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CloseTaxonomyReaderTask::doLogic() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.facet.taxonomy.TaxonomyReader taxoReader =
  // getRunData().getTaxonomyReader())
  {
    org::apache::lucene::facet::taxonomy::TaxonomyReader taxoReader =
        getRunData()->getTaxonomyReader();
    getRunData()->setTaxonomyReader(nullptr);
    if (taxoReader->getRefCount() != 1) {
      wcout << L"WARNING: CloseTaxonomyReader: reference count is currently "
            << taxoReader->getRefCount() << endl;
    }
  }
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks