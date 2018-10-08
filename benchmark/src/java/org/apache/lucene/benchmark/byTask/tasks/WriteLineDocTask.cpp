using namespace std;

#include "WriteLineDocTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../PerfRunData.h"
#include "../feeds/DocMaker.h"
#include "../utils/Config.h"
#include "../utils/StreamUtils.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using StreamUtils = org::apache::lucene::benchmark::byTask::utils::StreamUtils;
using Document = org::apache::lucene::document::Document;
using IndexableField = org::apache::lucene::index::IndexableField;
const wstring WriteLineDocTask::FIELDS_HEADER_INDICATOR =
    L"FIELDS_HEADER_INDICATOR###";
std::deque<wstring> const WriteLineDocTask::DEFAULT_FIELDS =
    std::deque<wstring>{
        org::apache::lucene::benchmark::byTask::feeds::DocMaker::TITLE_FIELD,
        org::apache::lucene::benchmark::byTask::feeds::DocMaker::DATE_FIELD,
        org::apache::lucene::benchmark::byTask::feeds::DocMaker::BODY_FIELD};
const wstring WriteLineDocTask::DEFAULT_SUFFICIENT_FIELDS =
    org::apache::lucene::benchmark::byTask::feeds::DocMaker::TITLE_FIELD +
    StringHelper::toString(L',') +
    org::apache::lucene::benchmark::byTask::feeds::DocMaker::BODY_FIELD;

WriteLineDocTask::WriteLineDocTask(shared_ptr<PerfRunData> runData) throw(
    runtime_error)
    : PerfTask(runData), fname(config->get(L"line.file.out", nullptr)),
      lineFileOut(make_shared<PrintWriter>(make_shared<BufferedWriter>(
          make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8),
          StreamUtils::BUFFER_SIZE))),
      docMaker(runData->getDocMaker()),
      sufficientFields(std::deque<bool>(fieldsToWrite.size()))
{
  shared_ptr<Config> config = runData->getConfig();
  if (fname == L"") {
    throw invalid_argument(L"line.file.out must be set");
  }
  shared_ptr<OutputStream> out = StreamUtils::outputStream(Paths->get(fname));

  // init fields
  wstring f2r = config->get(L"line.fields", L"");
  if (f2r == L"") {
    fieldsToWrite = DEFAULT_FIELDS;
  } else {
    if (f2r.find(SEP) != wstring::npos) {
      throw invalid_argument(L"line.fields " + f2r +
                             L" should not contain the separator char: " +
                             StringHelper::toString(SEP));
    }
    fieldsToWrite = f2r.split(L",");
  }

  // init sufficient fields
  wstring suff = config->get(L"sufficient.fields", DEFAULT_SUFFICIENT_FIELDS);
  if (L"," == suff) {
    checkSufficientFields = false;
  } else {
    checkSufficientFields = true;
    unordered_set<wstring> sf =
        unordered_set<wstring>(Arrays::asList(suff.split(L",")));
    for (int i = 0; i < fieldsToWrite.size(); i++) {
      if (find(sf.begin(), sf.end(), fieldsToWrite[i]) != sf.end()) {
        sufficientFields[i] = true;
      }
    }
  }

  writeHeader(lineFileOut_);
}

void WriteLineDocTask::writeHeader(shared_ptr<PrintWriter> out)
{
  shared_ptr<StringBuilder> sb = threadBuffer->get();
  if (sb == nullptr) {
    sb = make_shared<StringBuilder>();
    threadBuffer->set(sb);
  }
  sb->setLength(0);
  sb->append(FIELDS_HEADER_INDICATOR);
  for (auto f : fieldsToWrite) {
    sb->append(SEP)->append(f);
  }
  out->println(sb->toString());
}

wstring WriteLineDocTask::getLogMessage(int recsCount)
{
  return L"Wrote " + to_wstring(recsCount) + L" line docs";
}

int WriteLineDocTask::doLogic() 
{
  shared_ptr<Document> doc =
      docSize > 0 ? docMaker->makeDocument(docSize) : docMaker->makeDocument();

  shared_ptr<Matcher> matcher = threadNormalizer->get();
  if (matcher == nullptr) {
    matcher = Pattern::compile(L"[\t\r\n]+").matcher(L"");
    threadNormalizer->set(matcher);
  }

  shared_ptr<StringBuilder> sb = threadBuffer->get();
  if (sb == nullptr) {
    sb = make_shared<StringBuilder>();
    threadBuffer->set(sb);
  }
  sb->setLength(0);

  bool sufficient = !checkSufficientFields;
  for (int i = 0; i < fieldsToWrite.size(); i++) {
    shared_ptr<IndexableField> f = doc->getField(fieldsToWrite[i]);
    wstring text =
        f == nullptr
            ? L""
            : matcher->reset(f->stringValue()).replaceAll(L" ")->trim();
    sb->append(text)->append(SEP);
    sufficient |= text.length() > 0 && sufficientFields[i];
  }
  if (sufficient) {
    sb->setLength(sb->length() - 1); // remove redundant last separator
    // lineFileOut is a PrintWriter, which synchronizes internally in println.
    lineFileOut(doc)->println(sb->toString());
  }

  return 1;
}

shared_ptr<PrintWriter> WriteLineDocTask::lineFileOut(shared_ptr<Document> doc)
{
  return lineFileOut_;
}

WriteLineDocTask::~WriteLineDocTask()
{
  lineFileOut_->close();
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
}

void WriteLineDocTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  docSize = static_cast<int>(stof(params));
}

bool WriteLineDocTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks