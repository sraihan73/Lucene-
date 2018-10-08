using namespace std;

#include "SearchTravRetLoadFieldSelectorTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/DocumentStoredFieldVisitor.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Document = org::apache::lucene::document::Document;
using DocumentStoredFieldVisitor =
    org::apache::lucene::document::DocumentStoredFieldVisitor;
using IndexReader = org::apache::lucene::index::IndexReader;

SearchTravRetLoadFieldSelectorTask::SearchTravRetLoadFieldSelectorTask(
    shared_ptr<PerfRunData> runData)
    : SearchTravTask(runData)
{
}

bool SearchTravRetLoadFieldSelectorTask::withRetrieve() { return true; }

shared_ptr<Document>
SearchTravRetLoadFieldSelectorTask::retrieveDoc(shared_ptr<IndexReader> ir,
                                                int id) 
{
  if (fieldsToLoad == nullptr) {
    return ir->document(id);
  } else {
    shared_ptr<DocumentStoredFieldVisitor> visitor =
        make_shared<DocumentStoredFieldVisitor>(fieldsToLoad);
    ir->document(id, visitor);
    return visitor->getDocument();
  }
}

void SearchTravRetLoadFieldSelectorTask::setParams(const wstring &params)
{
  this->params =
      params; // cannot just call super.setParams(), b/c its params differ.
  fieldsToLoad = unordered_set<>();
  for (shared_ptr<StringTokenizer> tokenizer =
           make_shared<StringTokenizer>(params, L",");
       tokenizer->hasMoreTokens();) {
    wstring s = tokenizer->nextToken();
    fieldsToLoad->add(s);
  }
}

bool SearchTravRetLoadFieldSelectorTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks