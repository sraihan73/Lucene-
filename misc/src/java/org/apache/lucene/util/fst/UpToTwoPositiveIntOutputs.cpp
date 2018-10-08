using namespace std;

#include "UpToTwoPositiveIntOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

UpToTwoPositiveIntOutputs::TwoLongs::TwoLongs(int64_t first, int64_t second)
    : first(first), second(second)
{
  assert(first >= 0);
  assert(second >= 0);
}

wstring UpToTwoPositiveIntOutputs::TwoLongs::toString()
{
  return L"TwoLongs:" + to_wstring(first) + L"," + to_wstring(second);
}

bool UpToTwoPositiveIntOutputs::TwoLongs::equals(any _other)
{
  if (std::dynamic_pointer_cast<TwoLongs>(_other) != nullptr) {
    shared_ptr<TwoLongs> *const other =
        any_cast<std::shared_ptr<TwoLongs>>(_other);
    return first == other->first && second == other->second;
  } else {
    return false;
  }
}

int UpToTwoPositiveIntOutputs::TwoLongs::hashCode()
{
  return static_cast<int>(
      (first ^
       (static_cast<int64_t>(static_cast<uint64_t>(first) >> 32))) ^
      (second ^ (second >> 32)));
}

const optional<int64_t> UpToTwoPositiveIntOutputs::NO_OUTPUT =
    optional<int64_t>(0);
const shared_ptr<UpToTwoPositiveIntOutputs>
    UpToTwoPositiveIntOutputs::singletonShare =
        make_shared<UpToTwoPositiveIntOutputs>(true);
const shared_ptr<UpToTwoPositiveIntOutputs>
    UpToTwoPositiveIntOutputs::singletonNoShare =
        make_shared<UpToTwoPositiveIntOutputs>(false);

UpToTwoPositiveIntOutputs::UpToTwoPositiveIntOutputs(bool doShare)
    : doShare(doShare)
{
}

shared_ptr<UpToTwoPositiveIntOutputs>
UpToTwoPositiveIntOutputs::getSingleton(bool doShare)
{
  return doShare ? singletonShare : singletonNoShare;
}

optional<int64_t> UpToTwoPositiveIntOutputs::get(int64_t v)
{
  if (v == 0) {
    return NO_OUTPUT;
  } else {
    return static_cast<int64_t>(v);
  }
}

shared_ptr<TwoLongs> UpToTwoPositiveIntOutputs::get(int64_t first,
                                                    int64_t second)
{
  return make_shared<TwoLongs>(first, second);
}

optional<int64_t> UpToTwoPositiveIntOutputs::common(any _output1,
                                                      any _output2)
{
  assert((valid(_output1, false)));
  assert((valid(_output2, false)));
  const optional<int64_t> output1 = any_cast<optional<int64_t>>(_output1);
  const optional<int64_t> output2 = any_cast<optional<int64_t>>(_output2);
  if (output1 == NO_OUTPUT || output2 == NO_OUTPUT) {
    return NO_OUTPUT;
  } else if (doShare) {
    assert(output1 > 0);
    assert(output2 > 0);
    return min(output1, output2);
  } else if (output1 == output2) {
    return output1;
  } else {
    return NO_OUTPUT;
  }
}

optional<int64_t> UpToTwoPositiveIntOutputs::subtract(any _output, any _inc)
{
  assert((valid(_output, false)));
  assert((valid(_inc, false)));
  const optional<int64_t> output = any_cast<optional<int64_t>>(_output);
  const optional<int64_t> inc = any_cast<optional<int64_t>>(_inc);
  assert(output >= inc);

  if (inc == NO_OUTPUT) {
    return output;
  } else if (output == inc) {
    return NO_OUTPUT;
  } else {
    return output - inc;
  }
}

any UpToTwoPositiveIntOutputs::add(any _prefix, any _output)
{
  assert((valid(_prefix, false)));
  assert((valid(_output, true)));
  const optional<int64_t> prefix = any_cast<optional<int64_t>>(_prefix);
  if (dynamic_cast<optional<int64_t>>(_output) != nullptr) {
    const optional<int64_t> output = any_cast<optional<int64_t>>(_output);
    if (prefix == NO_OUTPUT) {
      return output;
    } else if (output == NO_OUTPUT) {
      return prefix;
    } else {
      return prefix + output;
    }
  } else {
    shared_ptr<TwoLongs> *const output =
        any_cast<std::shared_ptr<TwoLongs>>(_output);
    constexpr int64_t v = prefix;
    return make_shared<TwoLongs>(output->first + v, output->second + v);
  }
}

void UpToTwoPositiveIntOutputs::write(
    any _output, shared_ptr<DataOutput> out) 
{
  assert((valid(_output, true)));
  if (dynamic_cast<optional<int64_t>>(_output) != nullptr) {
    const optional<int64_t> output = any_cast<optional<int64_t>>(_output);
    out->writeVLong(output << 1);
  } else {
    shared_ptr<TwoLongs> *const output =
        any_cast<std::shared_ptr<TwoLongs>>(_output);
    out->writeVLong((output->first << 1) | 1);
    out->writeVLong(output->second);
  }
}

any UpToTwoPositiveIntOutputs::read(shared_ptr<DataInput> in_) throw(
    IOException)
{
  constexpr int64_t code = in_->readVLong();
  if ((code & 1) == 0) {
    // single long
    constexpr int64_t v =
        static_cast<int64_t>(static_cast<uint64_t>(code) >> 1);
    if (v == 0) {
      return NO_OUTPUT;
    } else {
      return static_cast<int64_t>(v);
    }
  } else {
    // two longs
    constexpr int64_t first =
        static_cast<int64_t>(static_cast<uint64_t>(code) >> 1);
    constexpr int64_t second = in_->readVLong();
    return make_shared<TwoLongs>(first, second);
  }
}

bool UpToTwoPositiveIntOutputs::valid(optional<int64_t> &o)
{
  assert(o);
  assert((std::dynamic_pointer_cast<std>(o) != nullptr, : optional<int64_t>));
  assert(o == NO_OUTPUT || o > 0);
  return true;
}

bool UpToTwoPositiveIntOutputs::valid(any _o, bool allowDouble)
{
  if (!allowDouble) {
    assert((std::dynamic_pointer_cast<std>(_o) != nullptr,
            : optional<int64_t>));
    return valid(any_cast<optional<int64_t>>(_o));
  } else if (std::dynamic_pointer_cast<TwoLongs>(_o) != nullptr) {
    return true;
  } else {
    return valid(any_cast<optional<int64_t>>(_o));
  }
}

any UpToTwoPositiveIntOutputs::getNoOutput() { return NO_OUTPUT; }

wstring UpToTwoPositiveIntOutputs::outputToString(any output)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return output.toString();
}

any UpToTwoPositiveIntOutputs::merge(any first, any second)
{
  assert((valid(first, false)));
  assert((valid(second, false)));
  return make_shared<TwoLongs>(any_cast<optional<int64_t>>(first),
                               any_cast<optional<int64_t>>(second));
}

int64_t UpToTwoPositiveIntOutputs::ramBytesUsed(any o)
{
  if (dynamic_cast<optional<int64_t>>(o) != nullptr) {
    return RamUsageEstimator::sizeOf(any_cast<optional<int64_t>>(o));
  } else {
    assert(std::dynamic_pointer_cast<TwoLongs>(o) != nullptr);
    return TWO_LONGS_NUM_BYTES;
  }
}
} // namespace org::apache::lucene::util::fst