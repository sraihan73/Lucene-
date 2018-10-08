using namespace std;

#include "PositiveIntOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const optional<int64_t> PositiveIntOutputs::NO_OUTPUT =
    optional<int64_t>(0);
const shared_ptr<PositiveIntOutputs> PositiveIntOutputs::singleton =
    make_shared<PositiveIntOutputs>();

PositiveIntOutputs::PositiveIntOutputs() {}

shared_ptr<PositiveIntOutputs> PositiveIntOutputs::getSingleton()
{
  return singleton;
}

optional<int64_t> PositiveIntOutputs::common(optional<int64_t> &output1,
                                               optional<int64_t> &output2)
{
  assert(valid(output1));
  assert(valid(output2));
  if (output1 == NO_OUTPUT || output2 == NO_OUTPUT) {
    return NO_OUTPUT;
  } else {
    assert(output1 > 0);
    assert(output2 > 0);
    return min(output1, output2);
  }
}

optional<int64_t> PositiveIntOutputs::subtract(optional<int64_t> &output,
                                                 optional<int64_t> &inc)
{
  assert(valid(output));
  assert(valid(inc));
  assert(output >= inc);

  if (inc == NO_OUTPUT) {
    return output;
  } else if (output == inc) {
    return NO_OUTPUT;
  } else {
    return output - inc;
  }
}

optional<int64_t> PositiveIntOutputs::add(optional<int64_t> &prefix,
                                            optional<int64_t> &output)
{
  assert(valid(prefix));
  assert(valid(output));
  if (prefix == NO_OUTPUT) {
    return output;
  } else if (output == NO_OUTPUT) {
    return prefix;
  } else {
    return prefix + output;
  }
}

void PositiveIntOutputs::write(optional<int64_t> &output,
                               shared_ptr<DataOutput> out) 
{
  assert(valid(output));
  out->writeVLong(output);
}

optional<int64_t>
PositiveIntOutputs::read(shared_ptr<DataInput> in_) 
{
  int64_t v = in_->readVLong();
  if (v == 0) {
    return NO_OUTPUT;
  } else {
    return v;
  }
}

bool PositiveIntOutputs::valid(optional<int64_t> &o)
{
  assert(o);
  assert((o == NO_OUTPUT || o > 0, L"o=" + o));
  return true;
}

optional<int64_t> PositiveIntOutputs::getNoOutput() { return NO_OUTPUT; }

wstring PositiveIntOutputs::outputToString(optional<int64_t> &output)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return output.toString();
}

wstring PositiveIntOutputs::toString() { return L"PositiveIntOutputs"; }

int64_t PositiveIntOutputs::ramBytesUsed(optional<int64_t> &output)
{
  return RamUsageEstimator::sizeOf(output);
}
} // namespace org::apache::lucene::util::fst