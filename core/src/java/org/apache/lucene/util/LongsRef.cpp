using namespace std;

#include "LongsRef.h"

namespace org::apache::lucene::util
{

std::deque<int64_t> const LongsRef::EMPTY_LONGS = std::deque<int64_t>(0);

LongsRef::LongsRef() { longs = EMPTY_LONGS; }

LongsRef::LongsRef(int capacity) { longs = std::deque<int64_t>(capacity); }

LongsRef::LongsRef(std::deque<int64_t> &longs, int offset, int length)
{
  this->longs = longs;
  this->offset = offset;
  this->length = length;
  assert(isValid());
}

shared_ptr<LongsRef> LongsRef::clone()
{
  return make_shared<LongsRef>(longs, offset, length);
}

int LongsRef::hashCode()
{
  constexpr int prime = 31;
  int result = 0;
  constexpr int64_t end = offset + length;
  for (int i = offset; i < end; i++) {
    result =
        prime * result +
        static_cast<int>(longs[i] ^
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(longs[i]) >> 32)));
  }
  return result;
}

bool LongsRef::equals(any other)
{
  if (other == nullptr) {
    return false;
  }
  if (std::dynamic_pointer_cast<LongsRef>(other) != nullptr) {
    return this->longsEquals(any_cast<std::shared_ptr<LongsRef>>(other));
  }
  return false;
}

bool LongsRef::longsEquals(shared_ptr<LongsRef> other)
{
  return FutureArrays::equals(this->longs, this->offset,
                              this->offset + this->length, other->longs,
                              other->offset, other->offset + other->length);
}

int LongsRef::compareTo(shared_ptr<LongsRef> other)
{
  return FutureArrays::compare(this->longs, this->offset,
                               this->offset + this->length, other->longs,
                               other->offset, other->offset + other->length);
}

wstring LongsRef::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L'[');
  constexpr int64_t end = offset + length;
  for (int i = offset; i < end; i++) {
    if (i > offset) {
      sb->append(L' ');
    }
    sb->append(Long::toHexString(longs[i]));
  }
  sb->append(L']');
  return sb->toString();
}

shared_ptr<LongsRef> LongsRef::deepCopyOf(shared_ptr<LongsRef> other)
{
  return make_shared<LongsRef>(
      Arrays::copyOfRange(other->longs, other->offset,
                          other->offset + other->length),
      0, other->length);
}

bool LongsRef::isValid()
{
  if (longs.empty()) {
    throw make_shared<IllegalStateException>(L"longs is null");
  }
  if (length < 0) {
    throw make_shared<IllegalStateException>(L"length is negative: " +
                                             to_wstring(length));
  }
  if (length > longs.size()) {
    throw make_shared<IllegalStateException>(L"length is out of bounds: " +
                                             to_wstring(length) +
                                             L",longs.length=" + longs.size());
  }
  if (offset < 0) {
    throw make_shared<IllegalStateException>(L"offset is negative: " +
                                             to_wstring(offset));
  }
  if (offset > longs.size()) {
    throw make_shared<IllegalStateException>(L"offset out of bounds: " +
                                             to_wstring(offset) +
                                             L",longs.length=" + longs.size());
  }
  if (offset + length < 0) {
    throw make_shared<IllegalStateException>(
        L"offset+length is negative: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length));
  }
  if (offset + length > longs.size()) {
    throw make_shared<IllegalStateException>(
        L"offset+length out of bounds: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length) + L",longs.length=" + longs.size());
  }
  return true;
}
} // namespace org::apache::lucene::util