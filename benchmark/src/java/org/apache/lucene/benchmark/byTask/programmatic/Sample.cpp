using namespace std;

#include "Sample.h"
#include "../PerfRunData.h"
#include "../tasks/AddDocTask.h"
#include "../tasks/CloseIndexTask.h"
#include "../tasks/CreateIndexTask.h"
#include "../tasks/RepSumByNameTask.h"
#include "../tasks/TaskSequence.h"
#include "../utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::programmatic
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using AddDocTask = org::apache::lucene::benchmark::byTask::tasks::AddDocTask;
using CloseIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CloseIndexTask;
using CreateIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CreateIndexTask;
using RepSumByNameTask =
    org::apache::lucene::benchmark::byTask::tasks::RepSumByNameTask;
using TaskSequence =
    org::apache::lucene::benchmark::byTask::tasks::TaskSequence;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

void Sample::main(std::deque<wstring> &args) 
{
  shared_ptr<Properties> p = initProps();
  shared_ptr<Config> conf = make_shared<Config>(p);
  shared_ptr<PerfRunData> runData = make_shared<PerfRunData>(conf);

  // 1. top sequence
  shared_ptr<TaskSequence> top = make_shared<TaskSequence>(
      runData, nullptr, nullptr, false); // top level, not parallel

  // 2. task to create the index
  shared_ptr<CreateIndexTask> create = make_shared<CreateIndexTask>(runData);
  top->addTask(create);

  // 3. task seq to add 500 docs (order matters - top to bottom - add seq to
  // top, only then add to seq)
  shared_ptr<TaskSequence> seq1 =
      make_shared<TaskSequence>(runData, L"AddDocs", top, false);
  seq1->setRepetitions(500);
  seq1->setNoChildReport();
  top->addTask(seq1);

  // 4. task to add the doc
  shared_ptr<AddDocTask> addDoc = make_shared<AddDocTask>(runData);
  // addDoc.setParams("1200"); // doc size limit if supported
  seq1->addTask(addDoc); // order matters 9see comment above)

  // 5. task to close the index
  shared_ptr<CloseIndexTask> close = make_shared<CloseIndexTask>(runData);
  top->addTask(close);

  // task to report
  shared_ptr<RepSumByNameTask> rep = make_shared<RepSumByNameTask>(runData);
  top->addTask(rep);

  // print algorithm
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wcout << top->toString() << endl;

  // execute
  top->doLogic();
}

shared_ptr<Properties> Sample::initProps()
{
  shared_ptr<Properties> p = make_shared<Properties>();
  p->setProperty(L"task.max.depth.log", L"3");
  p->setProperty(L"max.buffered", L"buf:10:10:100:100:10:10:100:100");
  p->setProperty(
      L"doc.maker",
      L"org.apache.lucene.benchmark.byTask.feeds.ReutersContentSource");
  p->setProperty(L"log.step", L"2000");
  p->setProperty(L"doc.delete.step", L"8");
  p->setProperty(L"analyzer",
                 L"org.apache.lucene.analysis.standard.StandardAnalyzer");
  p->setProperty(L"doc.term.deque", L"false");
  p->setProperty(L"directory", L"FSDirectory");
  p->setProperty(L"query.maker",
                 L"org.apache.lucene.benchmark.byTask.feeds.ReutersQueryMaker");
  p->setProperty(L"doc.stored", L"true");
  p->setProperty(L"docs.dir", L"reuters-out");
  p->setProperty(L"compound",
                 L"cmpnd:true:true:true:true:false:false:false:false");
  p->setProperty(L"doc.tokenized", L"true");
  p->setProperty(L"merge.factor", L"mrg:10:100:10:100:10:100:10:100");
  return p;
}
} // namespace org::apache::lucene::benchmark::byTask::programmatic