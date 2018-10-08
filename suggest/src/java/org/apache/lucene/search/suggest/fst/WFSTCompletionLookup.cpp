using namespace std;

#include "WFSTCompletionLookup.h"

namespace org::apache::lucene::search::suggest::fst
{
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using SortedInputIterator =
    org::apache::lucene::search::suggest::SortedInputIterator;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
using org::apache::lucene::util::fst::Util::Result;
using org::apache::lucene::util::fst::Util::TopResults;

WFSTCompletionLookup::WFSTCompletionLookup(shared_ptr<Directory> tempDir,
                                           const wstring &tempFileNamePrefix)
    : WFSTCompletionLookup(tempDir, tempFileNamePrefix, true)
{
}

WFSTCompletionLookup::WFSTCompletionLookup(shared_ptr<Directory> tempDir,
                                           const wstring &tempFileNamePrefix,
                                           bool exactFirst)
    : exactFirst(exactFirst), tempDir(tempDir),
      tempFileNamePrefix(tempFileNamePrefix)
{
}

void WFSTCompletionLookup::build(shared_ptr<InputIterator> iterator) throw(
    IOException)
{
  if (iterator->hasPayloads()) {
    throw invalid_argument(L"this suggester doesn't support payloads");
  }
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  count = 0;
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  shared_ptr<InputIterator> iter =
      make_shared<WFSTInputIterator>(tempDir, tempFileNamePrefix, iterator);
  shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();
  shared_ptr<BytesRefBuilder> previous = nullptr;
  shared_ptr<PositiveIntOutputs> outputs = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> builder =
      make_shared<Builder<int64_t>>(FST::INPUT_TYPE::BYTE1, outputs);
  while ((scratch = iter->next()) != nullptr) {
    int64_t cost = iter->weight();

    if (previous == nullptr) {
      previous = make_shared<BytesRefBuilder>();
    } else if (scratch->equals(previous->get())) {
      continue; // for duplicate suggestions, the best weight is actually
                // added
    }
    Util::toIntsRef(scratch, scratchInts);
    builder->add(scratchInts->get(), cost);
    previous->copyBytes(scratch);
    count++;
  }
  fst = builder->finish();
}

bool WFSTCompletionLookup::store(shared_ptr<DataOutput> output) throw(
    IOException)
{
  output->writeVLong(count);
  if (fst == nullptr) {
    return false;
  }
  fst->save(output);
  return true;
}

bool WFSTCompletionLookup::load(shared_ptr<DataInput> input) 
{
  count = input->readVLong();
  this->fst =
      make_shared<FST<int64_t>>(input, PositiveIntOutputs::getSingleton());
  return true;
}

deque<std::shared_ptr<Lookup::LookupResult>> WFSTCompletionLookup::lookup(
    shared_ptr<std::wstring> key,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, bool onlyMorePopular,
    int num)
{
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  assert(num > 0);

  if (onlyMorePopular) {
    throw invalid_argument(
        L"this suggester only works with onlyMorePopular=false");
  }

  if (fst == nullptr) {
    return Collections::emptyList();
  }

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  scratch->copyChars(key);
  int prefixLength = scratch->length();
  shared_ptr<FST::Arc<int64_t>> arc = make_shared<FST::Arc<int64_t>>();

  // match the prefix portion exactly
  optional<int64_t> prefixOutput = nullopt;
  try {
    prefixOutput = lookupPrefix(scratch->get(), arc);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }

  if (!prefixOutput) {
    return Collections::emptyList();
  }

  deque<std::shared_ptr<Lookup::LookupResult>> results =
      deque<std::shared_ptr<Lookup::LookupResult>>(num);
  shared_ptr<CharsRefBuilder> spare = make_shared<CharsRefBuilder>();
  if (exactFirst && arc->isFinal()) {
    spare->copyUTF8Bytes(scratch->get());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    results.push_back(make_shared<Lookup::LookupResult>(
        spare->toString(), decodeWeight(prefixOutput + arc->nextFinalOutput)));
    if (--num == 0) {
      return results; // that was quick
    }
  }

  // complete top-N
  shared_ptr<Util::TopResults<int64_t>> completions;
  try {
    completions = Util::shortestPaths(fst, arc, prefixOutput, weightComparator,
                                      num, !exactFirst);
    assert(completions->isComplete);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }

  shared_ptr<BytesRefBuilder> suffix = make_shared<BytesRefBuilder>();
  for (auto completion : completions) {
    scratch->setLength(prefixLength);
    // append suffix
    Util::toBytesRef(completion->input, suffix);
    scratch->append(suffix);
    spare->copyUTF8Bytes(scratch->get());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    results.push_back(make_shared<Lookup::LookupResult>(
        spare->toString(), decodeWeight(completion->output)));
  }
  return results;
}

optional<int64_t> WFSTCompletionLookup::lookupPrefix(
    shared_ptr<BytesRef> scratch,
    shared_ptr<FST::Arc<int64_t>> arc) 
{
  assert(0 == fst->outputs.getNoOutput().longValue());
  int64_t output = 0;
  shared_ptr<FST::BytesReader> bytesReader = fst->getBytesReader();

  fst->getFirstArc(arc);

  std::deque<char> bytes = scratch->bytes;
  int pos = scratch->offset;
  int end = pos + scratch->length;
  while (pos < end) {
    if (fst->findTargetArc(bytes[pos++] & 0xff, arc, arc, bytesReader) ==
        nullptr) {
      return nullopt;
    } else {
      output += arc->output.value();
    }
  }

  return output;
}

any WFSTCompletionLookup::get(shared_ptr<std::wstring> key)
{
  if (fst == nullptr) {
    return nullptr;
  }
  shared_ptr<FST::Arc<int64_t>> arc = make_shared<FST::Arc<int64_t>>();
  optional<int64_t> result = nullopt;
  try {
    result = lookupPrefix(make_shared<BytesRef>(key), arc);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
  if (!result || !arc->isFinal()) {
    return nullptr;
  } else {
    return static_cast<Integer>(decodeWeight(result + arc->nextFinalOutput));
  }
}

int WFSTCompletionLookup::decodeWeight(int64_t encoded)
{
  return static_cast<int>(numeric_limits<int>::max() - encoded);
}

int WFSTCompletionLookup::encodeWeight(int64_t value)
{
  if (value < 0 || value > numeric_limits<int>::max()) {
    throw make_shared<UnsupportedOperationException>(L"cannot encode value: " +
                                                     to_wstring(value));
  }
  return numeric_limits<int>::max() - static_cast<int>(value);
}

WFSTCompletionLookup::WFSTInputIterator::WFSTInputIterator(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    shared_ptr<InputIterator> source) 
    : org::apache::lucene::search::suggest::SortedInputIterator(
          tempDir, tempFileNamePrefix, source)
{
  assert(source->hasPayloads() == false);
}

void WFSTCompletionLookup::WFSTInputIterator::encode(
    shared_ptr<ByteSequencesWriter> writer,
    shared_ptr<ByteArrayDataOutput> output, std::deque<char> &buffer,
    shared_ptr<BytesRef> spare, shared_ptr<BytesRef> payload,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
    int64_t weight) 
{
  if (spare->length + 4 >= buffer.size()) {
    buffer = ArrayUtil::grow(buffer, spare->length + 4);
  }
  output->reset(buffer);
  output->writeBytes(spare->bytes, spare->offset, spare->length);
  output->writeInt(encodeWeight(weight));
  writer->write(buffer, 0, output->getPosition());
}

int64_t WFSTCompletionLookup::WFSTInputIterator::decode(
    shared_ptr<BytesRef> scratch, shared_ptr<ByteArrayDataInput> tmpInput)
{
  scratch->length -= 4; // int
  // skip suggestion:
  tmpInput->reset(scratch->bytes, scratch->offset + scratch->length, 4);
  return tmpInput->readInt();
}

const shared_ptr<java::util::Comparator<int64_t>>
    WFSTCompletionLookup::weightComparator =
        make_shared<ComparatorAnonymousInnerClass>();

WFSTCompletionLookup::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int WFSTCompletionLookup::ComparatorAnonymousInnerClass::compare(
    optional<int64_t> &left, optional<int64_t> &right)
{
  return left.compareTo(right);
}

int64_t WFSTCompletionLookup::ramBytesUsed()
{
  return (fst == nullptr) ? 0 : fst->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
WFSTCompletionLookup::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singleton(Accountables::namedAccountable(L"fst", fst));
  }
}

int64_t WFSTCompletionLookup::getCount() { return count; }
} // namespace org::apache::lucene::search::suggest::fst