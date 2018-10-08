using namespace std;

#include "PrintReaderTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../PerfRunData.h"
#include "OpenReaderTask.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;

PrintReaderTask::PrintReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

void PrintReaderTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  userData = params;
}

bool PrintReaderTask::supportsParams() { return true; }

int PrintReaderTask::doLogic() 
{
  shared_ptr<Directory> dir = getRunData()->getDirectory();
  shared_ptr<IndexReader> r = nullptr;
  if (userData == L"") {
    r = DirectoryReader::open(dir);
  } else {
    r = DirectoryReader::open(OpenReaderTask::findIndexCommit(dir, userData));
  }
  wcout << L"--> numDocs:" << r->numDocs() << L" dels:" << r->numDeletedDocs()
        << endl;
  delete r;
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks