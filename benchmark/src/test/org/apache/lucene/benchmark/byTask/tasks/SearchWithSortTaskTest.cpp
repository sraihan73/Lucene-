using namespace std;

#include "SearchWithSortTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/SearchWithSortTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using SortField = org::apache::lucene::search::SortField;

void SearchWithSortTaskTest::testSetParams_docField() 
{
  shared_ptr<SearchWithSortTask> task = make_shared<SearchWithSortTask>(
      make_shared<PerfRunData>(make_shared<Config>(make_shared<Properties>())));
  task->setParams(L"doc");
  assertEquals(SortField::Type::DOC, task->getSort()->getSort()[0]->getType());
}
} // namespace org::apache::lucene::benchmark::byTask::tasks