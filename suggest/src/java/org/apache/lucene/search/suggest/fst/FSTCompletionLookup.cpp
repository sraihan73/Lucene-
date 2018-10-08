using namespace std;

#include "FSTCompletionLookup.h"

namespace org::apache::lucene::search::suggest::fst
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using Completion =
    org::apache::lucene::search::suggest::fst::FSTCompletion::Completion;
using TSTLookup = org::apache::lucene::search::suggest::tst::TSTLookup;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using FST = org::apache::lucene::util::fst::FST;
using NoOutputs = org::apache::lucene::util::fst::NoOutputs;
int FSTCompletionLookup::INVALID_BUCKETS_COUNT = -1;

FSTCompletionLookup::FSTCompletionLookup()
    : FSTCompletionLookup(nullptr, nullptr)
{
}

FSTCompletionLookup::FSTCompletionLookup(shared_ptr<Directory> tempDir,
                                         const wstring &tempFileNamePrefix)
    : FSTCompletionLookup(tempDir, tempFileNamePrefix,
                          FSTCompletion::DEFAULT_BUCKETS, true)
{
}

FSTCompletionLookup::FSTCompletionLookup(shared_ptr<Directory> tempDir,
                                         const wstring &tempFileNamePrefix,
                                         int buckets, bool exactMatchFirst)
    : tempDir(tempDir), tempFileNamePrefix(tempFileNamePrefix)
{
  this->buckets = buckets;
  this->exactMatchFirst = exactMatchFirst;
}

FSTCompletionLookup::FSTCompletionLookup(shared_ptr<Directory> tempDir,
                                         const wstring &tempFileNamePrefix,
                                         shared_ptr<FSTCompletion> completion,
                                         bool exactMatchFirst)
    : FSTCompletionLookup(tempDir, tempFileNamePrefix, INVALID_BUCKETS_COUNT,
                          exactMatchFirst)
{
  this->normalCompletion =
      make_shared<FSTCompletion>(completion->getFST(), false, exactMatchFirst);
  this->higherWeightsCompletion =
      make_shared<FSTCompletion>(completion->getFST(), true, exactMatchFirst);
}

void FSTCompletionLookup::build(shared_ptr<InputIterator> iterator) throw(
    IOException)
{
  if (iterator->hasPayloads()) {
    throw invalid_argument(L"this suggester doesn't support payloads");
  }
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }

  shared_ptr<OfflineSorter> sorter =
      make_shared<OfflineSorter>(tempDir, tempFileNamePrefix);
  shared_ptr<ExternalRefSorter> externalSorter =
      make_shared<ExternalRefSorter>(sorter);
  shared_ptr<IndexOutput> tempInput = tempDir->createTempOutput(
      tempFileNamePrefix, L"input", IOContext::DEFAULT);
  wstring tempSortedFileName = L"";

  shared_ptr<OfflineSorter::ByteSequencesWriter> writer =
      make_shared<OfflineSorter::ByteSequencesWriter>(tempInput);
  shared_ptr<OfflineSorter::ByteSequencesReader> reader = nullptr;

  // Push floats up front before sequences to sort them. For now, assume they
  // are non-negative. If negative floats are allowed some trickery needs to be
  // done to find their byte order.
  count = 0;
  try {
    std::deque<char> buffer(0);
    shared_ptr<ByteArrayDataOutput> output =
        make_shared<ByteArrayDataOutput>(buffer);
    shared_ptr<BytesRef> spare;
    int inputLineCount = 0;
    while ((spare = iterator->next()) != nullptr) {
      if (spare->length + 4 >= buffer.size()) {
        buffer = ArrayUtil::grow(buffer, spare->length + 4);
      }

      output->reset(buffer);
      output->writeInt(encodeWeight(iterator->weight()));
      output->writeBytes(spare->bytes, spare->offset, spare->length);
      writer->write(buffer, 0, output->getPosition());
      inputLineCount++;
    }
    CodecUtil::writeFooter(tempInput);
    delete writer;

    // We don't know the distribution of scores and we need to bucket them, so
    // we'll sort and divide into equal buckets.
    tempSortedFileName = sorter->sort(tempInput->getName());
    tempDir->deleteFile(tempInput->getName());

    shared_ptr<FSTCompletionBuilder> builder =
        make_shared<FSTCompletionBuilder>(buckets, externalSorter,
                                          sharedTailLength);

    reader = make_shared<OfflineSorter::ByteSequencesReader>(
        tempDir->openChecksumInput(tempSortedFileName, IOContext::READONCE),
        tempSortedFileName);
    int64_t line = 0;
    int previousBucket = 0;
    int previousScore = 0;
    shared_ptr<ByteArrayDataInput> input = make_shared<ByteArrayDataInput>();
    shared_ptr<BytesRef> tmp2 = make_shared<BytesRef>();
    while (true) {
      shared_ptr<BytesRef> scratch = reader->next();
      if (scratch == nullptr) {
        break;
      }
      input->reset(scratch->bytes, scratch->offset, scratch->length);
      int currentScore = input->readInt();

      int bucket;
      if (line > 0 && currentScore == previousScore) {
        bucket = previousBucket;
      } else {
        bucket = static_cast<int>(line * buckets / inputLineCount);
      }
      previousScore = currentScore;
      previousBucket = bucket;

      // Only append the input, discard the weight.
      tmp2->bytes = scratch->bytes;
      tmp2->offset = scratch->offset + input->getPosition();
      tmp2->length = scratch->length - input->getPosition();
      builder->add(tmp2, bucket);

      line++;
      count++;
    }

    // The two FSTCompletions share the same automaton.
    this->higherWeightsCompletion = builder->build();
    this->normalCompletion = make_shared<FSTCompletion>(
        higherWeightsCompletion->getFST(), false, exactMatchFirst);

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({reader, writer, externalSorter});
    IOUtils::deleteFilesIgnoringExceptions(
        tempDir, {tempInput->getName(), tempSortedFileName});
  }
}

int FSTCompletionLookup::encodeWeight(int64_t value)
{
  if (value < numeric_limits<int>::min() ||
      value > numeric_limits<int>::max()) {
    throw make_shared<UnsupportedOperationException>(L"cannot encode value: " +
                                                     to_wstring(value));
  }
  return static_cast<int>(value);
}

deque<std::shared_ptr<Lookup::LookupResult>>
FSTCompletionLookup::lookup(shared_ptr<std::wstring> key,
                            shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                            bool higherWeightsFirst, int num)
{
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  const deque<std::shared_ptr<Completion>> completions;
  if (higherWeightsFirst) {
    completions = higherWeightsCompletion->lookup(key, num);
  } else {
    completions = normalCompletion->lookup(key, num);
  }

  const deque<std::shared_ptr<Lookup::LookupResult>> results =
      deque<std::shared_ptr<Lookup::LookupResult>>(completions.size());
  shared_ptr<CharsRefBuilder> spare = make_shared<CharsRefBuilder>();
  for (auto c : completions) {
    spare->copyUTF8Bytes(c->utf8);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    results.push_back(
        make_shared<Lookup::LookupResult>(spare->toString(), c->bucket));
  }
  return results;
}

any FSTCompletionLookup::get(shared_ptr<std::wstring> key)
{
  constexpr int bucket = normalCompletion->getBucket(key);
  return bucket == -1 ? nullptr : static_cast<int64_t>(bucket);
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool FSTCompletionLookup::store(shared_ptr<DataOutput> output) throw(
    IOException)
{
  output->writeVLong(count);
  if (normalCompletion == nullptr || normalCompletion->getFST() == nullptr) {
    return false;
  }
  normalCompletion->getFST()->save(output);
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool FSTCompletionLookup::load(shared_ptr<DataInput> input) 
{
  count = input->readVLong();
  this->higherWeightsCompletion = make_shared<FSTCompletion>(
      make_shared<FST<>>(input, NoOutputs::getSingleton()));
  this->normalCompletion = make_shared<FSTCompletion>(
      higherWeightsCompletion->getFST(), false, exactMatchFirst);
  return true;
}

int64_t FSTCompletionLookup::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this()) +
                  RamUsageEstimator::shallowSizeOf(normalCompletion) +
                  RamUsageEstimator::shallowSizeOf(higherWeightsCompletion);
  if (normalCompletion != nullptr) {
    mem += normalCompletion->getFST()->ramBytesUsed();
  }
  if (higherWeightsCompletion != nullptr &&
      (normalCompletion == nullptr ||
       normalCompletion->getFST() != higherWeightsCompletion->getFST())) {
    // the fst should be shared between the 2 completion instances, don't count
    // it twice
    mem += higherWeightsCompletion->getFST()->ramBytesUsed();
  }
  return mem;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FSTCompletionLookup::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (normalCompletion != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"fst", normalCompletion->getFST()));
  }
  if (higherWeightsCompletion != nullptr &&
      (normalCompletion == nullptr ||
       normalCompletion->getFST() != higherWeightsCompletion->getFST())) {
    resources.push_back(Accountables::namedAccountable(
        L"higher weights fst", higherWeightsCompletion->getFST()));
  }
  return Collections::unmodifiableList(resources);
}

int64_t FSTCompletionLookup::getCount() { return count; }
} // namespace org::apache::lucene::search::suggest::fst