using namespace std;

#include "TestBytesStore.h"

namespace org::apache::lucene::util::fst
{
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBytesStore::testRandom() 
{

  constexpr int iters = atLeast(10);
  constexpr int maxBytes = TEST_NIGHTLY ? 200000 : 20000;
  for (int iter = 0; iter < iters; iter++) {
    constexpr int numBytes = TestUtil::nextInt(random(), 1, maxBytes);
    const std::deque<char> expected = std::deque<char>(numBytes);
    constexpr int blockBits = TestUtil::nextInt(random(), 8, 15);
    shared_ptr<BytesStore> *const bytes = make_shared<BytesStore>(blockBits);
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << L" numBytes=" << numBytes
            << L" blockBits=" << blockBits << endl;
    }

    int pos = 0;
    while (pos < numBytes) {
      int op = random()->nextInt(8);
      if (VERBOSE) {
        wcout << L"  cycle pos=" << pos << endl;
      }
      switch (op) {

      case 0: {
        // write random byte
        char b = static_cast<char>(random()->nextInt(256));
        if (VERBOSE) {
          wcout << L"    writeByte b=" << b << endl;
        }

        expected[pos++] = b;
        bytes->writeByte(b);
      } break;

      case 1: {
        // write random byte[]
        int len = random()->nextInt(min(numBytes - pos, 100));
        std::deque<char> temp(len);
        random()->nextBytes(temp);
        if (VERBOSE) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"    writeBytes len=" << len << L" bytes="
                << Arrays->toString(temp) << endl;
        }
        System::arraycopy(temp, 0, expected, pos, temp.size());
        bytes->writeBytes(temp, 0, temp.size());
        pos += len;
      } break;

      case 2: {
        // write int @ absolute pos
        if (pos > 4) {
          int x = random()->nextInt();
          int randomPos = random()->nextInt(pos - 4);
          if (VERBOSE) {
            wcout << L"    abs writeInt pos=" << randomPos << L" x=" << x
                  << endl;
          }
          bytes->writeInt(randomPos, x);
          expected[randomPos++] = static_cast<char>(x >> 24);
          expected[randomPos++] = static_cast<char>(x >> 16);
          expected[randomPos++] = static_cast<char>(x >> 8);
          expected[randomPos++] = static_cast<char>(x);
        }
      } break;

      case 3: {
        // reverse bytes
        if (pos > 1) {
          int len = TestUtil::nextInt(random(), 2, min(100, pos));
          int start;
          if (len == pos) {
            start = 0;
          } else {
            start = random()->nextInt(pos - len);
          }
          int end = start + len - 1;
          if (VERBOSE) {
            wcout << L"    reverse start=" << start << L" end=" << end
                  << L" len=" << len << L" pos=" << pos << endl;
          }
          bytes->reverse(start, end);

          while (start <= end) {
            char b = expected[end];
            expected[end] = expected[start];
            expected[start] = b;
            start++;
            end--;
          }
        }
      } break;

      case 4: {
        // abs write random byte[]
        if (pos > 2) {
          int randomPos = random()->nextInt(pos - 1);
          int len =
              TestUtil::nextInt(random(), 1, min(pos - randomPos - 1, 100));
          std::deque<char> temp(len);
          random()->nextBytes(temp);
          if (VERBOSE) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            wcout << L"    abs writeBytes pos=" << randomPos << L" len=" << len
                  << L" bytes=" << Arrays->toString(temp) << endl;
          }
          System::arraycopy(temp, 0, expected, randomPos, temp.size());
          bytes->writeBytes(randomPos, temp, 0, temp.size());
        }
      } break;

      case 5: {
        // copyBytes
        if (pos > 1) {
          int src = random()->nextInt(pos - 1);
          int dest = TestUtil::nextInt(random(), src + 1, pos - 1);
          int len = TestUtil::nextInt(random(), 1, min(300, pos - dest));
          if (VERBOSE) {
            wcout << L"    copyBytes src=" << src << L" dest=" << dest
                  << L" len=" << len << endl;
          }
          System::arraycopy(expected, src, expected, dest, len);
          bytes->copyBytes(src, dest, len);
        }
      } break;

      case 6: {
        // skip
        int len = random()->nextInt(min(100, numBytes - pos));

        if (VERBOSE) {
          wcout << L"    skip len=" << len << endl;
        }

        pos += len;
        bytes->skipBytes(len);

        // NOTE: must fill in zeros in case truncate was
        // used, else we get false fails:
        if (len > 0) {
          std::deque<char> zeros(len);
          bytes->writeBytes(pos - len, zeros, 0, len);
        }
      } break;

      case 7: {
        // absWriteByte
        if (pos > 0) {
          int dest = random()->nextInt(pos);
          char b = static_cast<char>(random()->nextInt(256));
          expected[dest] = b;
          bytes->writeByte(dest, b);
        }
        break;
      }
      }

      TestUtil::assertEquals(pos, bytes->getPosition());

      if (pos > 0 && random()->nextInt(50) == 17) {
        // truncate
        int len = TestUtil::nextInt(random(), 1, min(pos, 100));
        bytes->truncate(pos - len);
        pos -= len;
        Arrays::fill(expected, pos, pos + len, static_cast<char>(0));
        if (VERBOSE) {
          wcout << L"    truncate len=" << len << L" newPos=" << pos << endl;
        }
      }

