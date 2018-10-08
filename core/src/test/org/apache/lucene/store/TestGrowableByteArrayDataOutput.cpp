using namespace std;

#include "TestGrowableByteArrayDataOutput.h"

namespace org::apache::lucene::store
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriteSmallStrings() throws Exception
void TestGrowableByteArrayDataOutput::testWriteSmallStrings() throw(
    runtime_error)
{
  int minSizeForDoublePass =
      GrowableByteArrayDataOutput::MIN_UTF8_SIZE_TO_ENABLE_DOUBLE_PASS_ENCODING;

  // a simple string encoding test
  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    // create a small string such that the single pass approach is used
    int length = TestUtil::nextInt(random(), 1, minSizeForDoublePass - 1);
    wstring unicode =
        TestUtil::randomFixedByteLengthUnicodeString(random(), length);
    std::deque<char> utf8(UnicodeUtil::maxUTF8Length(unicode.length()));
    int len = UnicodeUtil::UTF16toUTF8(unicode, 0, unicode.length(), utf8);

    shared_ptr<GrowableByteArrayDataOutput> dataOutput =
        make_shared<GrowableByteArrayDataOutput>(1 << 8);
    // explicitly write utf8 len so that we know how many bytes it occupies
    dataOutput->writeVInt(len);
    int vintLen = dataOutput->getPosition();
    // now write the string which will internally write number of bytes as a
    // vint and then utf8 bytes
    dataOutput->writeString(unicode);

    assertEquals(
        L"GrowableByteArrayDataOutput wrote the wrong length after encode",
        len + vintLen * 2, dataOutput->getPosition());
    for (int j = 0, k = vintLen * 2; j < len; j++, k++) {
      TestUtil::assertEquals(utf8[j], dataOutput->getBytes()[k]);
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWriteLargeStrings() throws Exception
void TestGrowableByteArrayDataOutput::testWriteLargeStrings() throw(
    runtime_error)
{
  int minSizeForDoublePass =
      GrowableByteArrayDataOutput::MIN_UTF8_SIZE_TO_ENABLE_DOUBLE_PASS_ENCODING;

  int num = atLeast(100);
  for (int i = 0; i < num; i++) {
    wstring unicode = TestUtil::randomRealisticUnicodeString(
        random(), minSizeForDoublePass, 10 * minSizeForDoublePass);
    std::deque<char> utf8(UnicodeUtil::maxUTF8Length(unicode.length()));
    int len = UnicodeUtil::UTF16toUTF8(unicode, 0, unicode.length(), utf8);

    shared_ptr<GrowableByteArrayDataOutput> dataOutput =
        make_shared<GrowableByteArrayDataOutput>(1 << 8);
    // explicitly write utf8 len so that we know how many bytes it occupies
    dataOutput->writeVInt(len);
    int vintLen = dataOutput->getPosition();
    // now write the string which will internally write number of bytes as a
    // vint and then utf8 bytes
    dataOutput->writeString(unicode);

    assertEquals(
        L"GrowableByteArrayDataOutput wrote the wrong length after encode",
        len + vintLen * 2, dataOutput->getPosition());
    for (int j = 0, k = vintLen * 2; j < len; j++, k++) {
      TestUtil::assertEquals(utf8[j], dataOutput->getBytes()[k]);
    }
  }
}
} // namespace org::apache::lucene::store