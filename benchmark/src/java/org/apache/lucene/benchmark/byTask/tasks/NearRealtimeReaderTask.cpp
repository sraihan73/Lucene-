using namespace std;

#include "NearRealtimeReaderTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

NearRealtimeReaderTask::NearRealtimeReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int NearRealtimeReaderTask::doLogic() 
{

  shared_ptr<PerfRunData> *const runData = getRunData();

  // Get initial reader
  shared_ptr<IndexWriter> w = runData->getIndexWriter();
  if (w == nullptr) {
    throw runtime_error(
        L"please open the writer before invoking NearRealtimeReader");
  }

  if (runData->getIndexReader() != nullptr) {
    throw runtime_error(
        L"please close the existing reader before invoking NearRealtimeReader");
  }

  int64_t t = System::currentTimeMillis();
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  runData->setIndexReader(r);
  // Transfer our reference to runData
  r->decRef();

  // TODO: gather basic metrics for reporting -- eg mean,
  // stddev, min/max reopen latencies

  // Parent sequence sets stopNow
  reopenCount = 0;
  while (!stopNow_) {
    int64_t waitForMsec = (pauseMSec - (System::currentTimeMillis() - t));
    if (waitForMsec > 0) {
      delay(waitForMsec);
      // System.out.println("NRT wait: " + waitForMsec + " msec");
    }

    t = System::currentTimeMillis();
    shared_ptr<DirectoryReader> *const newReader =
        DirectoryReader::openIfChanged(r);
    if (newReader != nullptr) {
      constexpr int delay = static_cast<int>(System::currentTimeMillis() - t);
      if (reopenTimes.size() == reopenCount) {
        reopenTimes = ArrayUtil::grow(reopenTimes, 1 + reopenCount);
      }
      reopenTimes[reopenCount++] = delay;
      // TODO: somehow we need to enable warming, here
      runData->setIndexReader(newReader);
      // Transfer our reference to runData
      newReader->decRef();
      r = newReader;
    }
  }
  stopNow_ = false;

  return reopenCount;
}

void NearRealtimeReaderTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  pauseMSec = static_cast<int64_t>(1000.0 * stof(params));
}

NearRealtimeReaderTask::~NearRealtimeReaderTask()
{
  wcout << L"NRT reopen times:" << endl;
  for (int i = 0; i < reopenCount; i++) {
    wcout << L" " << reopenTimes[i];
  }
  wcout << endl;
}

bool NearRealtimeReaderTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks