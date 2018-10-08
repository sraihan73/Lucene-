using namespace std;

#include "TestCharacterUtils.h"

namespace org::apache::lucene::analysis
{
using CharacterBuffer =
    org::apache::lucene::analysis::CharacterUtils::CharacterBuffer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

void TestCharacterUtils::testConversions()
{
  const std::deque<wchar_t> orig =
      TestUtil::randomUnicodeString(random(), 100).toCharArray();
  const std::deque<int> buf = std::deque<int>(orig.size());
  const std::deque<wchar_t> restored = std::deque<wchar_t>(buf.size());
  constexpr int o1 = TestUtil::nextInt(random(), 0, min(5, orig.size()));
  constexpr int o2 = TestUtil::nextInt(random(), 0, o1);
  constexpr int o3 = TestUtil::nextInt(random(), 0, o1);
  constexpr int codePointCount =
      CharacterUtils::toCodePoints(orig, o1, orig.size() - o1, buf, o2);
  constexpr int charCount =
      CharacterUtils::toChars(buf, o2, codePointCount, restored, o3);
  TestUtil::assertEquals(orig.size() - o1, charCount);
  assertArrayEquals(Arrays::copyOfRange(orig, o1, o1 + charCount),
                    Arrays::copyOfRange(restored, o3, o3 + charCount));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNewCharacterBuffer()
void TestCharacterUtils::testNewCharacterBuffer()
{
  shared_ptr<CharacterBuffer> newCharacterBuffer =
      CharacterUtils::newCharacterBuffer(1024);
  TestUtil::assertEquals(1024, newCharacterBuffer->getBuffer().size());
  TestUtil::assertEquals(0, newCharacterBuffer->getOffset());
  TestUtil::assertEquals(0, newCharacterBuffer->getLength());

  newCharacterBuffer = CharacterUtils::newCharacterBuffer(2);
  TestUtil::assertEquals(2, newCharacterBuffer->getBuffer().size());
  TestUtil::assertEquals(0, newCharacterBuffer->getOffset());
  TestUtil::assertEquals(0, newCharacterBuffer->getLength());

  // length must be >= 2
  expectThrows(invalid_argument::typeid,
               [&]() { CharacterUtils::newCharacterBuffer(1); });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFillNoHighSurrogate() throws
// java.io.IOException
void TestCharacterUtils::testFillNoHighSurrogate() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"helloworld");
  shared_ptr<CharacterBuffer> buffer = CharacterUtils::newCharacterBuffer(6);
  assertTrue(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(0, buffer->getOffset());
  TestUtil::assertEquals(6, buffer->getLength());
  TestUtil::assertEquals(L"hellow", wstring(buffer->getBuffer()));
  assertFalse(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(4, buffer->getLength());
  TestUtil::assertEquals(0, buffer->getOffset());

  TestUtil::assertEquals(
      L"orld",
      wstring(buffer->getBuffer(), buffer->getOffset(), buffer->getLength()));
  assertFalse(CharacterUtils::fill(buffer, reader));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFill() throws java.io.IOException
void TestCharacterUtils::testFill() 
{
  wstring input = L"1234\ud801\udc1c789123\ud801\ud801\udc1c\ud801";
  shared_ptr<Reader> reader = make_shared<StringReader>(input);
  shared_ptr<CharacterBuffer> buffer = CharacterUtils::newCharacterBuffer(5);
  assertTrue(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(4, buffer->getLength());
  TestUtil::assertEquals(
      L"1234",
      wstring(buffer->getBuffer(), buffer->getOffset(), buffer->getLength()));
  assertTrue(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(5, buffer->getLength());
  TestUtil::assertEquals(L"\ud801\udc1c789", wstring(buffer->getBuffer()));
  assertTrue(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(4, buffer->getLength());
  TestUtil::assertEquals(
      L"123\ud801",
      wstring(buffer->getBuffer(), buffer->getOffset(), buffer->getLength()));
  assertFalse(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(3, buffer->getLength());
  TestUtil::assertEquals(
      L"\ud801\udc1c\ud801",
      wstring(buffer->getBuffer(), buffer->getOffset(), buffer->getLength()));
  assertFalse(CharacterUtils::fill(buffer, reader));
  TestUtil::assertEquals(0, buffer->getLength());
}
} // namespace org::apache::lucene::analysis