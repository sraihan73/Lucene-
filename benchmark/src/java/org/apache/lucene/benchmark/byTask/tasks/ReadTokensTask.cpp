using namespace std;

#include "ReadTokensTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../PerfRunData.h"
#include "../feeds/DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Document = org::apache::lucene::document::Document;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableField = org::apache::lucene::index::IndexableField;

ReadTokensTask::ReadTokensTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

void ReadTokensTask::setup() 
{
  PerfTask::setup();
  shared_ptr<DocMaker> docMaker = getRunData()->getDocMaker();
  doc = docMaker->makeDocument();
}

wstring ReadTokensTask::getLogMessage(int recsCount)
{
  return L"read " + to_wstring(recsCount) + L" docs; " +
         to_wstring(totalTokenCount) + L" tokens";
}

void ReadTokensTask::tearDown() 
{
  doc.reset();
  PerfTask::tearDown();
}

int ReadTokensTask::doLogic() 
{
  deque<std::shared_ptr<IndexableField>> fields = doc->getFields();
  shared_ptr<Analyzer> analyzer = getRunData()->getAnalyzer();
  int tokenCount = 0;
  for (auto field : fields) {
    if (field->fieldType()->indexOptions() == IndexOptions::NONE ||
        field->fieldType()->tokenized() == false) {
      continue;
    }

    shared_ptr<TokenStream> *const stream =
        field->tokenStream(analyzer, nullptr);
    // reset the TokenStream to the first token
    stream->reset();

    shared_ptr<TermToBytesRefAttribute> termAtt =
        stream->getAttribute(TermToBytesRefAttribute::typeid);
    while (stream->incrementToken()) {
      termAtt->getBytesRef();
      tokenCount++;
    }
    stream->end();
    delete stream;
  }
  totalTokenCount += tokenCount;
  return tokenCount;
}

void ReadTokensTask::ReusableStringReader::init(const wstring &s)
{
  this->s = s;
  left = s.length();
  this->upto = 0;
}

int ReadTokensTask::ReusableStringReader::read(std::deque<wchar_t> &c)
{
  return read(c, 0, c.size());
}

int ReadTokensTask::ReusableStringReader::read(std::deque<wchar_t> &c, int off,
                                               int len)
{
  if (left > len) {
    s.getChars(upto, upto + len, c, off);
    upto += len;
    left -= len;
    return len;
  } else if (0 == left) {
    return -1;
  } else {
    s.getChars(upto, upto + left, c, off);
    int r = left;
    left = 0;
    upto = s.length();
    return r;
  }
}

ReadTokensTask::ReusableStringReader::~ReusableStringReader() {}
} // namespace org::apache::lucene::benchmark::byTask::tasks