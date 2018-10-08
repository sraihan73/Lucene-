using namespace std;

#include "CharTermAttributeImpl.h"
#include "../../util/AttributeReflector.h"
#include "../../util/BytesRef.h"
#include "../../util/BytesRefBuilder.h"
#include "../../util/FutureObjects.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FutureObjects = org::apache::lucene::util::FutureObjects;
int CharTermAttributeImpl::MIN_BUFFER_SIZE = 10;

CharTermAttributeImpl::CharTermAttributeImpl() {}

void CharTermAttributeImpl::copyBuffer(std::deque<wchar_t> &buffer, int offset,
                                       int length)
{
  growTermBuffer(length);
  System::arraycopy(buffer, offset, termBuffer, 0, length);
  termLength = length;
}

std::deque<wchar_t> CharTermAttributeImpl::buffer() { return termBuffer; }

std::deque<wchar_t> CharTermAttributeImpl::resizeBuffer(int newSize)
{
  if (termBuffer.size() < newSize) {
    // Not big enough; create a new array with slight
    // over allocation and preserve content
    const std::deque<wchar_t> newCharBuffer =
        std::deque<wchar_t>(ArrayUtil::oversize(newSize, Character::BYTES));
    System::arraycopy(termBuffer, 0, newCharBuffer, 0, termBuffer.size());
    termBuffer = newCharBuffer;
  }
  return termBuffer;
}

void CharTermAttributeImpl::growTermBuffer(int newSize)
{
  if (termBuffer.size() < newSize) {
    // Not big enough; create a new array with slight
    // over allocation:
    termBuffer =
        std::deque<wchar_t>(ArrayUtil::oversize(newSize, Character::BYTES));
  }
}

shared_ptr<CharTermAttribute> CharTermAttributeImpl::setLength(int length)
{
  FutureObjects::checkFromIndexSize(0, length, termBuffer.size());
  termLength = length;
  return shared_from_this();
}

shared_ptr<CharTermAttribute> CharTermAttributeImpl::setEmpty()
{
  termLength = 0;
  return shared_from_this();
}

shared_ptr<BytesRef> CharTermAttributeImpl::getBytesRef()
{
  builder->copyChars(termBuffer, 0, termLength);
  return builder->get();
}

int CharTermAttributeImpl::length() { return termLength; }

wchar_t CharTermAttributeImpl::charAt(int index)
{
  FutureObjects::checkIndex(index, termLength);
  return termBuffer[index];
}

shared_ptr<std::wstring> CharTermAttributeImpl::subSequence(int const start,
                                                            int const end)
{
  FutureObjects::checkFromToIndex(start, end, termLength);
  return wstring(termBuffer, start, end - start);
}

shared_ptr<CharTermAttribute>
CharTermAttributeImpl::append(shared_ptr<std::wstring> csq)
{
  if (csq == nullptr) // needed for Appendable compliance
  {
    return appendNull();
  }
  return append(csq, 0, csq->length());
}

shared_ptr<CharTermAttribute>
CharTermAttributeImpl::append(shared_ptr<std::wstring> csq, int start, int end)
{
  if (csq == nullptr) // needed for Appendable compliance
  {
    csq = L"null";
  }
  // TODO: the optimized cases (jdk methods) will already do such checks, maybe
  // re-organize this?
  FutureObjects::checkFromToIndex(start, end, csq->length());
  constexpr int len = end - start;
  if (len == 0) {
    return shared_from_this();
  }
  resizeBuffer(termLength + len);
  if (len > 4) { // only use instanceof check series for longer CSQs, else
                 // simply iterate
    if (dynamic_cast<wstring>(csq) != nullptr) {
      (static_cast<wstring>(csq))->getChars(start, end, termBuffer, termLength);
    } else if (std::dynamic_pointer_cast<StringBuilder>(csq) != nullptr) {
      (std::static_pointer_cast<StringBuilder>(csq))
          ->getChars(start, end, termBuffer, termLength);
    } else if (std::dynamic_pointer_cast<CharTermAttribute>(csq) != nullptr) {
      System::arraycopy(
          (std::static_pointer_cast<CharTermAttribute>(csq))->buffer(), start,
          termBuffer, termLength, len);
    } else if (std::dynamic_pointer_cast<CharBuffer>(csq) != nullptr &&
               (std::static_pointer_cast<CharBuffer>(csq))->hasArray()) {
      shared_ptr<CharBuffer> *const cb =
          std::static_pointer_cast<CharBuffer>(csq);
      System::arraycopy(cb->array_(),
                        cb->arrayOffset() + cb->position() + start, termBuffer,
                        termLength, len);
    } else if (std::dynamic_pointer_cast<StringBuilder>(csq) != nullptr) {
      (std::static_pointer_cast<StringBuilder>(csq))
          ->getChars(start, end, termBuffer, termLength);
    } else {
      while (start < end) {
        termBuffer[termLength++] = csq->charAt(start++);
      }
      // no fall-through here, as termLength is updated!
      return shared_from_this();
    }
    termLength += len;
    return shared_from_this();
  } else {
    while (start < end) {
      termBuffer[termLength++] = csq->charAt(start++);
    }
    return shared_from_this();
  }
}

shared_ptr<CharTermAttribute> CharTermAttributeImpl::append(wchar_t c)
{
  resizeBuffer(termLength + 1)[termLength++] = c;
  return shared_from_this();
}

shared_ptr<CharTermAttribute> CharTermAttributeImpl::append(const wstring &s)
{
  if (s == L"") // needed for Appendable compliance
  {
    return appendNull();
  }
  constexpr int len = s.length();
  s.getChars(0, len, resizeBuffer(termLength + len), termLength);
  termLength += len;
  return shared_from_this();
}

shared_ptr<CharTermAttribute>
CharTermAttributeImpl::append(shared_ptr<StringBuilder> s)
{
  if (s == nullptr) // needed for Appendable compliance
  {
    return appendNull();
  }
  constexpr int len = s->length();
  s->getChars(0, len, resizeBuffer(termLength + len), termLength);
  termLength += len;
  return shared_from_this();
}

shared_ptr<CharTermAttribute>
CharTermAttributeImpl::append(shared_ptr<CharTermAttribute> ta)
{
  if (ta == nullptr) // needed for Appendable compliance
  {
    return appendNull();
  }
  constexpr int len = ta->length();
  System::arraycopy(ta->buffer(), 0, resizeBuffer(termLength + len), termLength,
                    len);
  termLength += len;
  return shared_from_this();
}

shared_ptr<CharTermAttribute> CharTermAttributeImpl::appendNull()
{
  resizeBuffer(termLength + 4);
  termBuffer[termLength++] = L'n';
  termBuffer[termLength++] = L'u';
  termBuffer[termLength++] = L'l';
  termBuffer[termLength++] = L'l';
  return shared_from_this();
}

int CharTermAttributeImpl::hashCode()
{
  int code = termLength;
  code = code * 31 + ArrayUtil::hashCode(termBuffer, 0, termLength);
  return code;
}

void CharTermAttributeImpl::clear() { termLength = 0; }

shared_ptr<CharTermAttributeImpl> CharTermAttributeImpl::clone()
{
  shared_ptr<CharTermAttributeImpl> t =
      std::static_pointer_cast<CharTermAttributeImpl>(AttributeImpl::clone());
  // Do a deep clone
  t->termBuffer = std::deque<wchar_t>(this->termLength);
  System::arraycopy(this->termBuffer, 0, t->termBuffer, 0, this->termLength);
  t->builder = make_shared<BytesRefBuilder>();
  t->builder->copyBytes(builder->get());
  return t;
}

bool CharTermAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<CharTermAttributeImpl>(other) != nullptr) {
    shared_ptr<CharTermAttributeImpl> *const o =
        (any_cast<std::shared_ptr<CharTermAttributeImpl>>(other));
    if (termLength != o->termLength) {
      return false;
    }
    for (int i = 0; i < termLength; i++) {
      if (termBuffer[i] != o->termBuffer[i]) {
        return false;
      }
    }
    return true;
  }

  return false;
}

wstring CharTermAttributeImpl::toString()
{
  return wstring(termBuffer, 0, termLength);
}

void CharTermAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(CharTermAttribute::typeid, L"term", toString());
  reflector(TermToBytesRefAttribute::typeid, L"bytes", getBytesRef());
}

void CharTermAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<CharTermAttribute> t =
      std::static_pointer_cast<CharTermAttribute>(target);
  t->copyBuffer(termBuffer, 0, termLength);
}
} // namespace org::apache::lucene::analysis::tokenattributes