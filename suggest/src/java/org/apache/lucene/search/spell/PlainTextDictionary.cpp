using namespace std;

#include "PlainTextDictionary.h"

namespace org::apache::lucene::search::spell
{
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using IOUtils = org::apache::lucene::util::IOUtils;

PlainTextDictionary::PlainTextDictionary(shared_ptr<Path> path) throw(
    IOException)
{
  in_ = Files::newBufferedReader(path, StandardCharsets::UTF_8);
}

PlainTextDictionary::PlainTextDictionary(shared_ptr<InputStream> dictFile)
{
  in_ = make_shared<BufferedReader>(
      IOUtils::getDecodingReader(dictFile, StandardCharsets::UTF_8));
}

PlainTextDictionary::PlainTextDictionary(shared_ptr<Reader> reader)
{
  in_ = make_shared<BufferedReader>(reader);
}

shared_ptr<InputIterator>
PlainTextDictionary::getEntryIterator() 
{
  return make_shared<InputIterator::InputIteratorWrapper>(
      make_shared<FileIterator>(shared_from_this()));
}

PlainTextDictionary::FileIterator::FileIterator(
    shared_ptr<PlainTextDictionary> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<BytesRef>
PlainTextDictionary::FileIterator::next() 
{
  if (done) {
    return nullptr;
  }
  bool success = false;
  shared_ptr<BytesRef> result;
  try {
    wstring line;
    if ((line = outerInstance->in_->readLine()) != L"") {
      spare->copyChars(line);
      result = spare->get();
    } else {
      done = true;
      IOUtils::close({outerInstance->in_});
      result.reset();
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({outerInstance->in_});
    }
  }
  return result;
}
} // namespace org::apache::lucene::search::spell