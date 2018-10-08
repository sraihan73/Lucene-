using namespace std;

#include "FSTOrdsOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using Outputs = org::apache::lucene::util::fst::Outputs;
const shared_ptr<Output> FSTOrdsOutputs::NO_OUTPUT = make_shared<Output>(
    make_shared<org::apache::lucene::util::BytesRef>(), 0, 0);
const shared_ptr<org::apache::lucene::util::BytesRef> FSTOrdsOutputs::NO_BYTES =
    make_shared<org::apache::lucene::util::BytesRef>();

FSTOrdsOutputs::Output::Output(shared_ptr<BytesRef> bytes, int64_t startOrd,
                               int64_t endOrd)
    : bytes(bytes), startOrd(startOrd), endOrd(endOrd)
{
  assert((startOrd >= 0, L"startOrd=" + to_wstring(startOrd)));
  assert((endOrd >= 0, L"endOrd=" + to_wstring(endOrd)));
}

wstring FSTOrdsOutputs::Output::toString()
{
  int64_t x;
  if (endOrd > numeric_limits<int64_t>::max() / 2) {
    x = numeric_limits<int64_t>::max() - endOrd;
  } else {
    assert(endOrd >= 0);
    x = -endOrd;
  }
  return to_wstring(startOrd) + L" to " + to_wstring(x);
}

int FSTOrdsOutputs::Output::hashCode()
{
  int hash = bytes->hashCode();
  hash = static_cast<int>(hash ^ startOrd);
  hash = static_cast<int>(hash ^ endOrd);
  return hash;
}

bool FSTOrdsOutputs::Output::equals(any _other)
{
  if (std::dynamic_pointer_cast<Output>(_other) != nullptr) {
    shared_ptr<Output> other = any_cast<std::shared_ptr<Output>>(_other);
    return bytes->equals(other->bytes) && startOrd == other->startOrd &&
           endOrd == other->endOrd;
  } else {
    return false;
  }
}

shared_ptr<Output> FSTOrdsOutputs::common(shared_ptr<Output> output1,
                                          shared_ptr<Output> output2)
{
  shared_ptr<BytesRef> bytes1 = output1->bytes;
  shared_ptr<BytesRef> bytes2 = output2->bytes;

  assert(bytes1 != nullptr);
  assert(bytes2 != nullptr);

  int pos1 = bytes1->offset;
  int pos2 = bytes2->offset;
  int stopAt1 = pos1 + min(bytes1->length, bytes2->length);
  while (pos1 < stopAt1) {
    if (bytes1->bytes[pos1] != bytes2->bytes[pos2]) {
      break;
    }
    pos1++;
    pos2++;
  }

  shared_ptr<BytesRef> prefixBytes;

  if (pos1 == bytes1->offset) {
    // no common prefix
    prefixBytes = NO_BYTES;
  } else if (pos1 == bytes1->offset + bytes1->length) {
    // bytes1 is a prefix of bytes2
    prefixBytes = bytes1;
  } else if (pos2 == bytes2->offset + bytes2->length) {
    // bytes2 is a prefix of bytes1
    prefixBytes = bytes2;
  } else {
    prefixBytes = make_shared<BytesRef>(bytes1->bytes, bytes1->offset,
                                        pos1 - bytes1->offset);
  }

  return newOutput(prefixBytes, min(output1->startOrd, output2->startOrd),
                   min(output1->endOrd, output2->endOrd));
}

shared_ptr<Output> FSTOrdsOutputs::subtract(shared_ptr<Output> output,
                                            shared_ptr<Output> inc)
{
  assert(output != nullptr);
  assert(inc != nullptr);
  if (inc == NO_OUTPUT) {
    // no prefix removed
    return output;
  } else {
    assert((StringHelper::startsWith(output->bytes, inc->bytes)));
    shared_ptr<BytesRef> suffix;
    if (inc->bytes->length == output->bytes->length) {
      // entire output removed
      suffix = NO_BYTES;
    } else if (inc->bytes->length == 0) {
      suffix = output->bytes;
    } else {
      assert((inc->bytes->length < output->bytes->length,
              L"inc.length=" + to_wstring(inc->bytes->length) +
                  L" vs output.length=" + to_wstring(output->bytes->length)));
      assert(inc->bytes->length > 0);
      suffix = make_shared<BytesRef>(
          output->bytes->bytes, output->bytes->offset + inc->bytes->length,
          output->bytes->length - inc->bytes->length);
    }
    assert(output->startOrd >= inc->startOrd);
    assert(output->endOrd >= inc->endOrd);
    return newOutput(suffix, output->startOrd - inc->startOrd,
                     output->endOrd - inc->endOrd);
  }
}

