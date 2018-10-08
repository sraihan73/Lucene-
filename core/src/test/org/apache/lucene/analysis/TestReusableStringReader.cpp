using namespace std;

#include "TestReusableStringReader.h"

namespace org::apache::lucene::analysis
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestReusableStringReader::test() 
{
  shared_ptr<ReusableStringReader> reader = make_shared<ReusableStringReader>();
  assertEquals(-1, reader->read());
  assertEquals(-1, reader->read(std::deque<wchar_t>(1)));
  assertEquals(-1, reader->read(std::deque<wchar_t>(2), 1, 1));
  assertEquals(-1, reader->read(CharBuffer::wrap(std::deque<wchar_t>(2))));

  reader->setValue(L"foobar");
  std::deque<wchar_t> buf(4);
  assertEquals(4, reader->read(buf));
  assertEquals(L"foob", wstring(buf));
  assertEquals(2, reader->read(buf));
  assertEquals(L"ar", wstring(buf, 0, 2));
  assertEquals(-1, reader->read(buf));
  reader->close();

  reader->setValue(L"foobar");
  assertEquals(0, reader->read(buf, 1, 0));
  assertEquals(3, reader->read(buf, 1, 3));
  assertEquals(L"foo", wstring(buf, 1, 3));
  assertEquals(2, reader->read(CharBuffer::wrap(buf, 2, 2)));
  assertEquals(L"ba", wstring(buf, 2, 2));
  assertEquals(L'r', static_cast<wchar_t>(reader->read()));
  assertEquals(-1, reader->read(buf));
  reader->close();

  reader->setValue(L"foobar");
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int ch;
  while ((ch = reader->read()) != -1) {
    sb->append(static_cast<wchar_t>(ch));
  }
  reader->close();
  assertEquals(L"foobar", sb->toString());
}
} // namespace org::apache::lucene::analysis