using namespace std;

#include "CharSequenceOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using CharsRef = org::apache::lucene::util::CharsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const shared_ptr<org::apache::lucene::util::CharsRef>
    CharSequenceOutputs::NO_OUTPUT =
        make_shared<org::apache::lucene::util::CharsRef>();
const shared_ptr<CharSequenceOutputs> CharSequenceOutputs::singleton =
    make_shared<CharSequenceOutputs>();

CharSequenceOutputs::CharSequenceOutputs() {}

shared_ptr<CharSequenceOutputs> CharSequenceOutputs::getSingleton()
{
  return singleton;
}

shared_ptr<CharsRef> CharSequenceOutputs::common(shared_ptr<CharsRef> output1,
                                                 shared_ptr<CharsRef> output2)
{
  assert(output1 != nullptr);
  assert(output2 != nullptr);

  int pos1 = output1->offset;
  int pos2 = output2->offset;
  int stopAt1 = pos1 + min(output1->length_, output2->length_);
  while (pos1 < stopAt1) {
    if (output1->chars[pos1] != output2->chars[pos2]) {
      break;
    }
    pos1++;
    pos2++;
  }

  if (pos1 == output1->offset) {
    // no common prefix
    return NO_OUTPUT;
  } else if (pos1 == output1->offset + output1->length_) {
    // output1 is a prefix of output2
    return output1;
  } else if (pos2 == output2->offset + output2->length_) {
    // output2 is a prefix of output1
    return output2;
  } else {
    return make_shared<CharsRef>(output1->chars, output1->offset,
                                 pos1 - output1->offset);
  }
}

shared_ptr<CharsRef> CharSequenceOutputs::subtract(shared_ptr<CharsRef> output,
                                                   shared_ptr<CharsRef> inc)
{
  assert(output != nullptr);
  assert(inc != nullptr);
  if (inc == NO_OUTPUT) {
    // no prefix removed
    return output;
  } else if (inc->length_ == output->length_) {
    // entire output removed
    return NO_OUTPUT;
  } else {
    assert((inc->length_ < output->length_, L"inc.length=" + inc->length_ +
                                                L" vs output.length=" +
                                                output->length_));
    assert(inc->length_ > 0);
    return make_shared<CharsRef>(output->chars, output->offset + inc->length_,
                                 output->length_ - inc->length_);
  }
}

shared_ptr<CharsRef> CharSequenceOutputs::add(shared_ptr<CharsRef> prefix,
                                              shared_ptr<CharsRef> output)
{
  assert(prefix != nullptr);
  assert(output != nullptr);
  if (prefix == NO_OUTPUT) {
    return output;
  } else if (output == NO_OUTPUT) {
    return prefix;
  } else {
    assert(prefix->length_ > 0);
    assert(output->length_ > 0);
    shared_ptr<CharsRef> result =
        make_shared<CharsRef>(prefix->length_ + output->length_);
    System::arraycopy(prefix->chars, prefix->offset, result->chars, 0,
                      prefix->length_);
    System::arraycopy(output->chars, output->offset, result->chars,
                      prefix->length_, output->length_);
    result->length_ = prefix->length_ + output->length_;
    return result;
  }
}

void CharSequenceOutputs::write(shared_ptr<CharsRef> prefix,
                                shared_ptr<DataOutput> out) 
{
  assert(prefix != nullptr);
  out->writeVInt(prefix->length_);
  // TODO: maybe UTF8?
  for (int idx = 0; idx < prefix->length_; idx++) {
    out->writeVInt(prefix->chars[prefix->offset + idx]);
  }
}

shared_ptr<CharsRef>
CharSequenceOutputs::read(shared_ptr<DataInput> in_) 
{
  constexpr int len = in_->readVInt();
  if (len == 0) {
    return NO_OUTPUT;
  } else {
    shared_ptr<CharsRef> *const output = make_shared<CharsRef>(len);
    for (int idx = 0; idx < len; idx++) {
      output->chars[idx] = static_cast<wchar_t>(in_->readVInt());
    }
    output->length_ = len;
    return output;
  }
}

void CharSequenceOutputs::skipOutput(shared_ptr<DataInput> in_) throw(
    IOException)
{
  constexpr int len = in_->readVInt();
  for (int idx = 0; idx < len; idx++) {
    in_->readVInt();
  }
}

shared_ptr<CharsRef> CharSequenceOutputs::getNoOutput() { return NO_OUTPUT; }

wstring CharSequenceOutputs::outputToString(shared_ptr<CharsRef> output)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return output->toString();
}

int64_t CharSequenceOutputs::ramBytesUsed(shared_ptr<CharsRef> output)
{
  return BASE_NUM_BYTES + RamUsageEstimator::sizeOf(output->chars);
}
} // namespace org::apache::lucene::util::fst