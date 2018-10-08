using namespace std;

#include "StringMSBRadixSorter.h"

namespace org::apache::lucene::util
{

StringMSBRadixSorter::StringMSBRadixSorter()
    : MSBRadixSorter(numeric_limits<int>::max())
{
}

int StringMSBRadixSorter::byteAt(int i, int k)
{
  shared_ptr<BytesRef> ref = get(i);
  if (ref->length <= k) {
    return -1;
  }
  return ref->bytes[ref->offset + k] & 0xff;
}

shared_ptr<Sorter> StringMSBRadixSorter::getFallbackSorter(int k)
{
  return make_shared<IntroSorterAnonymousInnerClass>(shared_from_this(), k);
}

StringMSBRadixSorter::IntroSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass(
        shared_ptr<StringMSBRadixSorter> outerInstance, int k)
{
  this->outerInstance = outerInstance;
  this->k = k;
  pivot = make_shared<BytesRef>(), scratch1 = make_shared<BytesRef>(),
  scratch2 = make_shared<BytesRef>();
}

void StringMSBRadixSorter::IntroSorterAnonymousInnerClass::get(
    int i, int k, shared_ptr<BytesRef> scratch)
{
  shared_ptr<BytesRef> ref = outerInstance->get(i);
  assert(ref->length >= k);
  scratch->bytes = ref->bytes;
  scratch->offset = ref->offset + k;
  scratch->length = ref->length - k;
}

void StringMSBRadixSorter::IntroSorterAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->swap(i, j);
}

int StringMSBRadixSorter::IntroSorterAnonymousInnerClass::compare(int i, int j)
{
  get(i, k, scratch1);
  get(j, k, scratch2);
  return scratch1->compareTo(scratch2);
}

void StringMSBRadixSorter::IntroSorterAnonymousInnerClass::setPivot(int i)
{
  get(i, k, pivot);
}

int StringMSBRadixSorter::IntroSorterAnonymousInnerClass::comparePivot(int j)
{
  get(j, k, scratch2);
  return pivot->compareTo(scratch2);
}
} // namespace org::apache::lucene::util