shared_ptr<Output> FSTOrdsOutputs::add(shared_ptr<Output> prefix,
                                       shared_ptr<Output> output)
{
  assert(prefix != nullptr);
  assert(output != nullptr);
  if (prefix == NO_OUTPUT) {
    return output;
  } else if (output == NO_OUTPUT) {
    return prefix;
  } else {
    shared_ptr<BytesRef> bytes =
        make_shared<BytesRef>(prefix->bytes->length + output->bytes->length);
    System::arraycopy(prefix->bytes->bytes, prefix->bytes->offset, bytes->bytes,
                      0, prefix->bytes->length);
    System::arraycopy(output->bytes->bytes, output->bytes->offset, bytes->bytes,
                      prefix->bytes->length, output->bytes->length);
    bytes->length = prefix->bytes->length + output->bytes->length;
    return newOutput(bytes, prefix->startOrd + output->startOrd,
                     prefix->endOrd + output->endOrd);
  }
}

void FSTOrdsOutputs::write(shared_ptr<Output> prefix,
                           shared_ptr<DataOutput> out) 
{
  out->writeVInt(prefix->bytes->length);
  out->writeBytes(prefix->bytes->bytes, prefix->bytes->offset,
                  prefix->bytes->length);
  out->writeVLong(prefix->startOrd);
  out->writeVLong(prefix->endOrd);
}

shared_ptr<Output>
FSTOrdsOutputs::read(shared_ptr<DataInput> in_) 
{
  int len = in_->readVInt();
  shared_ptr<BytesRef> bytes;
  if (len == 0) {
    bytes = NO_BYTES;
  } else {
    bytes = make_shared<BytesRef>(len);
    in_->readBytes(bytes->bytes, 0, len);
    bytes->length = len;
  }

  int64_t startOrd = in_->readVLong();
  int64_t endOrd = in_->readVLong();

  shared_ptr<Output> result = newOutput(bytes, startOrd, endOrd);

  return result;
}

void FSTOrdsOutputs::skipOutput(shared_ptr<DataInput> in_) 
{
  int len = in_->readVInt();
  in_->skipBytes(len);
  in_->readVLong();
  in_->readVLong();
}

void FSTOrdsOutputs::skipFinalOutput(shared_ptr<DataInput> in_) throw(
    IOException)
{
  skipOutput(in_);
}

shared_ptr<Output> FSTOrdsOutputs::getNoOutput() { return NO_OUTPUT; }

wstring FSTOrdsOutputs::outputToString(shared_ptr<Output> output)
{
  if ((output->endOrd == 0 ||
       output->endOrd == numeric_limits<int64_t>::max()) &&
      output->startOrd == 0) {
    return L"";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return output->toString();
  }
}

shared_ptr<Output> FSTOrdsOutputs::newOutput(shared_ptr<BytesRef> bytes,
                                             int64_t startOrd,
                                             int64_t endOrd)
{
  if (bytes->length == 0 && startOrd == 0 && endOrd == 0) {
    return NO_OUTPUT;
  } else {
    return make_shared<Output>(bytes, startOrd, endOrd);
  }
}

int64_t FSTOrdsOutputs::ramBytesUsed(shared_ptr<Output> output)
{
  return 2 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + 2 * Long::BYTES +
         2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
         RamUsageEstimator::NUM_BYTES_ARRAY_HEADER + 2 * Integer::BYTES +
         output->bytes->length;
}
} // namespace org::apache::lucene::codecs::blocktreeords