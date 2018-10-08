using namespace std;

#include "IntSequenceOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IntsRef = org::apache::lucene::util::IntsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const shared_ptr<org::apache::lucene::util::IntsRef>
    IntSequenceOutputs::NO_OUTPUT =
        make_shared<org::apache::lucene::util::IntsRef>();
const shared_ptr<IntSequenceOutputs> IntSequenceOutputs::singleton =
    make_shared<IntSequenceOutputs>();

IntSequenceOutputs::IntSequenceOutputs() {}

shared_ptr<IntSequenceOutputs> IntSequenceOutputs::getSingleton()
{
  return singleton;
}

shared_ptr<IntsRef> IntSequenceOutputs::common(shared_ptr<IntsRef> output1,
                                               shared_ptr<IntsRef> output2)
{
  assert(output1 != nullptr);
  assert(output2 != nullptr);

  int pos1 = output1->offset;
  int pos2 = output2->offset;
  int stopAt1 = pos1 + min(output1->length, output2->length);
  while (pos1 < stopAt1) {
    if (output1->ints[pos1] != output2->ints[pos2]) {
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
    return make_shared<IntsRef>(output1->ints, output1->offset,
                                pos1 - output1->offset);
  }
}

shared_ptr<IntsRef> IntSequenceOutputs::subtract(shared_ptr<IntsRef> output,
                                                 shared_ptr<IntsRef> inc)
{
  assert(output != nullptr);
  assert(inc != nullptr);
  if (inc == NO_OUTPUT) {
    // no prefix removed
    return output;
  } else if (inc->length == output->length) {
    // entire output removed
    return NO_OUTPUT;
  } else {
    assert((inc->length < output->length,
            L"inc.length=" + to_wstring(inc->length) + L" vs output.length=" +
                to_wstring(output->length)));
    assert(inc->length > 0);
    return make_shared<IntsRef>(output->ints, output->offset + inc->length,
                                output->length - inc->length);
  }
}

shared_ptr<IntsRef> IntSequenceOutputs::add(shared_ptr<IntsRef> prefix,
                                            shared_ptr<IntsRef> output)
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
    shared_ptr<IntsRef> result =
        make_shared<IntsRef>(prefix->length + output->length);
    System::arraycopy(prefix->ints, prefix->offset, result->ints, 0,
                      prefix->length);
    System::arraycopy(output->ints, output->offset, result->ints,
                      prefix->length, output->length);
    result->length = prefix->length + output->length;
    return result;
  }
}

void IntSequenceOutputs::write(shared_ptr<IntsRef> prefix,
                               shared_ptr<DataOutput> out) 
{
  assert(prefix != nullptr);
  out->writeVInt(prefix->length);
  for (int idx = 0; idx < prefix->length; idx++) {
    out->writeVInt(prefix->ints[prefix->offset + idx]);
  }
}

shared_ptr<IntsRef>
IntSequenceOutputs::read(shared_ptr<DataInput> in_) 
{
  constexpr int len = in_->readVInt();
  if (len == 0) {
    return NO_OUTPUT;
  } else {
    shared_ptr<IntsRef> *const output = make_shared<IntsRef>(len);
    for (int idx = 0; idx < len; idx++) {
      output->ints[idx] = in_->readVInt();
    }
    output->length = len;
    return output;
  }
}

void IntSequenceOutputs::skipOutput(shared_ptr<DataInput> in_) throw(
    IOException)
{
  constexpr int len = in_->readVInt();
  if (len == 0) {
    return;
  }
  for (int idx = 0; idx < len; idx++) {
    in_->readVInt();
  }
}

shared_ptr<IntsRef> IntSequenceOutputs::getNoOutput() { return NO_OUTPUT; }

wstring IntSequenceOutputs::outputToString(shared_ptr<IntsRef> output)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return output->toString();
}

int64_t IntSequenceOutputs::ramBytesUsed(shared_ptr<IntsRef> output)
{
  return BASE_NUM_BYTES + RamUsageEstimator::sizeOf(output->ints);
}

wstring IntSequenceOutputs::toString() { return L"IntSequenceOutputs"; }
} // namespace org::apache::lucene::util::fst