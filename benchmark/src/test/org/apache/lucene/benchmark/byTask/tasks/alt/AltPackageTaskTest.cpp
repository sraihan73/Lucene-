using namespace std;

#include "AltPackageTaskTest.h"
#include "../../../../../../../../java/org/apache/lucene/benchmark/byTask/Benchmark.h"

namespace org::apache::lucene::benchmark::byTask::tasks::alt
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using Benchmark = org::apache::lucene::benchmark::byTask::Benchmark;

void AltPackageTaskTest::testWithoutAlt() 
{
  expectThrows(runtime_error::typeid, [&]() { execBenchmark(altAlg(false)); });
}

void AltPackageTaskTest::testWithAlt() 
{
  shared_ptr<Benchmark> bm = execBenchmark(altAlg(true));
  assertNotNull(bm);
  assertNotNull(bm->getRunData()->getPoints());
}

std::deque<wstring> AltPackageTaskTest::altAlg(bool allowAlt)
{
  wstring altTask = L"{ AltTest }";
  if (allowAlt) {
    return std::deque<wstring>{L"alt.tasks.packages = " +
                                    this->getClass().getPackage().getName(),
                                altTask};
  }
  return std::deque<wstring>{altTask};
}
} // namespace org::apache::lucene::benchmark::byTask::tasks::alt