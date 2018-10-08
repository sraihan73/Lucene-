using namespace std;

#include "InfoStream.h"

namespace org::apache::lucene::util
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
const shared_ptr<InfoStream> InfoStream::NO_OUTPUT = make_shared<NoOutput>();

void InfoStream::NoOutput::message(const wstring &component,
                                   const wstring &message)
{
  assert(
      (false, L"message() should not be called when isEnabled returns false"));
}

bool InfoStream::NoOutput::isEnabled(const wstring &component) { return false; }

InfoStream::NoOutput::~NoOutput() {}

shared_ptr<InfoStream> InfoStream::defaultInfoStream = NO_OUTPUT;

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<InfoStream> InfoStream::getDefault() { return defaultInfoStream; }

// C++ WARNING: The following method was originally marked 'synchronized':
void InfoStream::setDefault(shared_ptr<InfoStream> infoStream)
{
  if (infoStream == nullptr) {
    throw invalid_argument(
        wstring(L"Cannot set InfoStream default implementation to null. ") +
        L"To disable logging use InfoStream.NO_OUTPUT");
  }
  defaultInfoStream = infoStream;
}
} // namespace org::apache::lucene::util