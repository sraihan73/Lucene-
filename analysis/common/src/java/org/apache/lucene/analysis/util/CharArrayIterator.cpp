using namespace std;

#include "CharArrayIterator.h"

namespace org::apache::lucene::analysis::util
{

std::deque<wchar_t> CharArrayIterator::getText() { return array_; }

int CharArrayIterator::getStart() { return start; }

int CharArrayIterator::getLength() { return length; }

void CharArrayIterator::setText(std::deque<wchar_t> &array_, int start,
                                int length)
{
  this->array_ = array_;
  this->start = start;
  this->index = start;
  this->length = length;
  this->limit = start + length;
}

wchar_t CharArrayIterator::current()
{
  return (index == limit) ? DONE : jreBugWorkaround(array_[index]);
}

wchar_t CharArrayIterator::first()
{
  index = start;
  return current();
}

int CharArrayIterator::getBeginIndex() { return 0; }

int CharArrayIterator::getEndIndex() { return length; }

int CharArrayIterator::getIndex() { return index - start; }

wchar_t CharArrayIterator::last()
{
  index = (limit == start) ? limit : limit - 1;
  return current();
}

wchar_t CharArrayIterator::next()
{
  if (++index >= limit) {
    index = limit;
    return DONE;
  } else {
    return current();
  }
}

wchar_t CharArrayIterator::previous()
{
  if (--index < start) {
    index = start;
    return DONE;
  } else {
    return current();
  }
}

wchar_t CharArrayIterator::setIndex(int position)
{
  if (position < getBeginIndex() || position > getEndIndex()) {
    throw invalid_argument(L"Illegal Position: " + to_wstring(position));
  }
  index = start + position;
  return current();
}

shared_ptr<CharArrayIterator> CharArrayIterator::clone()
{
  try {
    return std::static_pointer_cast<CharArrayIterator>(__super::clone());
  } catch (const CloneNotSupportedException &e) {
    // CharacterIterator does not allow you to throw CloneNotSupported
    throw runtime_error(e);
  }
}

shared_ptr<CharArrayIterator> CharArrayIterator::newSentenceInstance()
{
  if (HAS_BUGGY_BREAKITERATORS) {
    return make_shared<CharArrayIteratorAnonymousInnerClass>();
  } else {
    return make_shared<CharArrayIteratorAnonymousInnerClass2>();
  }
}

CharArrayIterator::CharArrayIteratorAnonymousInnerClass::
    CharArrayIteratorAnonymousInnerClass()
{
}

wchar_t
CharArrayIterator::CharArrayIteratorAnonymousInnerClass::jreBugWorkaround(
    wchar_t ch)
{
  return ch >= 0xD800 && ch <= 0xDFFF ? 0x002C : ch;
}

CharArrayIterator::CharArrayIteratorAnonymousInnerClass2::
    CharArrayIteratorAnonymousInnerClass2()
{
}

wchar_t
CharArrayIterator::CharArrayIteratorAnonymousInnerClass2::jreBugWorkaround(
    wchar_t ch)
{
  return ch;
}

shared_ptr<CharArrayIterator> CharArrayIterator::newWordInstance()
{
  if (HAS_BUGGY_BREAKITERATORS) {
    return make_shared<CharArrayIteratorAnonymousInnerClass3>();
  } else {
    return make_shared<CharArrayIteratorAnonymousInnerClass4>();
  }
}

CharArrayIterator::CharArrayIteratorAnonymousInnerClass3::
    CharArrayIteratorAnonymousInnerClass3()
{
}

wchar_t
CharArrayIterator::CharArrayIteratorAnonymousInnerClass3::jreBugWorkaround(
    wchar_t ch)
{
  return ch >= 0xD800 && ch <= 0xDFFF ? 0x0041 : ch;
}

CharArrayIterator::CharArrayIteratorAnonymousInnerClass4::
    CharArrayIteratorAnonymousInnerClass4()
{
}

wchar_t
CharArrayIterator::CharArrayIteratorAnonymousInnerClass4::jreBugWorkaround(
    wchar_t ch)
{
  return ch;
}

CharArrayIterator::StaticConstructor::StaticConstructor()
{
  bool v;
  try {
    shared_ptr<BreakIterator> bi =
        BreakIterator::getSentenceInstance(Locale::US);
    bi->setText(L"\udb40\udc53");
    bi->next();
    v = false;
  } catch (const runtime_error &e) {
    v = true;
  }
  HAS_BUGGY_BREAKITERATORS = v;
}

CharArrayIterator::StaticConstructor CharArrayIterator::staticConstructor;
} // namespace org::apache::lucene::analysis::util