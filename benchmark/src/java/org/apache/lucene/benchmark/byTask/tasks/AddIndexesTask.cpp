using namespace std;

#include "AddIndexesTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/CodecReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/SlowCodecReaderWrapper.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CodecReader = org::apache::lucene::index::CodecReader;
using SlowCodecReaderWrapper =
    org::apache::lucene::index::SlowCodecReaderWrapper;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
const wstring AddIndexesTask::ADDINDEXES_INPUT_DIR = L"addindexes.input.dir";

AddIndexesTask::AddIndexesTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

void AddIndexesTask::setup() 
{
  PerfTask::setup();
  wstring inputDirProp =
      getRunData()->getConfig()->get(ADDINDEXES_INPUT_DIR, L"");
  if (inputDirProp == L"") {
    throw invalid_argument(L"config parameter " + ADDINDEXES_INPUT_DIR +
                           L" not specified in configuration");
  }
  inputDir = FSDirectory::open(Paths->get(inputDirProp));
}

int AddIndexesTask::doLogic() 
{
  shared_ptr<IndexWriter> writer = getRunData()->getIndexWriter();
  if (useAddIndexesDir) {
    writer->addIndexes({inputDir});
  } else {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader r =
    // org.apache.lucene.index.DirectoryReader.open(inputDir))
    {
      org::apache::lucene::index::IndexReader r =
          org::apache::lucene::index::DirectoryReader::open(inputDir);
      std::deque<std::shared_ptr<CodecReader>> leaves(r->leaves().size());
      int i = 0;
      for (auto leaf : r->leaves()) {
        leaves[i++] = SlowCodecReaderWrapper::wrap(leaf->reader());
      }
      writer->addIndexes(leaves);
    }
  }
  return 1;
}

void AddIndexesTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  useAddIndexesDir = StringHelper::fromString<bool>(params);
}

bool AddIndexesTask::supportsParams() { return true; }

void AddIndexesTask::tearDown() 
{
  delete inputDir;
  PerfTask::tearDown();
}
} // namespace org::apache::lucene::benchmark::byTask::tasks