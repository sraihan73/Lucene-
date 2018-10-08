using namespace std;

#include "OpenReaderTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexCommit.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using Directory = org::apache::lucene::store::Directory;
const wstring OpenReaderTask::USER_DATA = L"userData";

OpenReaderTask::OpenReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int OpenReaderTask::doLogic() 
{
  shared_ptr<Directory> dir = getRunData()->getDirectory();
  shared_ptr<DirectoryReader> r = nullptr;
  if (commitUserData != L"") {
    r = DirectoryReader::open(
        OpenReaderTask::findIndexCommit(dir, commitUserData));
  } else {
    r = DirectoryReader::open(dir);
  }
  getRunData()->setIndexReader(r);
  // We transfer reference to the run data
  r->decRef();
  return 1;
}

void OpenReaderTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  if (params != L"") {
    std::deque<wstring> split = params.split(L",");
    if (split.size() > 0) {
      commitUserData = split[0];
    }
  }
}

bool OpenReaderTask::supportsParams() { return true; }

shared_ptr<IndexCommit>
OpenReaderTask::findIndexCommit(shared_ptr<Directory> dir,
                                const wstring &userData) 
{
  shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
      DirectoryReader::listCommits(dir);
  for (auto ic : commits) {
    unordered_map<wstring, wstring> map_obj = ic->getUserData();
    wstring ud = L"";
    if (map_obj.size() > 0) {
      ud = map_obj[USER_DATA];
    }
    if (ud != L"" && ud == userData) {
      return ic;
    }
  }

  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"index does not contain commit with userData: " +
                            userData);
}
} // namespace org::apache::lucene::benchmark::byTask::tasks