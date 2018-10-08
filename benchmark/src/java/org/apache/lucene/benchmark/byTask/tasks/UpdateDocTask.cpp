using namespace std;

#include "UpdateDocTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../PerfRunData.h"
#include "../feeds/DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Document = org::apache::lucene::document::Document;
using Term = org::apache::lucene::index::Term;
using IndexWriter = org::apache::lucene::index::IndexWriter;

UpdateDocTask::UpdateDocTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

void UpdateDocTask::setup() 
{
  PerfTask::setup();
  shared_ptr<DocMaker> docMaker = getRunData()->getDocMaker();
  if (docSize > 0) {
    doc = docMaker->makeDocument(docSize);
  } else {
    doc = docMaker->makeDocument();
  }
}

void UpdateDocTask::tearDown() 
{
  doc.reset();
  PerfTask::tearDown();
}

int UpdateDocTask::doLogic() 
{
  const wstring docID = doc[DocMaker::ID_FIELD];
  if (docID == L"") {
    throw make_shared<IllegalStateException>(
        L"document must define the docid field");
  }
  shared_ptr<IndexWriter> *const iw = getRunData()->getIndexWriter();
  iw->updateDocument(make_shared<Term>(DocMaker::ID_FIELD, docID), doc);
  return 1;
}

wstring UpdateDocTask::getLogMessage(int recsCount)
{
  return L"updated " + to_wstring(recsCount) + L" docs";
}

void UpdateDocTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  docSize = static_cast<int>(stof(params));
}

bool UpdateDocTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks