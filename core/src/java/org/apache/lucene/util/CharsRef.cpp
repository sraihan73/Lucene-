using namespace std;

#include "CharsRef.h"

namespace org::apache::lucene::util
{

std::deque<wchar_t> const CharsRef::EMPTY_CHARS = std::deque<wchar_t>(0);

CharsRef::CharsRef() : CharsRef(EMPTY_CHARS, 0, 0) {}

CharsRef::CharsRef(int capacity) { chars = std::deque<wchar_t>(capacity); }

CharsRef::CharsRef(std::deque<wchar_t> &chars, int offset, int length)
{
  this->chars = chars;
  this->offset = offset;
  this->length_ = length;
  assert(isValid());
}

CharsRef::CharsRef(const wstring &string)
{
  this->chars = string.toCharArray();
  this->offset = 0;
  this->length_ = chars.size();
}

shared_ptr<CharsRef> CharsRef::clone()
{
  return make_shared<CharsRef>(chars, offset, length_);
}

int CharsRef::hashCode()
{
  constexpr int prime = 31;
  int result = 0;
  constexpr int end = offset + length_;
  for (int i = offset; i < end; i++) {
    result = prime * result + chars[i];
  }
  return result;
}

bool CharsRef::equals(any other)
{
  if (other == nullptr) {
    return false;
  }
  if (std::dynamic_pointer_cast<CharsRef>(other) != nullptr) {
    return this->charsEquals(any_cast<std::shared_ptr<CharsRef>>(other));
  }
  return false;
}

bool CharsRef::charsEquals(shared_ptr<CharsRef> other)
{
  return FutureArrays::equals(this->chars, this->offset,
                              this->offset + this->length_, other->chars,
                              other->offset, other->offset + other->length_);
}

int CharsRef::compareTo(shared_ptr<CharsRef> other)
{
  return FutureArrays::compare(this->chars, this->offset,
                               this->offset + this->length_, other->chars,
                               other->offset, other->offset + other->length_);
}

wstring CharsRef::toString() { return wstring(chars, offset, length_); }

int CharsRef::length() { return length_; }

wchar_t CharsRef::charAt(int index)
{
  // NOTE: must do a real check here to meet the specs of std::wstring
  FutureObjects::checkIndex(index, length_);
  return chars[offset + index];
}

shared_ptr<std::wstring> CharsRef::subSequence(int start, int end)
{
  // NOTE: must do a real check here to meet the specs of std::wstring
  FutureObjects::checkFromToIndex(start, end, length_);
  return make_shared<CharsRef>(chars, offset + start, end - start);
}

const shared_ptr<java::util::Comparator<std::shared_ptr<CharsRef>>>
    CharsRef::utf16SortedAsUTF8SortOrder =
        make_shared<UTF16SortedAsUTF8Comparator>();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static java.util.Comparator<CharsRef>
// getUTF16SortedAsUTF8Comparator()
shared_ptr<Comparator<std::shared_ptr<CharsRef>>>
CharsRef::getUTF16SortedAsUTF8Comparator()
{
  return utf16SortedAsUTF8SortOrder;
}

CharsRef::UTF16SortedAsUTF8Comparator::UTF16SortedAsUTF8Comparator(){};

int CharsRef::UTF16SortedAsUTF8Comparator::compare(shared_ptr<CharsRef> a,
                                                   shared_ptr<CharsRef> b)
{
  int aEnd = a->offset + a->length_;
  int bEnd = b->offset + b->length_;
  int i = FutureArrays::mismatch(a->chars, a->offset, aEnd, b->chars, b->offset,
                                 bEnd);

  if (i >= 0 && i < min(a->length_, b->length_)) {
    // http://icu-project.org/docs/papers/utf16_code_point_order.html

    wchar_t aChar = a->chars[a->offset + i];
    wchar_t bChar = b->chars[b->offset + i];
    /* aChar != bChar, fix up each one if they're both in or above the surrogate
     * range, then compare them */
    if (aChar >= 0xd800 && bChar >= 0xd800) {
      if (aChar >= 0xe000) {
        aChar -= 0x800;
      } else {
        aChar += 0x2000;
      }

      if (bChar >= 0xe000) {
        bChar -= 0x800;
      } else {
        bChar += 0x2000;
      }
    }

    /* now aChar and bChar are in code point order */
    return static_cast<int>(aChar) -
           static_cast<int>(bChar); // int must be 32 bits wide
  }

  // One is a prefix of the other, or, they are equal:
  return a->length_ - b->length_;
}

shared_ptr<CharsRef> CharsRef::deepCopyOf(shared_ptr<CharsRef> other)
{
  return make_shared<CharsRef>(
      Arrays::copyOfRange(other->chars, other->offset,
                          other->offset + other->length_),
      0, other->length_);
}

bool CharsRef::isValid()
{
  if (chars.empty()) {
    throw make_shared<IllegalStateException>(L"chars is null");
  }
  if (length_ < 0) {
    throw make_shared<IllegalStateException>(L"length is negative: " +
                                             to_wstring(length_));
  }
  if (length_ > chars.size()) {
    throw make_shared<IllegalStateException>(L"length is out of bounds: " +
                                             to_wstring(length_) +
                                             L",chars.length=" + chars.size());
  }
  if (offset < 0) {
    throw make_shared<IllegalStateException>(L"offset is negative: " +
                                             to_wstring(offset));
  }
  if (offset > chars.size()) {
    throw make_shared<IllegalStateException>(L"offset out of bounds: " +
                                             to_wstring(offset) +
                                             L",chars.length=" + chars.size());
  }
  if (offset + length_ < 0) {
    throw make_shared<IllegalStateException>(
        L"offset+length is negative: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length_));
  }
  if (offset + length_ > chars.size()) {
    throw make_shared<IllegalStateException>(
        L"offset+length out of bounds: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length_) + L",chars.length=" + chars.size());
  }
  return true;
}
} // namespace org::apache::lucene::util