using namespace std;

#include "WriteEnwikiLineDocTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../PerfRunData.h"
#include "../feeds/DocMaker.h"
#include "../utils/StreamUtils.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using StreamUtils = org::apache::lucene::benchmark::byTask::utils::StreamUtils;
using Document = org::apache::lucene::document::Document;
using IndexableField = org::apache::lucene::index::IndexableField;

WriteEnwikiLineDocTask::WriteEnwikiLineDocTask(
    shared_ptr<PerfRunData> runData) 
    : WriteLineDocTask(runData),
      categoryLineFileOut(make_shared<PrintWriter>(make_shared<BufferedWriter>(
          make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8),
          StreamUtils::BUFFER_SIZE)))
{
  shared_ptr<OutputStream> out =
      StreamUtils::outputStream(categoriesLineFile(Paths->get(fname)));
  writeHeader(categoryLineFileOut);
}

shared_ptr<Path> WriteEnwikiLineDocTask::categoriesLineFile(shared_ptr<Path> f)
{
  shared_ptr<Path> dir = f->toAbsolutePath().getParent();
  wstring categoriesName = L"categories-" + f->getFileName();
  return dir->resolve(categoriesName);
}

WriteEnwikiLineDocTask::~WriteEnwikiLineDocTask()
{
  categoryLineFileOut->close();
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
}

shared_ptr<PrintWriter>
WriteEnwikiLineDocTask::lineFileOut(shared_ptr<Document> doc)
{
  shared_ptr<IndexableField> titleField = doc->getField(DocMaker::TITLE_FIELD);
  if (titleField != nullptr &&
      StringHelper::startsWith(titleField->stringValue(), L"Category:")) {
    return categoryLineFileOut;
  }
  return WriteLineDocTask::lineFileOut(doc);
}
} // namespace org::apache::lucene::benchmark::byTask::tasks