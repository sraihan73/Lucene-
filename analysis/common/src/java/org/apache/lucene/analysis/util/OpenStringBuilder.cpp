using namespace std;

#include "OpenStringBuilder.h"

namespace org::apache::lucene::analysis::util
{

OpenStringBuilder::OpenStringBuilder() : OpenStringBuilder(32) {}

OpenStringBuilder::OpenStringBuilder(int size)
{
  buf = std::deque<wchar_t>(size);
}

OpenStringBuilder::OpenStringBuilder(std::deque<wchar_t> &arr, int len)
{
  set(arr, len);
}

void OpenStringBuilder::setLength(int len) { this->len = len; }

void OpenStringBuilder::set(std::deque<wchar_t> &arr, int end)
{
  this->buf = arr;
  this->len = end;
}

std::deque<wchar_t> OpenStringBuilder::getArray() { return buf; }

int OpenStringBuilder::size() { return len; }

int OpenStringBuilder::length() { return len; }

int OpenStringBuilder::capacity() { return buf.size(); }

shared_ptr<Appendable> OpenStringBuilder::append(shared_ptr<std::wstring> csq)
{
  return append(csq, 0, csq->length());
}

shared_ptr<Appendable> OpenStringBuilder::append(shared_ptr<std::wstring> csq,
                                                 int start, int end)
{
  reserve(end - start);
  for (int i = start; i < end; i++) {
    unsafeWrite(csq->charAt(i));
  }
  return shared_from_this();
}

shared_ptr<Appendable> OpenStringBuilder::append(wchar_t c)
{
  write(c);
  return shared_from_this();
}

wchar_t OpenStringBuilder::charAt(int index) { return buf[index]; }

void OpenStringBuilder::setCharAt(int index, wchar_t ch) { buf[index] = ch; }

shared_ptr<std::wstring> OpenStringBuilder::subSequence(int start, int end)
{
  throw make_shared<UnsupportedOperationException>(); // todo
}

void OpenStringBuilder::unsafeWrite(wchar_t b) { buf[len++] = b; }

void OpenStringBuilder::unsafeWrite(int b)
{
  unsafeWrite(static_cast<wchar_t>(b));
}

void OpenStringBuilder::unsafeWrite(std::deque<wchar_t> &b, int off, int len)
{
  System::arraycopy(b, off, buf, this->len, len);
  this->len += len;
}

void OpenStringBuilder::resize(int len)
{
  std::deque<wchar_t> newbuf(max(buf.size() << 1, len));
  System::arraycopy(buf, 0, newbuf, 0, size());
  buf = newbuf;
}

void OpenStringBuilder::reserve(int num)
{
  if (len + num > buf.size()) {
    resize(len + num);
  }
}

void OpenStringBuilder::write(wchar_t b)
{
  if (len >= buf.size()) {
    resize(len + 1);
  }
  unsafeWrite(b);
}

void OpenStringBuilder::write(int b) { write(static_cast<wchar_t>(b)); }

void OpenStringBuilder::write(std::deque<wchar_t> &b)
{
  write(b, 0, b.size());
}

void OpenStringBuilder::write(std::deque<wchar_t> &b, int off, int len)
{
  reserve(len);
  unsafeWrite(b, off, len);
}

void OpenStringBuilder::write(shared_ptr<OpenStringBuilder> arr)
{
  write(arr->buf, 0, len);
}

void OpenStringBuilder::write(const wstring &s)
{
  reserve(s.length());
  s.getChars(0, s.length(), buf, len);
  len += s.length();
}

void OpenStringBuilder::flush() {}

void OpenStringBuilder::reset() { len = 0; }

std::deque<wchar_t> OpenStringBuilder::toCharArray()
{
  std::deque<wchar_t> newbuf(size());
  System::arraycopy(buf, 0, newbuf, 0, size());
  return newbuf;
}

wstring OpenStringBuilder::toString() { return wstring(buf, 0, size()); }
} // namespace org::apache::lucene::analysis::util