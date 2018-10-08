using namespace std;

#include "AddIndexesTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/AddIndexesTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CloseIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CreateIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Version = org::apache::lucene::util::Version;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<java::nio::file::Path> testDir, AddIndexesTaskTest::inputDir;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void
// beforeClassAddIndexesTaskTest() throws Exception
void AddIndexesTaskTest::beforeClassAddIndexesTaskTest() 
{
  testDir = createTempDir(L"addIndexesTask");

  // create a dummy index under inputDir
  inputDir = testDir->resolve(L"input");
  shared_ptr<Directory> tmpDir = newFSDirectory(inputDir);
  try {
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        tmpDir, make_shared<IndexWriterConfig>(nullptr));
    for (int i = 0; i < 10; i++) {
      writer->addDocument(make_shared<Document>());
    }
    delete writer;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete tmpDir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassAddIndexesTaskTest()
void AddIndexesTaskTest::afterClassAddIndexesTaskTest()
{
  testDir = inputDir = nullptr;
}

shared_ptr<PerfRunData>
AddIndexesTaskTest::createPerfRunData() 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"writer.version", Version::LATEST->toString());
  props->setProperty(L"print.props", L"false"); // don't print anything
  props->setProperty(L"directory", L"RAMDirectory");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(AddIndexesTask::ADDINDEXES_INPUT_DIR,
                     inputDir->toAbsolutePath()->toString());
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void AddIndexesTaskTest::assertIndex(shared_ptr<PerfRunData> runData) throw(
    runtime_error)
{
  shared_ptr<Directory> taskDir = runData->getDirectory();
  assertSame(RAMDirectory::typeid, taskDir->getClass());
  shared_ptr<IndexReader> r = DirectoryReader::open(taskDir);
  try {
    assertEquals(10, r->numDocs());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete r;
  }
}

void AddIndexesTaskTest::testAddIndexesDefault() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData();
  // create the target index first
  (make_shared<CreateIndexTask>(runData))->doLogic();

  shared_ptr<AddIndexesTask> task = make_shared<AddIndexesTask>(runData);
  task->setup();

  // add the input index
  task->doLogic();

  // close the index
  (make_shared<CloseIndexTask>(runData))->doLogic();

  assertIndex(runData);

  delete runData;
}

void AddIndexesTaskTest::testAddIndexesDir() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData();
  // create the target index first
  (make_shared<CreateIndexTask>(runData))->doLogic();

  shared_ptr<AddIndexesTask> task = make_shared<AddIndexesTask>(runData);
  task->setup();

  // add the input index
  task->setParams(L"true");
  task->doLogic();

  // close the index
  (make_shared<CloseIndexTask>(runData))->doLogic();

  assertIndex(runData);

  delete runData;
}

void AddIndexesTaskTest::testAddIndexesReader() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData();
  // create the target index first
  (make_shared<CreateIndexTask>(runData))->doLogic();

  shared_ptr<AddIndexesTask> task = make_shared<AddIndexesTask>(runData);
  task->setup();

  // add the input index
  task->setParams(L"false");
  task->doLogic();

  // close the index
  (make_shared<CloseIndexTask>(runData))->doLogic();

  assertIndex(runData);

  delete runData;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks