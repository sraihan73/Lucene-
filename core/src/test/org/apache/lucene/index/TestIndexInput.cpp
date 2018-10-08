using namespace std;

#include "TestIndexInput.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using org::junit::AfterClass;
using org::junit::BeforeClass;
std::deque<char> const TestIndexInput::READ_TEST_BYTES =
    std::deque<char>{static_cast<char>(0x80),
                      0x01,
                      static_cast<char>(0xFF),
                      0x7F,
                      static_cast<char>(0x80),
                      static_cast<char>(0x80),
                      0x01,
                      static_cast<char>(0x81),
                      static_cast<char>(0x80),
                      0x01,
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x07),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x0F),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x07),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x7F),
                      0x06,
                      L'L',
                      L'u',
                      L'c',
                      L'e',
                      L'n',
                      L'e',
                      0x02,
                      static_cast<char>(0xC2),
                      static_cast<char>(0xBF),
                      0x0A,
                      L'L',
                      L'u',
                      static_cast<char>(0xC2),
                      static_cast<char>(0xBF),
                      L'c',
                      L'e',
                      static_cast<char>(0xC2),
                      static_cast<char>(0xBF),
                      L'n',
                      L'e',
                      0x03,
                      static_cast<char>(0xE2),
                      static_cast<char>(0x98),
                      static_cast<char>(0xA0),
                      0x0C,
                      L'L',
                      L'u',
                      static_cast<char>(0xE2),
                      static_cast<char>(0x98),
                      static_cast<char>(0xA0),
                      L'c',
                      L'e',
                      static_cast<char>(0xE2),
                      static_cast<char>(0x98),
                      static_cast<char>(0xA0),
                      L'n',
                      L'e',
                      0x04,
                      static_cast<char>(0xF0),
                      static_cast<char>(0x9D),
                      static_cast<char>(0x84),
                      static_cast<char>(0x9E),
                      0x08,
                      static_cast<char>(0xF0),
                      static_cast<char>(0x9D),
                      static_cast<char>(0x84),
                      static_cast<char>(0x9E),
                      static_cast<char>(0xF0),
                      static_cast<char>(0x9D),
                      static_cast<char>(0x85),
                      static_cast<char>(0xA0),
                      0x0E,
                      L'L',
                      L'u',
                      static_cast<char>(0xF0),
                      static_cast<char>(0x9D),
                      static_cast<char>(0x84),
                      static_cast<char>(0x9E),
                      L'c',
                      L'e',
                      static_cast<char>(0xF0),
                      static_cast<char>(0x9D),
                      static_cast<char>(0x85),
                      static_cast<char>(0xA0),
                      L'n',
                      L'e',
                      0x01,
                      0x00,
                      0x08,
                      L'L',
                      L'u',
                      0x00,
                      L'c',
                      L'e',
                      0x00,
                      L'n',
                      L'e',
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x17),
                      static_cast<char>(0x01),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0xFF),
                      static_cast<char>(0x01)};