      if ((pos > 0 && random()->nextInt(200) == 17)) {
        verify(bytes, expected, pos);
      }
    }

    shared_ptr<BytesStore> bytesToVerify;

    if (random()->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"TEST: save/load final bytes" << endl;
      }
      shared_ptr<Directory> dir = newDirectory();
      shared_ptr<IndexOutput> out =
          dir->createOutput(L"bytes", IOContext::DEFAULT);
      bytes->writeTo(out);
      delete out;
      shared_ptr<IndexInput> in_ = dir->openInput(L"bytes", IOContext::DEFAULT);
      bytesToVerify = make_shared<BytesStore>(
          in_, numBytes,
          TestUtil::nextInt(random(), 256, numeric_limits<int>::max()));
      delete in_;
      delete dir;
    } else {
      bytesToVerify = bytes;
    }

    verify(bytesToVerify, expected, numBytes);
  }
}

void TestBytesStore::verify(shared_ptr<BytesStore> bytes,
                            std::deque<char> &expected,
                            int totalLength) 
{
  TestUtil::assertEquals(totalLength, bytes->getPosition());
  if (totalLength == 0) {
    return;
  }
  if (VERBOSE) {
    wcout << L"  verify..." << endl;
  }

  // First verify whole thing in one blast:
  std::deque<char> actual(totalLength);
  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"    bulk: reversed" << endl;
    }
    // reversed
    shared_ptr<FST::BytesReader> r = bytes->getReverseReader();
    assertTrue(r->reversed());
    r->setPosition(totalLength - 1);
    r->readBytes(actual, 0, actual.size());
    int start = 0;
    int end = totalLength - 1;
    while (start < end) {
      char b = actual[start];
      actual[start] = actual[end];
      actual[end] = b;
      start++;
      end--;
    }
  } else {
    // forward
    if (VERBOSE) {
      wcout << L"    bulk: forward" << endl;
    }
    shared_ptr<FST::BytesReader> r = bytes->getForwardReader();
    assertFalse(r->reversed());
    r->readBytes(actual, 0, actual.size());
  }

  for (int i = 0; i < totalLength; i++) {
    assertEquals(L"byte @ index=" + to_wstring(i), expected[i], actual[i]);
  }

  shared_ptr<FST::BytesReader> r;

  // Then verify ops:
  bool reversed = random()->nextBoolean();
  if (reversed) {
    if (VERBOSE) {
      wcout << L"    ops: reversed" << endl;
    }
    r = bytes->getReverseReader();
  } else {
    if (VERBOSE) {
      wcout << L"    ops: forward" << endl;
    }
    r = bytes->getForwardReader();
  }

  if (totalLength > 1) {
    int numOps = TestUtil::nextInt(random(), 100, 200);
    for (int op = 0; op < numOps; op++) {

      int numBytes = random()->nextInt(min(1000, totalLength - 1));
      int pos;
      if (reversed) {
        pos = TestUtil::nextInt(random(), numBytes, totalLength - 1);
      } else {
        pos = random()->nextInt(totalLength - numBytes);
      }
      if (VERBOSE) {
        wcout << L"    op iter=" << op << L" reversed=" << reversed
              << L" numBytes=" << numBytes << L" pos=" << pos << endl;
      }
      std::deque<char> temp(numBytes);
      r->setPosition(pos);
      TestUtil::assertEquals(pos, r->getPosition());
      r->readBytes(temp, 0, temp.size());
      for (int i = 0; i < numBytes; i++) {
        char expectedByte;
        if (reversed) {
          expectedByte = expected[pos - i];
        } else {
          expectedByte = expected[pos + i];
        }
        assertEquals(L"byte @ index=" + to_wstring(i), expectedByte, temp[i]);
      }

      int left;
      int expectedPos;

      if (reversed) {
        expectedPos = pos - numBytes;
        left = static_cast<int>(r->getPosition());
      } else {
        expectedPos = pos + numBytes;
        left = static_cast<int>(totalLength - r->getPosition());
      }
      TestUtil::assertEquals(expectedPos, r->getPosition());

      if (left > 4) {
        int skipBytes = random()->nextInt(left - 4);

        int expectedInt = 0;
        if (reversed) {
          expectedPos -= skipBytes;
          expectedInt |= (expected[expectedPos--] & 0xFF) << 24;
          expectedInt |= (expected[expectedPos--] & 0xFF) << 16;
          expectedInt |= (expected[expectedPos--] & 0xFF) << 8;
          expectedInt |= (expected[expectedPos--] & 0xFF);
        } else {
          expectedPos += skipBytes;
          expectedInt |= (expected[expectedPos++] & 0xFF) << 24;
          expectedInt |= (expected[expectedPos++] & 0xFF) << 16;
          expectedInt |= (expected[expectedPos++] & 0xFF) << 8;
          expectedInt |= (expected[expectedPos++] & 0xFF);
        }

        if (VERBOSE) {
          wcout << L"    skip numBytes=" << skipBytes << endl;
          wcout << L"    readInt" << endl;
        }

        r->skipBytes(skipBytes);
        TestUtil::assertEquals(expectedInt, r->readInt());
      }
    }
  }
}
} // namespace org::apache::lucene::util::fst