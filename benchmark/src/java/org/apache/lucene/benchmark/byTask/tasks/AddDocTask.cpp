using namespace std;

#include "AddDocTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../PerfRunData.h"
#include "../feeds/DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Document = org::apache::lucene::document::Document;

AddDocTask::AddDocTask(shared_ptr<PerfRunData> runData) : PerfTask(runData) {}

void AddDocTask::setup() 
{
  PerfTask::setup();
  shared_ptr<DocMaker> docMaker = getRunData()->getDocMaker();
  if (docSize > 0) {
    doc = docMaker->makeDocument(docSize);
  } else {
    doc = docMaker->makeDocument();
  }
}

void AddDocTask::tearDown() 
{
  doc.reset();
  PerfTask::tearDown();
}

wstring AddDocTask::getLogMessage(int recsCount)
{
  return wstring::format(Locale::ROOT, L"added %9d docs", recsCount);
}

int AddDocTask::doLogic() 
{
  getRunData()->getIndexWriter()->addDocument(doc);
  return 1;
}

void AddDocTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  docSize = static_cast<int>(stof(params));
}

bool AddDocTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks