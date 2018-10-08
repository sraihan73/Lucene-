using namespace std;

#include "SortedInputIterator.h"

namespace org::apache::lucene::search::suggest
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using ByteSequencesReader =
    org::apache::lucene::util::OfflineSorter::ByteSequencesReader;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;

SortedInputIterator::SortedInputIterator(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    shared_ptr<InputIterator> source) 
    : SortedInputIterator(tempDir, tempFileNamePrefix, source,
                          Comparator::naturalOrder())
{
}

SortedInputIterator::SortedInputIterator(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    shared_ptr<InputIterator> source,
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
        comparator) 
    : source(source), reader(sort()), comparator(comparator),
      hasPayloads(source->hasPayloads()), hasContexts(source->hasContexts()),
      tempDir(tempDir), tempFileNamePrefix(tempFileNamePrefix)
{
}

shared_ptr<BytesRef> SortedInputIterator::next() 
{
  bool success = false;
  if (done) {
    return nullptr;
  }
  try {
    shared_ptr<ByteArrayDataInput> input = make_shared<ByteArrayDataInput>();
    shared_ptr<BytesRef> bytes = reader->next();
    if (bytes != nullptr) {
      weight_ = decode(bytes, input);
      if (hasPayloads_) {
        payload_ = decodePayload(bytes, input);
      }
      if (hasContexts_) {
        contexts_ = decodeContexts(bytes, input);
      }
      success = true;
      return bytes;
    }
    close();
    success = done = true;
    return nullptr;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      done = true;
      close();
    }
  }
}

int64_t SortedInputIterator::weight() { return weight_; }

shared_ptr<BytesRef> SortedInputIterator::payload()
{
  if (hasPayloads_) {
    return payload_;
  }
  return nullptr;
}

bool SortedInputIterator::hasPayloads() { return hasPayloads_; }

shared_ptr<Set<std::shared_ptr<BytesRef>>> SortedInputIterator::contexts()
{
  return contexts_;
}

bool SortedInputIterator::hasContexts() { return hasContexts_; }

SortedInputIterator::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int SortedInputIterator::ComparatorAnonymousInnerClass::compare(
    shared_ptr<BytesRef> left, shared_ptr<BytesRef> right)
{
  // Make shallow copy in case decode changes the BytesRef:
  assert(left != right);
  leftScratch->bytes = left->bytes;
  leftScratch->offset = left->offset;
  leftScratch->length = left->length;
  rightScratch->bytes = right->bytes;
  rightScratch->offset = right->offset;
  rightScratch->length = right->length;
  int64_t leftCost = outerInstance->decode(leftScratch, input);
  int64_t rightCost = outerInstance->decode(rightScratch, input);
  if (outerInstance->hasPayloads_) {
    outerInstance->decodePayload(leftScratch, input);
    outerInstance->decodePayload(rightScratch, input);
  }
  if (outerInstance->hasContexts_) {
    outerInstance->decodeContexts(leftScratch, input);
    outerInstance->decodeContexts(rightScratch, input);
  }
  int cmp = outerInstance->comparator.compare(leftScratch, rightScratch);
  if (cmp != 0) {
    return cmp;
  }
  return Long::compare(leftCost, rightCost);
}

shared_ptr<ByteSequencesReader> SortedInputIterator::sort() 
{

  shared_ptr<OfflineSorter> sorter = make_shared<OfflineSorter>(
      tempDir, tempFileNamePrefix, tieBreakByCostComparator);
  tempInput = tempDir->createTempOutput(tempFileNamePrefix, L"input",
                                        IOContext::DEFAULT);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter writer = new
  // org.apache.lucene.util.OfflineSorter.ByteSequencesWriter(tempInput))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter writer =
        org::apache::lucene::util::OfflineSorter::ByteSequencesWriter(
            tempInput);
    shared_ptr<BytesRef> spare;
    std::deque<char> buffer(0);
    shared_ptr<ByteArrayDataOutput> output =
        make_shared<ByteArrayDataOutput>(buffer);

    while ((spare = source->next()) != nullptr) {
      encode(writer, output, buffer, spare, source->payload(),
             source->contexts(), source->weight());
    }
    CodecUtil::writeFooter(tempInput);
  }

  tempSortedFileName = sorter->sort(tempInput->getName());
  return make_shared<ByteSequencesReader>(
      tempDir->openChecksumInput(tempSortedFileName, IOContext::READONCE),
      tempSortedFileName);
}

SortedInputIterator::~SortedInputIterator()
{
  try {
    IOUtils::close({reader});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::deleteFilesIgnoringExceptions(
        tempDir, {tempInput == nullptr ? L"" : tempInput->getName(),
                  tempSortedFileName});
  }
}

void SortedInputIterator::encode(
    shared_ptr<ByteSequencesWriter> writer,
    shared_ptr<ByteArrayDataOutput> output, std::deque<char> &buffer,
    shared_ptr<BytesRef> spare, shared_ptr<BytesRef> payload,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
    int64_t weight) 
{
  int requiredLength =
      spare->length + 8 + ((hasPayloads_) ? 2 + payload->length : 0);
  if (hasContexts_) {
    for (auto ctx : contexts) {
      requiredLength += 2 + ctx->length;
    }
    requiredLength += 2; // for length of contexts
  }
  if (requiredLength >= buffer.size()) {
    buffer = ArrayUtil::grow(buffer, requiredLength);
  }
  output->reset(buffer);
  output->writeBytes(spare->bytes, spare->offset, spare->length);
  if (hasContexts_) {
    for (auto ctx : contexts) {
      output->writeBytes(ctx->bytes, ctx->offset, ctx->length);
      output->writeShort(static_cast<short>(ctx->length));
    }
    output->writeShort(static_cast<short>(contexts->size()));
  }
  if (hasPayloads_) {
    output->writeBytes(payload->bytes, payload->offset, payload->length);
    output->writeShort(static_cast<short>(payload->length));
  }
  output->writeLong(weight);
  writer->write(buffer, 0, output->getPosition());
}

int64_t SortedInputIterator::decode(shared_ptr<BytesRef> scratch,
                                      shared_ptr<ByteArrayDataInput> tmpInput)
{
  tmpInput->reset(scratch->bytes, scratch->offset, scratch->length);
  tmpInput->skipBytes(scratch->length - 8); // suggestion
  scratch->length -= Long::BYTES;           // long
  return tmpInput->readLong();
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
SortedInputIterator::decodeContexts(shared_ptr<BytesRef> scratch,
                                    shared_ptr<ByteArrayDataInput> tmpInput)
{
  tmpInput->reset(scratch->bytes, scratch->offset, scratch->length);
  tmpInput->skipBytes(scratch->length - 2); // skip to context set size
  short ctxSetSize = tmpInput->readShort();
  scratch->length -= 2;
  shared_ptr<Set<std::shared_ptr<BytesRef>>> *const contextSet =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (short i = 0; i < ctxSetSize; i++) {
    tmpInput->setPosition(scratch->offset + scratch->length - 2);
    short curContextLength = tmpInput->readShort();
    scratch->length -= 2;
    tmpInput->setPosition(scratch->offset + scratch->length - curContextLength);
    shared_ptr<BytesRef> contextSpare = make_shared<BytesRef>(curContextLength);
    tmpInput->readBytes(contextSpare->bytes, 0, curContextLength);
    contextSpare->length = curContextLength;
    contextSet->add(contextSpare);
    scratch->length -= curContextLength;
  }
  return contextSet;
}

shared_ptr<BytesRef>
SortedInputIterator::decodePayload(shared_ptr<BytesRef> scratch,
                                   shared_ptr<ByteArrayDataInput> tmpInput)
{
  tmpInput->reset(scratch->bytes, scratch->offset, scratch->length);
  tmpInput->skipBytes(scratch->length - 2);    // skip to payload size
  short payloadLength = tmpInput->readShort(); // read payload size
  assert((payloadLength >= 0, payloadLength));
  tmpInput->setPosition(scratch->offset + scratch->length - 2 -
                        payloadLength); // setPosition to start of payload
  shared_ptr<BytesRef> payloadScratch = make_shared<BytesRef>(payloadLength);
  tmpInput->readBytes(payloadScratch->bytes, 0, payloadLength); // read payload
  payloadScratch->length = payloadLength;
  scratch->length -= 2;             // payload length info (short)
  scratch->length -= payloadLength; // payload
  return payloadScratch;
}
} // namespace org::apache::lucene::search::suggest