std::deque<int> TestIndexInput::INTS;
std::deque<int64_t> TestIndexInput::LONGS;
std::deque<char> TestIndexInput::RANDOM_TEST_BYTES;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
// java.io.IOException
void TestIndexInput::beforeClass() 
{
  shared_ptr<Random> random = TestIndexInput::random();
  INTS = std::deque<int>(COUNT);
  LONGS = std::deque<int64_t>(COUNT);
  RANDOM_TEST_BYTES = std::deque<char>(COUNT * (5 + 4 + 9 + 8));
  shared_ptr<ByteArrayDataOutput> *const bdo =
      make_shared<ByteArrayDataOutput>(RANDOM_TEST_BYTES);
  for (int i = 0; i < COUNT; i++) {
    constexpr int i1 = INTS[i] = random->nextInt();
    bdo->writeVInt(i1);
    bdo->writeInt(i1);

    constexpr int64_t l1;
    if (rarely()) {
      // a long with lots of zeroes at the end
      l1 = LONGS[i] = TestUtil::nextLong(random, 0, numeric_limits<int>::max())
                      << 32;
    } else {
      l1 = LONGS[i] =
          TestUtil::nextLong(random, 0, numeric_limits<int64_t>::max());
    }
    bdo->writeVLong(l1);
    bdo->writeLong(l1);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestIndexInput::afterClass()
{
  INTS.clear();
  LONGS.clear();
  RANDOM_TEST_BYTES.clear();
}

void TestIndexInput::checkReads(shared_ptr<DataInput> is,
                                type_info expectedEx) 
{
  TestUtil::assertEquals(128, is->readVInt());
  TestUtil::assertEquals(16383, is->readVInt());
  TestUtil::assertEquals(16384, is->readVInt());
  TestUtil::assertEquals(16385, is->readVInt());
  TestUtil::assertEquals(numeric_limits<int>::max(), is->readVInt());
  TestUtil::assertEquals(-1, is->readVInt());
  TestUtil::assertEquals(static_cast<int64_t>(numeric_limits<int>::max()),
                         is->readVLong());
  TestUtil::assertEquals(numeric_limits<int64_t>::max(), is->readVLong());
  TestUtil::assertEquals(L"Lucene", is->readString());

  TestUtil::assertEquals(L"\u00BF", is->readString());
  TestUtil::assertEquals(L"Lu\u00BFce\u00BFne", is->readString());

  TestUtil::assertEquals(L"\u2620", is->readString());
  TestUtil::assertEquals(L"Lu\u2620ce\u2620ne", is->readString());

  TestUtil::assertEquals(L"\uD834\uDD1E", is->readString());
  TestUtil::assertEquals(L"\uD834\uDD1E\uD834\uDD60", is->readString());
  TestUtil::assertEquals(L"Lu\uD834\uDD1Ece\uD834\uDD60ne", is->readString());

  TestUtil::assertEquals(L"\u0000", is->readString());
  TestUtil::assertEquals(L"Lu\u0000ce\u0000ne", is->readString());

  runtime_error expected = expectThrows(expectedEx, [&]() { is->readVInt(); });
  assertTrue(expected.what()->startsWith(L"Invalid vInt"));
  TestUtil::assertEquals(1, is->readVInt()); // guard value

  expected = expectThrows(expectedEx, [&]() { is->readVLong(); });
  assertTrue(expected.what()->startsWith(L"Invalid vLong"));
  TestUtil::assertEquals(1LL, is->readVLong()); // guard value
}

void TestIndexInput::checkRandomReads(shared_ptr<DataInput> is) throw(
    IOException)
{
  for (int i = 0; i < COUNT; i++) {
    TestUtil::assertEquals(INTS[i], is->readVInt());
    TestUtil::assertEquals(INTS[i], is->readInt());
    TestUtil::assertEquals(LONGS[i], is->readVLong());
    TestUtil::assertEquals(LONGS[i], is->readLong());
  }
}

void TestIndexInput::testRawIndexInputRead() 
{
  for (int i = 0; i < 10; i++) {
    shared_ptr<Random> random = TestIndexInput::random();
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<IndexOutput> os =
        dir->createOutput(L"foo", newIOContext(random));
    os->writeBytes(READ_TEST_BYTES, READ_TEST_BYTES.size());
    delete os;
    shared_ptr<IndexInput> is = dir->openInput(L"foo", newIOContext(random));
    checkReads(is, IOException::typeid);
    delete is;

    os = dir->createOutput(L"bar", newIOContext(random));
    os->writeBytes(RANDOM_TEST_BYTES, RANDOM_TEST_BYTES.size());
    delete os;
    is = dir->openInput(L"bar", newIOContext(random));
    checkRandomReads(is);
    delete is;
    delete dir;
  }
}

void TestIndexInput::testByteArrayDataInput() 
{
  shared_ptr<ByteArrayDataInput> is =
      make_shared<ByteArrayDataInput>(READ_TEST_BYTES);
  checkReads(is, runtime_error::typeid);
  is = make_shared<ByteArrayDataInput>(RANDOM_TEST_BYTES);
  checkRandomReads(is);
}
} // namespace org::apache::lucene::index