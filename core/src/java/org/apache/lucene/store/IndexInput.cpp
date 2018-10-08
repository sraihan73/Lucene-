using namespace std;

#include "IndexInput.h"

namespace org::apache::lucene::store
{

IndexInput::IndexInput(const wstring &resourceDescription)
    : resourceDescription(resourceDescription)
{
  if (resourceDescription == L"") {
    throw invalid_argument(L"resourceDescription must not be null");
  }
}

wstring IndexInput::toString() { return resourceDescription; }

shared_ptr<IndexInput> IndexInput::clone()
{
  return std::static_pointer_cast<IndexInput>(DataInput::clone());
}

wstring IndexInput::getFullSliceDescription(const wstring &sliceDescription)
{
  if (sliceDescription == L"") {
    // Clones pass null sliceDescription:
    return toString();
  } else {
    return toString() + L" [slice=" + sliceDescription + L"]";
  }
}

shared_ptr<RandomAccessInput>
IndexInput::randomAccessSlice(int64_t offset,
                              int64_t length) 
{
  shared_ptr<IndexInput> *const slice =
      this->slice(L"randomaccess", offset, length);
  if (std::dynamic_pointer_cast<RandomAccessInput>(slice) != nullptr) {
    // slice() already supports random access
    return std::static_pointer_cast<RandomAccessInput>(slice);
  } else {
    // return default impl
    return make_shared<RandomAccessInputAnonymousInnerClass>(shared_from_this(),
                                                             slice);
  }
}

IndexInput::RandomAccessInputAnonymousInnerClass::
    RandomAccessInputAnonymousInnerClass(
        shared_ptr<IndexInput> outerInstance,
        shared_ptr<org::apache::lucene::store::IndexInput> slice)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

char IndexInput::RandomAccessInputAnonymousInnerClass::readByte(
    int64_t pos) 
{
  slice->seek(pos);
  return slice->readByte();
}

short IndexInput::RandomAccessInputAnonymousInnerClass::readShort(
    int64_t pos) 
{
  slice->seek(pos);
  return slice->readShort();
}

int IndexInput::RandomAccessInputAnonymousInnerClass::readInt(
    int64_t pos) 
{
  slice->seek(pos);
  return slice->readInt();
}

int64_t IndexInput::RandomAccessInputAnonymousInnerClass::readLong(
    int64_t pos) 
{
  slice->seek(pos);
  return slice->readLong();
}

wstring IndexInput::RandomAccessInputAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"RandomAccessInput(" + outerInstance->toString() + L")";
}
} // namespace org::apache::lucene::store