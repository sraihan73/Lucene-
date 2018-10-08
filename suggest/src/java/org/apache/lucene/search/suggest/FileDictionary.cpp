using namespace std;

#include "FileDictionary.h"

namespace org::apache::lucene::search::suggest
{
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring FileDictionary::DEFAULT_FIELD_DELIMITER = L"\t";

FileDictionary::FileDictionary(shared_ptr<InputStream> dictFile)
    : FileDictionary(dictFile, DEFAULT_FIELD_DELIMITER)
{
}

FileDictionary::FileDictionary(shared_ptr<Reader> reader)
    : FileDictionary(reader, DEFAULT_FIELD_DELIMITER)
{
}

FileDictionary::FileDictionary(shared_ptr<Reader> reader,
                               const wstring &fieldDelimiter)
    : fieldDelimiter(fieldDelimiter)
{
  in_ = make_shared<BufferedReader>(reader);
}

FileDictionary::FileDictionary(shared_ptr<InputStream> dictFile,
                               const wstring &fieldDelimiter)
    : fieldDelimiter(fieldDelimiter)
{
  in_ = make_shared<BufferedReader>(
      IOUtils::getDecodingReader(dictFile, StandardCharsets::UTF_8));
}

shared_ptr<InputIterator> FileDictionary::getEntryIterator()
{
  try {
    return make_shared<FileIterator>(shared_from_this());
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

FileDictionary::FileIterator::FileIterator(
    shared_ptr<FileDictionary> outerInstance) 
    : outerInstance(outerInstance)
{
  outerInstance->line = outerInstance->in_->readLine();
  if (outerInstance->line == L"") {
    outerInstance->done = true;
    IOUtils::close({outerInstance->in_});
  } else {
    std::deque<wstring> fields =
        outerInstance->line.split(outerInstance->fieldDelimiter);
    if (fields.size() > 3) {
      throw invalid_argument(L"More than 3 fields in one line");
    } else if (fields.size() == 3) { // term, weight, payload
      hasPayloads_ = true;
      spare->copyChars(fields[0]);
      readWeight(fields[1]);
      curPayload->copyChars(fields[2]);
    } else if (fields.size() == 2) { // term, weight
      spare->copyChars(fields[0]);
      readWeight(fields[1]);
    } else { // only term
      spare->copyChars(fields[0]);
      curWeight = 1;
    }
  }
}

int64_t FileDictionary::FileIterator::weight() { return curWeight; }

shared_ptr<BytesRef> FileDictionary::FileIterator::next() 
{
  if (outerInstance->done) {
    return nullptr;
  }
  if (isFirstLine) {
    isFirstLine = false;
    return spare->get();
  }
  outerInstance->line = outerInstance->in_->readLine();
  if (outerInstance->line != L"") {
    std::deque<wstring> fields =
        outerInstance->line.split(outerInstance->fieldDelimiter);
    if (fields.size() > 3) {
      throw invalid_argument(L"More than 3 fields in one line");
    } else if (fields.size() == 3) { // term, weight and payload
      spare->copyChars(fields[0]);
      readWeight(fields[1]);
      if (hasPayloads_) {
        curPayload->copyChars(fields[2]);
      }
    } else if (fields.size() == 2) { // term, weight
      spare->copyChars(fields[0]);
      readWeight(fields[1]);
      if (hasPayloads_) { // have an empty payload
        curPayload = make_shared<BytesRefBuilder>();
      }
    } else { // only term
      spare->copyChars(fields[0]);
      curWeight = 1;
      if (hasPayloads_) {
        curPayload = make_shared<BytesRefBuilder>();
      }
    }
    return spare->get();
  } else {
    outerInstance->done = true;
    IOUtils::close({outerInstance->in_});
    return nullptr;
  }
}

shared_ptr<BytesRef> FileDictionary::FileIterator::payload()
{
  return (hasPayloads_) ? curPayload->get() : nullptr;
}

bool FileDictionary::FileIterator::hasPayloads() { return hasPayloads_; }

void FileDictionary::FileIterator::readWeight(const wstring &weight)
{
  // keep reading floats for bw compat
  try {
    curWeight = StringHelper::fromString<int64_t>(weight);
  } catch (const NumberFormatException &e) {
    curWeight = static_cast<int64_t>(stod(weight));
  }
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
FileDictionary::FileIterator::contexts()
{
  return nullptr;
}

bool FileDictionary::FileIterator::hasContexts() { return false; }
} // namespace org::apache::lucene::search::suggest