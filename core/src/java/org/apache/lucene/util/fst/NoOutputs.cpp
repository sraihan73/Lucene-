using namespace std;

#include "NoOutputs.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
const any NoOutputs::NO_OUTPUT = make_shared<ObjectAnonymousInnerClass>();

NoOutputs::ObjectAnonymousInnerClass::ObjectAnonymousInnerClass() {}

int NoOutputs::ObjectAnonymousInnerClass::hashCode() { return 42; }

bool NoOutputs::ObjectAnonymousInnerClass::equals(any other)
{
  return other == shared_from_this();
}

const shared_ptr<NoOutputs> NoOutputs::singleton = make_shared<NoOutputs>();

NoOutputs::NoOutputs() {}

shared_ptr<NoOutputs> NoOutputs::getSingleton() { return singleton; }

any NoOutputs::common(any output1, any output2)
{
  assert(output1 == NO_OUTPUT);
  assert(output2 == NO_OUTPUT);
  return NO_OUTPUT;
}

any NoOutputs::subtract(any output, any inc)
{
  assert(output == NO_OUTPUT);
  assert(inc == NO_OUTPUT);
  return NO_OUTPUT;
}

any NoOutputs::add(any prefix, any output)
{
  assert((prefix == NO_OUTPUT, L"got " + prefix));
  assert(output == NO_OUTPUT);
  return NO_OUTPUT;
}

any NoOutputs::merge(any first, any second)
{
  assert(first == NO_OUTPUT);
  assert(second == NO_OUTPUT);
  return NO_OUTPUT;
}

void NoOutputs::write(any prefix, shared_ptr<DataOutput> out)
{
  // assert false;
}

any NoOutputs::read(shared_ptr<DataInput> in_)
{
  // assert false;
  // return null;
  return NO_OUTPUT;
}

any NoOutputs::getNoOutput() { return NO_OUTPUT; }

wstring NoOutputs::outputToString(any output) { return L""; }

int64_t NoOutputs::ramBytesUsed(any output) { return 0; }

wstring NoOutputs::toString() { return L"NoOutputs"; }
} // namespace org::apache::lucene::util::fst