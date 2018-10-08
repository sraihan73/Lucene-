using namespace std;

#include "ByteSequenceOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
const shared_ptr<org::apache::lucene::util::BytesRef>
    ByteSequenceOutputs::NO_OUTPUT =
        make_shared<org::apache::lucene::util::BytesRef>();
const shared_ptr<ByteSequenceOutputs> ByteSequenceOutputs::singleton =
    make_shared<ByteSequenceOutputs>();

ByteSequenceOutputs::ByteSequenceOutputs() {}

shared_ptr<ByteSequenceOutputs> ByteSequenceOutputs::getSingleton()
{
  return singleton;
}

shared_ptr<BytesRef> ByteSequenceOutputs::common(shared_ptr<BytesRef> output1,
                                                 shared_ptr<BytesRef> output2)
{
  assert(output1 != nullptr);
  assert(output2 != nullptr);

  int pos1 = output1->offset;
  int pos2 = output2->offset;
  int stopAt1 = pos1 + min(output1->length, output2->length);
  while (pos1 < stopAt1) {
    if (output1->bytes[pos1] != output2->bytes[pos2]) {
      break;
    }
    pos1++;
    pos2++;
  }

  if (pos1 == output1->offset) {
    // no common prefix
    return NO_OUTPUT;
  } else if (pos1 == output1->offset + output1->length) {
    // output1 is a prefix of output2
    return output1;
  } else if (pos2 == output2->offset + output2->length) {
    // output2 is a prefix of output1
    return output2;
  } else {
    return make_shared<BytesRef>(output1->bytes, output1->offset,
                                 pos1 - output1->offset);
  }
}

shared_ptr<BytesRef> ByteSequenceOutputs::subtract(shared_ptr<BytesRef> output,
                                                   shared_ptr<BytesRef> inc)
{
  assert(output != nullptr);
  assert(inc != nullptr);
  if (inc == NO_OUTPUT) {
    // no prefix removed
    return output;
  } else {
    assert((StringHelper::startsWith(output, inc)));
    if (inc->length == output->length) {
      // entire output removed
      return NO_OUTPUT;
    } else {
      assert((inc->length < output->length,
              L"inc.length=" + to_wstring(inc->length) + L" vs output.length=" +
                  to_wstring(output->length)));
      assert(inc->length > 0);
      return make_shared<BytesRef>(output->bytes, output->offset + inc->length,
                                   output->length - inc->length);
    }
  }
}

shared_ptr<BytesRef> ByteSequenceOutputs::add(shared_ptr<BytesRef> prefix,
                                              shared_ptr<BytesRef> output)
{
  assert(prefix != nullptr);
  assert(output != nullptr);
  if (prefix == NO_OUTPUT) {
    return output;
  } else if (output == NO_OUTPUT) {
    return prefix;
  } else {
    assert(prefix->length > 0);
    assert(output->length > 0);
    shared_ptr<BytesRef> result =
        make_shared<BytesRef>(prefix->length + output->length);
    System::arraycopy(prefix->bytes, prefix->offset, result->bytes, 0,
                      prefix->length);
    System::arraycopy(output->bytes, output->offset, result->bytes,
                      prefix->length, output->length);
    result->length = prefix->length + output->length;
    return result;
  }
}

void ByteSequenceOutputs::write(shared_ptr<BytesRef> prefix,
                                shared_ptr<DataOutput> out) 
{
  assert(prefix != nullptr);
  out->writeVInt(prefix->length);
  out->writeBytes(prefix->bytes, prefix->offset, prefix->length);
}

shared_ptr<BytesRef>
ByteSequenceOutputs::read(shared_ptr<DataInput> in_) 
{
  constexpr int len = in_->readVInt();
  if (len == 0) {
    return NO_OUTPUT;
  } else {
    shared_ptr<BytesRef> *const output = make_shared<BytesRef>(len);
    in_->readBytes(output->bytes, 0, len);
    output->length = len;
    return output;
  }
}

void ByteSequenceOutputs::skipOutput(shared_ptr<DataInput> in_) throw(
    IOException)
{
  constexpr int len = in_->readVInt();
  if (len != 0) {
    in_->skipBytes(len);
  }
}

shared_ptr<BytesRef> ByteSequenceOutputs::getNoOutput() { return NO_OUTPUT; }

wstring ByteSequenceOutputs::outputToString(shared_ptr<BytesRef> output)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return output->toString();
}

int64_t ByteSequenceOutputs::ramBytesUsed(shared_ptr<BytesRef> output)
{
  return BASE_NUM_BYTES + RamUsageEstimator::sizeOf(output->bytes);
}

wstring ByteSequenceOutputs::toString() { return L"ByteSequenceOutputs"; }
} // namespace org::apache::lucene::util::fst