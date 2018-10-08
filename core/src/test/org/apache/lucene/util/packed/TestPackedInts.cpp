using namespace std;

#include "TestPackedInts.h"

namespace org::apache::lucene::util::packed
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LongValues = org::apache::lucene::util::LongValues;
using LongsRef = org::apache::lucene::util::LongsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;
using Reader = org::apache::lucene::util::packed::PackedInts::Reader;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using org::junit::Ignore;

void TestPackedInts::testByteCount()
{
  constexpr int iters = atLeast(3);
  for (int i = 0; i < iters; ++i) {
    constexpr int valueCount = RandomNumbers::randomIntBetween(
        random(), 1, numeric_limits<int>::max());
    for (PackedInts::Format format : PackedInts::Format::values()) {
      for (int bpv = 1; bpv <= 64; ++bpv) {
        constexpr int64_t byteCount =
            format.byteCount(PackedInts::VERSION_CURRENT, valueCount, bpv);
        wstring msg = L"format=" + format + L", byteCount=" +
                      to_wstring(byteCount) + L", valueCount=" +
                      to_wstring(valueCount) + L", bpv=" + to_wstring(bpv);
        assertTrue(msg,
                   byteCount * 8 >= static_cast<int64_t>(valueCount) * bpv);
        if (format == PackedInts::Format::PACKED) {
          assertTrue(msg, (byteCount - 1) * 8 <
                              static_cast<int64_t>(valueCount) * bpv);
        }
      }
    }
  }
}

void TestPackedInts::testBitsRequired()
{
  TestUtil::assertEquals(
      61, PackedInts::bitsRequired(static_cast<int64_t>(pow(2, 61)) - 1));
  TestUtil::assertEquals(61, PackedInts::bitsRequired(0x1FFFFFFFFFFFFFFFLL));
  TestUtil::assertEquals(62, PackedInts::bitsRequired(0x3FFFFFFFFFFFFFFFLL));
  TestUtil::assertEquals(63, PackedInts::bitsRequired(0x7FFFFFFFFFFFFFFFLL));
  TestUtil::assertEquals(64, PackedInts::unsignedBitsRequired(-1));
  TestUtil::assertEquals(
      64, PackedInts::unsignedBitsRequired(numeric_limits<int64_t>::min()));
  TestUtil::assertEquals(1, PackedInts::bitsRequired(0));
}

void TestPackedInts::testMaxValues()
{
  assertEquals(L"1 bit -> max == 1", 1, PackedInts::maxValue(1));
  assertEquals(L"2 bit -> max == 3", 3, PackedInts::maxValue(2));
  assertEquals(L"8 bit -> max == 255", 255, PackedInts::maxValue(8));
  assertEquals(L"63 bit -> max == Long.MAX_VALUE",
               numeric_limits<int64_t>::max(), PackedInts::maxValue(63));
  assertEquals(L"64 bit -> max == Long.MAX_VALUE (same as for 63 bit)",
               numeric_limits<int64_t>::max(), PackedInts::maxValue(64));
}

void TestPackedInts::testPackedInts() 
{
  int num = atLeast(3);
  for (int iter = 0; iter < num; iter++) {
    for (int nbits = 1; nbits <= 64; nbits++) {
      constexpr int64_t maxValue = PackedInts::maxValue(nbits);
      constexpr int valueCount = TestUtil::nextInt(random(), 1, 600);
      constexpr int bufferSize = random()->nextBoolean()
                                     ? TestUtil::nextInt(random(), 0, 48)
                                     : TestUtil::nextInt(random(), 0, 4096);
      shared_ptr<Directory> *const d = newDirectory();

      shared_ptr<IndexOutput> out =
          d->createOutput(L"out.bin", newIOContext(random()));
      constexpr float acceptableOverhead;
      if (iter == 0) {
        // have the first iteration go through exact nbits
        acceptableOverhead = 0.0f;
      } else {
        acceptableOverhead = random()->nextFloat();
      }
      shared_ptr<PackedInts::Writer> w =
          PackedInts::getWriter(out, valueCount, nbits, acceptableOverhead);
      constexpr int64_t startFp = out->getFilePointer();

      constexpr int actualValueCount =
          random()->nextBoolean() ? valueCount
                                  : TestUtil::nextInt(random(), 0, valueCount);
      const std::deque<int64_t> values = std::deque<int64_t>(valueCount);
      for (int i = 0; i < actualValueCount; i++) {
        if (nbits == 64) {
          values[i] = random()->nextLong();
        } else {
          values[i] = TestUtil::nextLong(random(), 0, maxValue);
        }
        w->add(values[i]);
      }
      w->finish();
      constexpr int64_t fp = out->getFilePointer();
      delete out;

      // ensure that finish() added the (valueCount-actualValueCount) missing
      // values
      constexpr int64_t bytes = w->getFormat().byteCount(
          PackedInts::VERSION_CURRENT, valueCount, w->bitsPerValue_);
      TestUtil::assertEquals(bytes, fp - startFp);

      { // test header
        shared_ptr<IndexInput> in_ =
            d->openInput(L"out.bin", newIOContext(random()));
        // header = codec header | bitsPerValue | valueCount | format
        CodecUtil::checkHeader(in_, PackedInts::CODEC_NAME,
                               PackedInts::VERSION_START,
                               PackedInts::VERSION_CURRENT); // codec header
        TestUtil::assertEquals(w->bitsPerValue_, in_->readVInt());
        TestUtil::assertEquals(valueCount, in_->readVInt());
        TestUtil::assertEquals(w->getFormat().getId(), in_->readVInt());
        TestUtil::assertEquals(startFp, in_->getFilePointer());
        delete in_;
      }

      { // test reader
        shared_ptr<IndexInput> in_ =
            d->openInput(L"out.bin", newIOContext(random()));
        shared_ptr<PackedInts::Reader> r = PackedInts::getReader(in_);
        TestUtil::assertEquals(fp, in_->getFilePointer());
        for (int i = 0; i < valueCount; i++) {
          assertEquals(L"index=" + to_wstring(i) + L" valueCount=" +
                           to_wstring(valueCount) + L" nbits=" +
                           to_wstring(nbits) + L" for " +
                           r->getClass().getSimpleName(),
                       values[i], r->get(i));
        }
        delete in_;

        constexpr int64_t expectedBytesUsed = RamUsageTester::sizeOf(r);
        constexpr int64_t computedBytesUsed = r->ramBytesUsed();
        assertEquals(r->getClass() + L"expected " +
                         to_wstring(expectedBytesUsed) + L", got: " +
                         to_wstring(computedBytesUsed),
                     expectedBytesUsed, computedBytesUsed);
      }

      { // test reader iterator next
        shared_ptr<IndexInput> in_ =
            d->openInput(L"out.bin", newIOContext(random()));
        shared_ptr<PackedInts::ReaderIterator> r =
            PackedInts::getReaderIterator(in_, bufferSize);
        for (int i = 0; i < valueCount; i++) {
          assertEquals(L"index=" + to_wstring(i) + L" valueCount=" +
                           to_wstring(valueCount) + L" nbits=" +
                           to_wstring(nbits) + L" for " +
                           r->getClass().getSimpleName(),
                       values[i], r->next());
          TestUtil::assertEquals(i, r->ord());
        }
        TestUtil::assertEquals(fp, in_->getFilePointer());
        delete in_;
      }

      { // test reader iterator bulk next
        shared_ptr<IndexInput> in_ =
            d->openInput(L"out.bin", newIOContext(random()));
        shared_ptr<PackedInts::ReaderIterator> r =
            PackedInts::getReaderIterator(in_, bufferSize);
        int i = 0;
        while (i < valueCount) {
          constexpr int count = TestUtil::nextInt(random(), 1, 95);
          shared_ptr<LongsRef> *const next = r->next(count);
          for (int k = 0; k < next->length; ++k) {
            assertEquals(L"index=" + to_wstring(i) + L" valueCount=" +
                             to_wstring(valueCount) + L" nbits=" +
                             to_wstring(nbits) + L" for " +
                             r->getClass().getSimpleName(),
                         values[i + k], next->longs[next->offset + k]);
          }
          i += next->length;
        }
        TestUtil::assertEquals(fp, in_->getFilePointer());
        delete in_;
      }

      { // test direct reader get
        shared_ptr<IndexInput> in_ =
            d->openInput(L"out.bin", newIOContext(random()));
        shared_ptr<PackedInts::Reader> intsEnum =
            PackedInts::getDirectReader(in_);
        for (int i = 0; i < valueCount; i++) {
          const wstring msg = L"index=" + to_wstring(i) + L" valueCount=" +
                              to_wstring(valueCount) + L" nbits=" +
                              to_wstring(nbits) + L" for " +
                              intsEnum->getClass().getSimpleName();
          constexpr int index = random()->nextInt(valueCount);
          assertEquals(msg, values[index], intsEnum->get(index));
        }
        intsEnum->get(intsEnum->size() - 1);
        TestUtil::assertEquals(fp, in_->getFilePointer());
        delete in_;
      }
      delete d;
    }
  }
}

void TestPackedInts::testEndPointer() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  constexpr int valueCount = RandomNumbers::randomIntBetween(random(), 1, 1000);
  shared_ptr<IndexOutput> *const out =
      dir->createOutput(L"tests.bin", newIOContext(random()));
  for (int i = 0; i < valueCount; ++i) {
    out->writeLong(0);
  }
  delete out;
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"tests.bin", newIOContext(random()));
  for (int version = PackedInts::VERSION_START;
       version <= PackedInts::VERSION_CURRENT; ++version) {
    for (int bpv = 1; bpv <= 64; ++bpv) {
      for (PackedInts::Format format : PackedInts::Format::values()) {
        if (!format.isSupported(bpv)) {
          continue;
        }
        constexpr int64_t byteCount =
            format.byteCount(version, valueCount, bpv);
        wstring msg = L"format=" + format + L",version=" + to_wstring(version) +
                      L",valueCount=" + to_wstring(valueCount) + L",bpv=" +
                      to_wstring(bpv);

        // test iterator
        in_->seek(0LL);
        shared_ptr<PackedInts::ReaderIterator> *const it =
            PackedInts::getReaderIteratorNoHeader(
                in_, format, version, valueCount, bpv,
                RandomNumbers::randomIntBetween(random(), 1, 1 << 16));
        for (int i = 0; i < valueCount; ++i) {
          it->next();
        }
        assertEquals(msg, byteCount, in_->getFilePointer());

        // test direct reader
        in_->seek(0LL);
        shared_ptr<PackedInts::Reader> *const directReader =
            PackedInts::getDirectReaderNoHeader(in_, format, version,
                                                valueCount, bpv);
        directReader->get(valueCount - 1);
        assertEquals(msg, byteCount, in_->getFilePointer());

        // test reader
        in_->seek(0LL);
        PackedInts::getReaderNoHeader(in_, format, version, valueCount, bpv);
        assertEquals(msg, byteCount, in_->getFilePointer());
      }
    }
  }
  delete in_;
  delete dir;
}

void TestPackedInts::testControlledEquality()
{
  constexpr int VALUE_COUNT = 255;
  constexpr int BITS_PER_VALUE = 8;

  deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
      createPackedInts(VALUE_COUNT, BITS_PER_VALUE);
  for (auto packedInt : packedInts) {
    for (int i = 0; i < packedInt->size(); i++) {
      packedInt->set(i, i + 1);
    }
  }
  assertListEquality(packedInts);
}

void TestPackedInts::testRandomBulkCopy()
{
  constexpr int numIters = atLeast(3);
  for (int iter = 0; iter < numIters; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    constexpr int valueCount = atLeast(100000);
    int bits1 = TestUtil::nextInt(random(), 1, 64);
    int bits2 = TestUtil::nextInt(random(), 1, 64);
    if (bits1 > bits2) {
      int tmp = bits1;
      bits1 = bits2;
      bits2 = tmp;
    }
    if (VERBOSE) {
      wcout << L"  valueCount=" << valueCount << L" bits1=" << bits1
            << L" bits2=" << bits2 << endl;
    }

    shared_ptr<PackedInts::Mutable> *const packed1 =
        PackedInts::getMutable(valueCount, bits1, PackedInts::COMPACT);
    shared_ptr<PackedInts::Mutable> *const packed2 =
        PackedInts::getMutable(valueCount, bits2, PackedInts::COMPACT);

    constexpr int64_t maxValue = PackedInts::maxValue(bits1);
    for (int i = 0; i < valueCount; i++) {
      constexpr int64_t val = TestUtil::nextLong(random(), 0, maxValue);
      packed1->set(i, val);
      packed2->set(i, val);
    }

    const std::deque<int64_t> buffer = std::deque<int64_t>(valueCount);

    // Copy random slice over, 20 times:
    for (int iter2 = 0; iter2 < 20; iter2++) {
      int start = random()->nextInt(valueCount - 1);
      int len = TestUtil::nextInt(random(), 1, valueCount - start);
      int offset;
      if (VERBOSE) {
        wcout << L"  copy " << len << L" values @ " << start << endl;
      }
      if (len == valueCount) {
        offset = 0;
      } else {
        offset = random()->nextInt(valueCount - len);
      }
      if (random()->nextBoolean()) {
        int got = packed1->get(start, buffer, offset, len);
        assertTrue(got <= len);
        int sot = packed2->set(start, buffer, offset, got);
        assertTrue(sot <= got);
      } else {
        PackedInts::copy(packed1, offset, packed2, offset, len,
                         random()->nextInt(10 * len));
      }

      /*
      for(int i=0;i<valueCount;i++) {
        assertEquals("value " + i, packed1.get(i), packed2.get(i));
      }
      */
    }

    for (int i = 0; i < valueCount; i++) {
      assertEquals(L"value " + to_wstring(i), packed1->get(i), packed2->get(i));
    }
  }
}

void TestPackedInts::testRandomEquality()
{
  constexpr int numIters = TEST_NIGHTLY ? atLeast(2) : 1;
  for (int i = 0; i < numIters; ++i) {
    constexpr int valueCount = TestUtil::nextInt(random(), 1, 300);

    for (int bitsPerValue = 1; bitsPerValue <= 64; bitsPerValue++) {
      assertRandomEquality(valueCount, bitsPerValue, random()->nextLong());
    }
  }
}

void TestPackedInts::assertRandomEquality(int valueCount, int bitsPerValue,
                                          int64_t randomSeed)
{
  deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
      createPackedInts(valueCount, bitsPerValue);
  for (auto packedInt : packedInts) {
    try {
      fill(packedInt, PackedInts::maxValue(bitsPerValue), randomSeed);
    } catch (const runtime_error &e) {
      e.printStackTrace(System::err);
      fail(wstring::format(
          Locale::ROOT,
          L"Exception while filling %s: valueCount=%d, bitsPerValue=%s",
          packedInt->getClass().getSimpleName(), valueCount, bitsPerValue));
    }
  }
  assertListEquality(packedInts);
}

deque<std::shared_ptr<PackedInts::Mutable>>
TestPackedInts::createPackedInts(int valueCount, int bitsPerValue)
{
  deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
      deque<std::shared_ptr<PackedInts::Mutable>>();
  if (bitsPerValue <= 8) {
    packedInts.push_back(make_shared<Direct8>(valueCount));
  }
  if (bitsPerValue <= 16) {
    packedInts.push_back(make_shared<Direct16>(valueCount));
  }
  if (bitsPerValue <= 24 && valueCount <= Packed8ThreeBlocks::MAX_SIZE) {
    packedInts.push_back(make_shared<Packed8ThreeBlocks>(valueCount));
  }
  if (bitsPerValue <= 32) {
    packedInts.push_back(make_shared<Direct32>(valueCount));
  }
  if (bitsPerValue <= 48 && valueCount <= Packed16ThreeBlocks::MAX_SIZE) {
    packedInts.push_back(make_shared<Packed16ThreeBlocks>(valueCount));
  }
  if (bitsPerValue <= 63) {
    packedInts.push_back(make_shared<Packed64>(valueCount, bitsPerValue));
  }
  packedInts.push_back(make_shared<Direct64>(valueCount));
  for (int bpv = bitsPerValue;
       bpv <= Packed64SingleBlock::MAX_SUPPORTED_BITS_PER_VALUE; ++bpv) {
    if (Packed64SingleBlock::isSupported(bpv)) {
      packedInts.push_back(Packed64SingleBlock::create(valueCount, bpv));
    }
  }
  return packedInts;
}

void TestPackedInts::fill(shared_ptr<PackedInts::Mutable> packedInt,
                          int64_t maxValue, int64_t randomSeed)
{
  shared_ptr<Random> rnd2 = make_shared<Random>(randomSeed);
  for (int i = 0; i < packedInt->size(); i++) {
    int64_t value = TestUtil::nextLong(rnd2, 0, maxValue);
    packedInt->set(i, value);
    assertEquals(
        wstring::format(
            Locale::ROOT,
            L"The set/get of the value at index %d should match for %s", i,
            packedInt->getClass().getSimpleName()),
        value, packedInt->get(i));
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private static void
// assertListEquality(java.util.List<? extends PackedInts.Reader> packedInts)
void TestPackedInts::assertListEquality(deque<T1> packedInts)
{
  assertListEquality(L"", packedInts);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private static void assertListEquality(std::wstring
// message, java.util.List<? extends PackedInts.Reader> packedInts)
void TestPackedInts::assertListEquality(const wstring &message,
                                        deque<T1> packedInts)
{
  if (packedInts.empty()) {
    return;
  }
  shared_ptr<PackedInts::Reader> base = packedInts[0];
  int valueCount = base->size();
  for (auto packedInt : packedInts) {
    assertEquals(message + L". The number of values should be the same ",
                 valueCount, packedInt->size());
  }
  for (int i = 0; i < valueCount; i++) {
    for (int j = 1; j < packedInts.size(); j++) {
      assertEquals(
          wstring::format(
              Locale::ROOT,
              L"%s. The value at index %d should be the same for %s and %s",
              message, i, base->getClass().getSimpleName(),
              packedInts[j]->getClass().getSimpleName()),
          base->get(i), packedInts[j]->get(i));
    }
  }
}

void TestPackedInts::testSingleValue() 
{
  for (int bitsPerValue = 1; bitsPerValue <= 64; ++bitsPerValue) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexOutput> out =
        dir->createOutput(L"out", newIOContext(random()));
    shared_ptr<PackedInts::Writer> w =
        PackedInts::getWriter(out, 1, bitsPerValue, PackedInts::DEFAULT);
    int64_t value = 17LL & PackedInts::maxValue(bitsPerValue);
    w->add(value);
    w->finish();
    constexpr int64_t end = out->getFilePointer();
    delete out;

    shared_ptr<IndexInput> in_ = dir->openInput(L"out", newIOContext(random()));
    shared_ptr<Reader> reader = PackedInts::getReader(in_);
    wstring msg = L"Impl=" + w->getClass().getSimpleName() +
                  L", bitsPerValue=" + to_wstring(bitsPerValue);
    assertEquals(msg, 1, reader->size());
    assertEquals(msg, value, reader->get(0));
    assertEquals(msg, end, in_->getFilePointer());
    delete in_;

    delete dir;
  }
}

void TestPackedInts::testSecondaryBlockChange()
{
  shared_ptr<PackedInts::Mutable> mutable_ = make_shared<Packed64>(26, 5);
  mutable_->set(24, 31);
  assertEquals(L"The value #24 should be correct", 31, mutable_->get(24));
  mutable_->set(4, 16);
  assertEquals(L"The value #24 should remain unchanged", 31, mutable_->get(24));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("See LUCENE-4488") public void testIntOverflow()
void TestPackedInts::testIntOverflow()
{
  int INDEX = static_cast<int>(pow(2, 30)) + 1;
  int BITS = 2;

  shared_ptr<Packed64> p64 = nullptr;
  try {
    p64 = make_shared<Packed64>(INDEX, BITS);
  } catch (const OutOfMemoryError &oome) {
    // This can easily happen: we're allocating a
    // long[] that needs 256-273 MB.  Heap is 512 MB,
    // but not all of that is available for large
    // objects ... empirical testing shows we only
    // have ~ 67 MB free.
  }
  if (p64 != nullptr) {
    p64->set(INDEX - 1, 1);
    assertEquals(L"The value at position " + to_wstring(INDEX - 1) +
                     L" should be correct for Packed64",
                 1, p64->get(INDEX - 1));
    p64.reset();
  }

  shared_ptr<Packed64SingleBlock> p64sb = nullptr;
  try {
    p64sb = Packed64SingleBlock::create(INDEX, BITS);
  } catch (const OutOfMemoryError &oome) {
    // Ignore: see comment above
  }
  if (p64sb != nullptr) {
    p64sb->set(INDEX - 1, 1);
    assertEquals(L"The value at position " + to_wstring(INDEX - 1) +
                     L" should be correct for " +
                     p64sb->getClass().getSimpleName(),
                 1, p64sb->get(INDEX - 1));
  }

  int index = numeric_limits<int>::max() / 24 + 1;
  shared_ptr<Packed8ThreeBlocks> p8 = nullptr;
  try {
    p8 = make_shared<Packed8ThreeBlocks>(index);
  } catch (const OutOfMemoryError &oome) {
    // Ignore: see comment above
  }
  if (p8 != nullptr) {
    p8->set(index - 1, 1);
    assertEquals(L"The value at position " + to_wstring(index - 1) +
                     L" should be correct for Packed8ThreeBlocks",
                 1, p8->get(index - 1));
    p8.reset();
  }

  index = numeric_limits<int>::max() / 48 + 1;
  shared_ptr<Packed16ThreeBlocks> p16 = nullptr;
  try {
    p16 = make_shared<Packed16ThreeBlocks>(index);
  } catch (const OutOfMemoryError &oome) {
    // Ignore: see comment above
  }
  if (p16 != nullptr) {
    p16->set(index - 1, 1);
    assertEquals(L"The value at position " + to_wstring(index - 1) +
                     L" should be correct for Packed16ThreeBlocks",
                 1, p16->get(index - 1));
    p16.reset();
  }
}

void TestPackedInts::testFill()
{
  constexpr int valueCount = 1111;
  constexpr int from = random()->nextInt(valueCount + 1);
  constexpr int to = from + random()->nextInt(valueCount + 1 - from);
  for (int bpv = 1; bpv <= 64; ++bpv) {
    constexpr int64_t val =
        TestUtil::nextLong(random(), 0, PackedInts::maxValue(bpv));
    deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
        createPackedInts(valueCount, bpv);
    for (auto ints : packedInts) {
      wstring msg = ints->getClass().getSimpleName() + L" bpv=" +
                    to_wstring(bpv) + L", from=" + to_wstring(from) + L", to=" +
                    to_wstring(to) + L", val=" + to_wstring(val);
      ints->fill(0, ints->size(), 1);
      ints->fill(from, to, val);
      for (int i = 0; i < ints->size(); ++i) {
        if (i >= from && i < to) {
          assertEquals(msg + L", i=" + to_wstring(i), val, ints->get(i));
        } else {
          assertEquals(msg + L", i=" + to_wstring(i), 1, ints->get(i));
        }
      }
    }
  }
}

void TestPackedInts::testPackedIntsNull()
{
  // must be > 10 for the bulk reads below
  int size = TestUtil::nextInt(random(), 11, 256);
  shared_ptr<Reader> packedInts = make_shared<PackedInts::NullReader>(size);
  TestUtil::assertEquals(
      0, packedInts->get(TestUtil::nextInt(random(), 0, size - 1)));
  std::deque<int64_t> arr(size + 10);
  int r;
  Arrays::fill(arr, 1);
  r = packedInts->get(0, arr, 0, size - 1);
  TestUtil::assertEquals(size - 1, r);
  for (r--; r >= 0; r--) {
    TestUtil::assertEquals(0, arr[r]);
  }
  Arrays::fill(arr, 1);
  r = packedInts->get(10, arr, 0, size + 10);
  TestUtil::assertEquals(size - 10, r);
  for (int i = 0; i < size - 10; i++) {
    TestUtil::assertEquals(0, arr[i]);
  }
}

void TestPackedInts::testBulkGet()
{
  constexpr int valueCount = 1111;
  constexpr int index = random()->nextInt(valueCount);
  constexpr int len = TestUtil::nextInt(random(), 1, valueCount * 2);
  constexpr int off = random()->nextInt(77);

  for (int bpv = 1; bpv <= 64; ++bpv) {
    int64_t mask = PackedInts::maxValue(bpv);
    deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
        createPackedInts(valueCount, bpv);

    for (auto ints : packedInts) {
      for (int i = 0; i < ints->size(); ++i) {
        ints->set(i, (31LL * i - 1099) & mask);
      }
      std::deque<int64_t> arr(off + len);

      wstring msg = ints->getClass().getSimpleName() + L" valueCount=" +
                    to_wstring(valueCount) + L", index=" + to_wstring(index) +
                    L", len=" + to_wstring(len) + L", off=" + to_wstring(off);
      constexpr int gets = ints->get(index, arr, off, len);
      assertTrue(msg, gets > 0);
      assertTrue(msg, gets <= len);
      assertTrue(msg, gets <= ints->size() - index);

      for (int i = 0; i < arr.size(); ++i) {
        wstring m = msg + L", i=" + to_wstring(i);
        if (i >= off && i < off + gets) {
          assertEquals(m, ints->get(i - off + index), arr[i]);
        } else {
          assertEquals(m, 0, arr[i]);
        }
      }
    }
  }
}

void TestPackedInts::testBulkSet()
{
  constexpr int valueCount = 1111;
  constexpr int index = random()->nextInt(valueCount);
  constexpr int len = TestUtil::nextInt(random(), 1, valueCount * 2);
  constexpr int off = random()->nextInt(77);
  std::deque<int64_t> arr(off + len);

  for (int bpv = 1; bpv <= 64; ++bpv) {
    int64_t mask = PackedInts::maxValue(bpv);
    deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
        createPackedInts(valueCount, bpv);
    for (int i = 0; i < arr.size(); ++i) {
      arr[i] = (31LL * i + 19) & mask;
    }

    for (auto ints : packedInts) {
      wstring msg = ints->getClass().getSimpleName() + L" valueCount=" +
                    to_wstring(valueCount) + L", index=" + to_wstring(index) +
                    L", len=" + to_wstring(len) + L", off=" + to_wstring(off);
      constexpr int sets = ints->set(index, arr, off, len);
      assertTrue(msg, sets > 0);
      assertTrue(msg, sets <= len);

      for (int i = 0; i < ints->size(); ++i) {
        wstring m = msg + L", i=" + to_wstring(i);
        if (i >= index && i < index + sets) {
          assertEquals(m, arr[off - index + i], ints->get(i));
        } else {
          assertEquals(m, 0, ints->get(i));
        }
      }
    }
  }
}

void TestPackedInts::testCopy()
{
  constexpr int valueCount = TestUtil::nextInt(random(), 5, 600);
  constexpr int off1 = random()->nextInt(valueCount);
  constexpr int off2 = random()->nextInt(valueCount);
  constexpr int len =
      random()->nextInt(min(valueCount - off1, valueCount - off2));
  constexpr int mem = random()->nextInt(1024);

  for (int bpv = 1; bpv <= 64; ++bpv) {
    int64_t mask = PackedInts::maxValue(bpv);
    for (auto r1 : createPackedInts(valueCount, bpv)) {
      for (int i = 0; i < r1->size(); ++i) {
        r1->set(i, (31LL * i - 1023) & mask);
      }
      for (auto r2 : createPackedInts(valueCount, bpv)) {
        wstring msg = L"src=" + r1 + L", dest=" + r2 + L", srcPos=" +
                      to_wstring(off1) + L", destPos=" + to_wstring(off2) +
                      L", len=" + to_wstring(len) + L", mem=" + to_wstring(mem);
        PackedInts::copy(r1, off1, r2, off2, len, mem);
        for (int i = 0; i < r2->size(); ++i) {
          wstring m = msg + L", i=" + to_wstring(i);
          if (i >= off2 && i < off2 + len) {
            assertEquals(m, r1->get(i - off2 + off1), r2->get(i));
          } else {
            assertEquals(m, 0, r2->get(i));
          }
        }
      }
    }
  }
}

void TestPackedInts::testGrowableWriter()
{
  constexpr int valueCount = 113 + random()->nextInt(1111);
  shared_ptr<GrowableWriter> wrt =
      make_shared<GrowableWriter>(1, valueCount, PackedInts::DEFAULT);
  wrt->set(4, 2);
  wrt->set(7, 10);
  wrt->set(valueCount - 10, 99);
  wrt->set(99, 999);
  wrt->set(valueCount - 1, 1 << 10);
  TestUtil::assertEquals(1 << 10, wrt->get(valueCount - 1));
  wrt->set(99, (1 << 23) - 1);
  TestUtil::assertEquals(1 << 10, wrt->get(valueCount - 1));
  wrt->set(1, numeric_limits<int64_t>::max());
  wrt->set(2, -3);
  TestUtil::assertEquals(64, wrt->getBitsPerValue());
  TestUtil::assertEquals(1 << 10, wrt->get(valueCount - 1));
  TestUtil::assertEquals(numeric_limits<int64_t>::max(), wrt->get(1));
  TestUtil::assertEquals(-3LL, wrt->get(2));
  TestUtil::assertEquals(2, wrt->get(4));
  TestUtil::assertEquals((1 << 23) - 1, wrt->get(99));
  TestUtil::assertEquals(10, wrt->get(7));
  TestUtil::assertEquals(99, wrt->get(valueCount - 10));
  TestUtil::assertEquals(1 << 10, wrt->get(valueCount - 1));
  TestUtil::assertEquals(RamUsageTester::sizeOf(wrt), wrt->ramBytesUsed());
}

void TestPackedInts::testPagedGrowableWriter()
{
  int pageSize = 1 << (TestUtil::nextInt(random(), 6, 30));
  // supports 0 values?
  shared_ptr<PagedGrowableWriter> writer = make_shared<PagedGrowableWriter>(
      0, pageSize, TestUtil::nextInt(random(), 1, 64), random()->nextFloat());
  TestUtil::assertEquals(0, writer->size());

  // compare against AppendingDeltaPackedLongBuffer
  shared_ptr<PackedLongValues::Builder> buf =
      PackedLongValues::deltaPackedBuilder(random()->nextFloat());
  int size = random()->nextInt(1000000);
  int64_t max = 5;
  for (int i = 0; i < size; ++i) {
    buf->add(TestUtil::nextLong(random(), 0, max));
    if (rarely()) {
      max = PackedInts::maxValue(rarely() ? TestUtil::nextInt(random(), 0, 63)
                                          : TestUtil::nextInt(random(), 0, 31));
    }
  }
  writer = make_shared<PagedGrowableWriter>(size, pageSize,
                                            TestUtil::nextInt(random(), 1, 64),
                                            random()->nextFloat());
  TestUtil::assertEquals(size, writer->size());
  shared_ptr<LongValues> *const values = buf->build();
  for (int i = size - 1; i >= 0; --i) {
    writer->set(i, values->get(i));
  }
  for (int i = 0; i < size; ++i) {
    TestUtil::assertEquals(values->get(i), writer->get(i));
  }

  // test ramBytesUsed
  assertEquals(RamUsageTester::sizeOf(writer), writer->ramBytesUsed(), 8);

  // test copy
  shared_ptr<PagedGrowableWriter> copy = writer->resize(
      TestUtil::nextLong(random(), writer->size() / 2, writer->size() * 3 / 2));
  for (int64_t i = 0; i < copy->size(); ++i) {
    if (i < writer->size()) {
      TestUtil::assertEquals(writer->get(i), copy->get(i));
    } else {
      TestUtil::assertEquals(0, copy->get(i));
    }
  }

  // test grow
  shared_ptr<PagedGrowableWriter> grow = writer->grow(
      TestUtil::nextLong(random(), writer->size() / 2, writer->size() * 3 / 2));
  for (int64_t i = 0; i < grow->size(); ++i) {
    if (i < writer->size()) {
      TestUtil::assertEquals(writer->get(i), grow->get(i));
    } else {
      TestUtil::assertEquals(0, grow->get(i));
    }
  }
}

void TestPackedInts::testPagedMutable()
{
  constexpr int bitsPerValue = TestUtil::nextInt(random(), 1, 64);
  constexpr int64_t max = PackedInts::maxValue(bitsPerValue);
  int pageSize = 1 << (TestUtil::nextInt(random(), 6, 30));
  // supports 0 values?
  shared_ptr<PagedMutable> writer = make_shared<PagedMutable>(
      0, pageSize, bitsPerValue, random()->nextFloat() / 2);
  TestUtil::assertEquals(0, writer->size());

  // compare against AppendingDeltaPackedLongBuffer
  shared_ptr<PackedLongValues::Builder> buf =
      PackedLongValues::deltaPackedBuilder(random()->nextFloat());
  int size = random()->nextInt(1000000);

  for (int i = 0; i < size; ++i) {
    buf->add(bitsPerValue == 64 ? random()->nextLong()
                                : TestUtil::nextLong(random(), 0, max));
  }
  writer = make_shared<PagedMutable>(size, pageSize, bitsPerValue,
                                     random()->nextFloat());
  TestUtil::assertEquals(size, writer->size());
  shared_ptr<LongValues> *const values = buf->build();
  for (int i = size - 1; i >= 0; --i) {
    writer->set(i, values->get(i));
  }
  for (int i = 0; i < size; ++i) {
    TestUtil::assertEquals(values->get(i), writer->get(i));
  }

  // test ramBytesUsed
  TestUtil::assertEquals(RamUsageTester::sizeOf(writer) -
                             RamUsageTester::sizeOf(writer->format),
                         writer->ramBytesUsed());

  // test copy
  shared_ptr<PagedMutable> copy = writer->resize(
      TestUtil::nextLong(random(), writer->size() / 2, writer->size() * 3 / 2));
  for (int64_t i = 0; i < copy->size(); ++i) {
    if (i < writer->size()) {
      TestUtil::assertEquals(writer->get(i), copy->get(i));
    } else {
      TestUtil::assertEquals(0, copy->get(i));
    }
  }

  // test grow
  shared_ptr<PagedMutable> grow = writer->grow(
      TestUtil::nextLong(random(), writer->size() / 2, writer->size() * 3 / 2));
  for (int64_t i = 0; i < grow->size(); ++i) {
    if (i < writer->size()) {
      TestUtil::assertEquals(writer->get(i), grow->get(i));
    } else {
      TestUtil::assertEquals(0, grow->get(i));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testPagedGrowableWriterOverflow()
void TestPackedInts::testPagedGrowableWriterOverflow()
{
  constexpr int64_t size = TestUtil::nextLong(
      random(), 2 * static_cast<int64_t>(numeric_limits<int>::max()),
      3 * static_cast<int64_t>(numeric_limits<int>::max()));
  constexpr int pageSize = 1 << (TestUtil::nextInt(random(), 16, 30));
  shared_ptr<PagedGrowableWriter> *const writer =
      make_shared<PagedGrowableWriter>(size, pageSize, 1,
                                       random()->nextFloat());
  constexpr int64_t index = TestUtil::nextLong(
      random(), static_cast<int64_t>(numeric_limits<int>::max()), size - 1);
  writer->set(index, 2);
  TestUtil::assertEquals(2, writer->get(index));
  for (int i = 0; i < 1000000; ++i) {
    constexpr int64_t idx = TestUtil::nextLong(random(), 0, size);
    if (idx == index) {
      TestUtil::assertEquals(2, writer->get(idx));
    } else {
      TestUtil::assertEquals(0, writer->get(idx));
    }
  }
}

void TestPackedInts::testSave() 
{
  constexpr int valueCount = TestUtil::nextInt(random(), 1, 2048);
  for (int bpv = 1; bpv <= 64; ++bpv) {
    constexpr int maxValue = static_cast<int>(
        min(PackedInts::maxValue(31), PackedInts::maxValue(bpv)));
    shared_ptr<RAMDirectory> *const directory = make_shared<RAMDirectory>();
    deque<std::shared_ptr<PackedInts::Mutable>> packedInts =
        createPackedInts(valueCount, bpv);
    for (auto mutable_ : packedInts) {
      for (int i = 0; i < mutable_->size(); ++i) {
        mutable_->set(i, random()->nextInt(maxValue));
      }

      shared_ptr<IndexOutput> out =
          directory->createOutput(L"packed-ints.bin", IOContext::DEFAULT);
      mutable_->save(out);
      delete out;

      shared_ptr<IndexInput> in_ =
          directory->openInput(L"packed-ints.bin", IOContext::DEFAULT);
      shared_ptr<PackedInts::Reader> reader = PackedInts::getReader(in_);
      TestUtil::assertEquals(valueCount, reader->size());
      if (std::dynamic_pointer_cast<Packed64SingleBlock>(mutable_) != nullptr) {
        // make sure that we used the right format so that the reader has
        // the same performance characteristics as the mutable that has been
        // serialized
        assertTrue(std::dynamic_pointer_cast<Packed64SingleBlock>(reader) !=
                   nullptr);
      } else {
        assertFalse(std::dynamic_pointer_cast<Packed64SingleBlock>(reader) !=
                    nullptr);
      }
      for (int i = 0; i < valueCount; ++i) {
        TestUtil::assertEquals(mutable_->get(i), reader->get(i));
      }
      delete in_;
      directory->deleteFile(L"packed-ints.bin");
    }
    delete directory;
  }
}

void TestPackedInts::testEncodeDecode()
{
  for (PackedInts::Format format : PackedInts::Format::values()) {
    for (int bpv = 1; bpv <= 64; ++bpv) {
      if (!format.isSupported(bpv)) {
        continue;
      }
      wstring msg = format + L" " + to_wstring(bpv);

      shared_ptr<PackedInts::Encoder> *const encoder =
          PackedInts::getEncoder(format, PackedInts::VERSION_CURRENT, bpv);
      shared_ptr<PackedInts::Decoder> *const decoder =
          PackedInts::getDecoder(format, PackedInts::VERSION_CURRENT, bpv);
      constexpr int longBlockCount = encoder->longBlockCount();
      constexpr int longValueCount = encoder->longValueCount();
      constexpr int byteBlockCount = encoder->byteBlockCount();
      constexpr int byteValueCount = encoder->byteValueCount();
      TestUtil::assertEquals(longBlockCount, decoder->longBlockCount());
      TestUtil::assertEquals(longValueCount, decoder->longValueCount());
      TestUtil::assertEquals(byteBlockCount, decoder->byteBlockCount());
      TestUtil::assertEquals(byteValueCount, decoder->byteValueCount());

      constexpr int longIterations = random()->nextInt(100);
      constexpr int byteIterations =
          longIterations * longValueCount / byteValueCount;
      TestUtil::assertEquals(longIterations * longValueCount,
                             byteIterations * byteValueCount);
      constexpr int blocksOffset = random()->nextInt(100);
      constexpr int valuesOffset = random()->nextInt(100);
      constexpr int blocksOffset2 = random()->nextInt(100);
      constexpr int blocksLen = longIterations * longBlockCount;

      // 1. generate random inputs
      const std::deque<int64_t> blocks =
          std::deque<int64_t>(blocksOffset + blocksLen);
      for (int i = 0; i < blocks.size(); ++i) {
        blocks[i] = random()->nextLong();
        if (format == PackedInts::Format::PACKED_SINGLE_BLOCK &&
            64 % bpv != 0) {
          // clear highest bits for packed
          constexpr int toClear = 64 % bpv;
          blocks[i] = static_cast<int>(
              static_cast<unsigned int>((blocks[i] << toClear)) >> toClear);
        }
      }

      // 2. decode
      const std::deque<int64_t> values = std::deque<int64_t>(
          valuesOffset + longIterations * longValueCount);
      decoder->decode(blocks, blocksOffset, values, valuesOffset,
                      longIterations);
      for (auto value : values) {
        assertTrue(value <= PackedInts::maxValue(bpv));
      }
      // test decoding to int[]
      const std::deque<int> intValues;
      if (bpv <= 32) {
        intValues = std::deque<int>(values.size());
        decoder->decode(blocks, blocksOffset, intValues, valuesOffset,
                        longIterations);
        assertTrue(equals(intValues, values));
      } else {
        intValues.clear();
      }

      // 3. re-encode
      const std::deque<int64_t> blocks2 =
          std::deque<int64_t>(blocksOffset2 + blocksLen);
      encoder->encode(values, valuesOffset, blocks2, blocksOffset2,
                      longIterations);
      assertArrayEquals(
          msg, Arrays::copyOfRange(blocks, blocksOffset, blocks.size()),
          Arrays::copyOfRange(blocks2, blocksOffset2, blocks2.size()));
      // test encoding from int[]
      if (bpv <= 32) {
        const std::deque<int64_t> blocks3 =
            std::deque<int64_t>(blocks2.size());
        encoder->encode(intValues, valuesOffset, blocks3, blocksOffset2,
                        longIterations);
        assertArrayEquals(msg, blocks2, blocks3);
      }

      // 4. byte[] decoding
      const std::deque<char> byteBlocks = std::deque<char>(8 * blocks.size());
      ByteBuffer::wrap(byteBlocks).asLongBuffer().put(blocks);
      const std::deque<int64_t> values2 = std::deque<int64_t>(
          valuesOffset + longIterations * longValueCount);
      decoder->decode(byteBlocks, blocksOffset * 8, values2, valuesOffset,
                      byteIterations);
      for (auto value : values2) {
        assertTrue(msg, value <= PackedInts::maxValue(bpv));
      }
      assertArrayEquals(msg, values, values2);
      // test decoding to int[]
      if (bpv <= 32) {
        const std::deque<int> intValues2 = std::deque<int>(values2.size());
        decoder->decode(byteBlocks, blocksOffset * 8, intValues2, valuesOffset,
                        byteIterations);
        assertTrue(msg, equals(intValues2, values2));
      }

      // 5. byte[] encoding
      const std::deque<char> blocks3 =
          std::deque<char>(8 * (blocksOffset2 + blocksLen));
      encoder->encode(values, valuesOffset, blocks3, 8 * blocksOffset2,
                      byteIterations);
      assertEquals(msg, LongBuffer::wrap(blocks2),
                   ByteBuffer::wrap(blocks3).asLongBuffer());
      // test encoding from int[]
      if (bpv <= 32) {
        const std::deque<char> blocks4 = std::deque<char>(blocks3.size());
        encoder->encode(intValues, valuesOffset, blocks4, 8 * blocksOffset2,
                        byteIterations);
        assertArrayEquals(msg, blocks3, blocks4);
      }
    }
  }
}

bool TestPackedInts::equals(std::deque<int> &ints,
                            std::deque<int64_t> &longs)
{
  if (ints.size() != longs.size()) {
    return false;
  }
  for (int i = 0; i < ints.size(); ++i) {
    if ((ints[i] & 0xFFFFFFFFLL) != longs[i]) {
      return false;
    }
  }
  return true;
}

void TestPackedInts::testPackedLongValuesOnZeros()
{
  // Make sure that when all values are the same, they use 0 bits per value
  constexpr int pageSize = 1 << TestUtil::nextInt(random(), 6, 20);
  constexpr float acceptableOverheadRatio = random()->nextFloat();

  TestUtil::assertEquals(
      PackedLongValues::packedBuilder(pageSize, acceptableOverheadRatio)
          ->add(0)
          ->build()
          ->ramBytesUsed(),
      PackedLongValues::packedBuilder(pageSize, acceptableOverheadRatio)
          ->add(0)
          ->add(0)
          ->build()
          ->ramBytesUsed());

  constexpr int64_t l = random()->nextLong();
  TestUtil::assertEquals(
      PackedLongValues::deltaPackedBuilder(pageSize, acceptableOverheadRatio)
          ->add(l)
          ->build()
          ->ramBytesUsed(),
      PackedLongValues::deltaPackedBuilder(pageSize, acceptableOverheadRatio)
          ->add(l)
          ->add(l)
          ->build()
          ->ramBytesUsed());

  constexpr int64_t avg = random()->nextInt(100);
  TestUtil::assertEquals(
      PackedLongValues::monotonicBuilder(pageSize, acceptableOverheadRatio)
          ->add(l)
          ->add(l + avg)
          ->build()
          ->ramBytesUsed(),
      PackedLongValues::monotonicBuilder(pageSize, acceptableOverheadRatio)
          ->add(l)
          ->add(l + avg)
          ->add(l + 2 * avg)
          ->build()
          ->ramBytesUsed());
}

void TestPackedInts::testPackedLongValues()
{
  const std::deque<int64_t> arr =
      std::deque<int64_t>(RandomNumbers::randomIntBetween(
          random(), 1, TEST_NIGHTLY ? 1000000 : 100000));
  std::deque<float> ratioOptions = {PackedInts::DEFAULT, PackedInts::COMPACT,
                                     PackedInts::FAST};
  for (auto bpv : std::deque<int>{
           0, 1, 63, 64, RandomNumbers::randomIntBetween(random(), 2, 62)}) {
    for (DataType dataType : Arrays::asList(DataType::DELTA_PACKED)) {
      constexpr int pageSize = 1 << TestUtil::nextInt(random(), 6, 20);
      float acceptableOverheadRatio =
          ratioOptions[TestUtil::nextInt(random(), 0, ratioOptions.size() - 1)];
      shared_ptr<PackedLongValues::Builder> buf;
      constexpr int inc;
      switch (dataType) {
      case org::apache::lucene::util::packed::TestPackedInts::DataType::PACKED:
        buf =
            PackedLongValues::packedBuilder(pageSize, acceptableOverheadRatio);
        inc = 0;
        break;
      case org::apache::lucene::util::packed::TestPackedInts::DataType::
          DELTA_PACKED:
        buf = PackedLongValues::deltaPackedBuilder(pageSize,
                                                   acceptableOverheadRatio);
        inc = 0;
        break;
      case org::apache::lucene::util::packed::TestPackedInts::DataType::
          MONOTONIC:
        buf = PackedLongValues::monotonicBuilder(pageSize,
                                                 acceptableOverheadRatio);
        inc = TestUtil::nextInt(random(), -1000, 1000);
        break;
      default:
        throw runtime_error(L"added a type and forgot to add it here?");
      }

      if (bpv == 0) {
        arr[0] = random()->nextLong();
        for (int i = 1; i < arr.size(); ++i) {
          arr[i] = arr[i - 1] + inc;
        }
      } else if (bpv == 64) {
        for (int i = 0; i < arr.size(); ++i) {
          arr[i] = random()->nextLong();
        }
      } else {
        constexpr int64_t minValue = TestUtil::nextLong(
            random(), numeric_limits<int64_t>::min(),
            numeric_limits<int64_t>::max() - PackedInts::maxValue(bpv));
        for (int i = 0; i < arr.size(); ++i) {
          arr[i] = minValue + inc * i + random()->nextLong() &
                   PackedInts::maxValue(bpv); // _TestUtil.nextLong is too slow
        }
      }

      for (int i = 0; i < arr.size(); ++i) {
        buf->add(arr[i]);
        if (rarely()) {
          constexpr int64_t expectedBytesUsed = RamUsageTester::sizeOf(buf);
          constexpr int64_t computedBytesUsed = buf->ramBytesUsed();
          TestUtil::assertEquals(expectedBytesUsed, computedBytesUsed);
        }
      }
      TestUtil::assertEquals(arr.size(), buf->size());
      shared_ptr<PackedLongValues> *const values = buf->build();
      expectThrows(IllegalStateException::typeid,
                   [&]() { buf->add(random()->nextLong()); });
      TestUtil::assertEquals(arr.size(), values->size());

      for (int i = 0; i < arr.size(); ++i) {
        TestUtil::assertEquals(arr[i], values->get(i));
      }

      shared_ptr<PackedLongValues::Iterator> *const it = values->begin();
      for (int i = 0; i < arr.size(); ++i) {
        if (random()->nextBoolean()) {
          assertTrue(it->hasNext());
        }
        TestUtil::assertEquals(arr[i], it->next());
      }
      assertFalse(it->hasNext());

      constexpr int64_t expectedBytesUsed = RamUsageTester::sizeOf(values);
      constexpr int64_t computedBytesUsed = values->ramBytesUsed();
      TestUtil::assertEquals(expectedBytesUsed, computedBytesUsed);
    }
  }
}

void TestPackedInts::testPackedInputOutput() 
{
  const std::deque<int64_t> longs =
      std::deque<int64_t>(random()->nextInt(8192));
  const std::deque<int> bitsPerValues = std::deque<int>(longs.size());
  const std::deque<bool> skip = std::deque<bool>(longs.size());
  for (int i = 0; i < longs.size(); ++i) {
    constexpr int bpv = RandomNumbers::randomIntBetween(random(), 1, 64);
    bitsPerValues[i] =
        random()->nextBoolean() ? bpv : TestUtil::nextInt(random(), bpv, 64);
    if (bpv == 64) {
      longs[i] = random()->nextLong();
    } else {
      longs[i] = TestUtil::nextLong(random(), 0, PackedInts::maxValue(bpv));
    }
    skip[i] = rarely();
  }

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexOutput> *const out =
      dir->createOutput(L"out.bin", IOContext::DEFAULT);
  shared_ptr<PackedDataOutput> pout = make_shared<PackedDataOutput>(out);
  int64_t totalBits = 0;
  for (int i = 0; i < longs.size(); ++i) {
    pout->writeLong(longs[i], bitsPerValues[i]);
    totalBits += bitsPerValues[i];
    if (skip[i]) {
      pout->flush();
      totalBits =
          8 * static_cast<int64_t>(ceil(static_cast<double>(totalBits) / 8));
    }
  }
  pout->flush();
  TestUtil::assertEquals(
      static_cast<int64_t>(ceil(static_cast<double>(totalBits) / 8)),
      out->getFilePointer());
  delete out;
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"out.bin", IOContext::READONCE);
  shared_ptr<PackedDataInput> *const pin = make_shared<PackedDataInput>(in_);
  for (int i = 0; i < longs.size(); ++i) {
    assertEquals(L"" + to_wstring(i), longs[i],
                 pin->readLong(bitsPerValues[i]));
    if (skip[i]) {
      pin->skipToNextByte();
    }
  }
  TestUtil::assertEquals(
      static_cast<int64_t>(ceil(static_cast<double>(totalBits) / 8)),
      in_->getFilePointer());
  delete in_;
  delete dir;
}

void TestPackedInts::testBlockPackedReaderWriter() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int blockSize = 1 << TestUtil::nextInt(random(), 6, 18);
    constexpr int valueCount = random()->nextInt(1 << 18);
    const std::deque<int64_t> values = std::deque<int64_t>(valueCount);
    int64_t minValue = 0;
    int bpv = 0;
    for (int i = 0; i < valueCount; ++i) {
      if (i % blockSize == 0) {
        minValue = rarely() ? random()->nextInt(256)
                            : rarely() ? -5 : random()->nextLong();
        bpv = random()->nextInt(65);
      }
      if (bpv == 0) {
        values[i] = minValue;
      } else if (bpv == 64) {
        values[i] = random()->nextLong();
      } else {
        values[i] =
            minValue + TestUtil::nextLong(random(), 0, (1LL << bpv) - 1);
      }
    }

    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<IndexOutput> *const out =
        dir->createOutput(L"out.bin", IOContext::DEFAULT);
    shared_ptr<BlockPackedWriter> *const writer =
        make_shared<BlockPackedWriter>(out, blockSize);
    for (int i = 0; i < valueCount; ++i) {
      TestUtil::assertEquals(i, writer->ord());
      writer->add(values[i]);
    }
    TestUtil::assertEquals(valueCount, writer->ord());
    writer->finish();
    TestUtil::assertEquals(valueCount, writer->ord());
    constexpr int64_t fp = out->getFilePointer();
    delete out;

    shared_ptr<IndexInput> in1 = dir->openInput(L"out.bin", IOContext::DEFAULT);
    std::deque<char> buf(static_cast<int>(fp));
    in1->readBytes(buf, 0, static_cast<int>(fp));
    in1->seek(0LL);
    shared_ptr<ByteArrayDataInput> in2 = make_shared<ByteArrayDataInput>(buf);
    shared_ptr<DataInput> *const in_ = random()->nextBoolean() ? in1 : in2;
    shared_ptr<BlockPackedReaderIterator> *const it =
        make_shared<BlockPackedReaderIterator>(in_, PackedInts::VERSION_CURRENT,
                                               blockSize, valueCount);
    for (int i = 0; i < valueCount;) {
      if (random()->nextBoolean()) {
        assertEquals(L"" + to_wstring(i), values[i], it->next());
        ++i;
      } else {
        shared_ptr<LongsRef> *const nextValues =
            it->next(TestUtil::nextInt(random(), 1, 1024));
        for (int j = 0; j < nextValues->length; ++j) {
          assertEquals(L"" + to_wstring(i + j), values[i + j],
                       nextValues->longs[nextValues->offset + j]);
        }
        i += nextValues->length;
      }
      TestUtil::assertEquals(i, it->ord());
    }
    TestUtil::assertEquals(
        fp,
        std::dynamic_pointer_cast<ByteArrayDataInput>(in_) != nullptr
            ? (std::static_pointer_cast<ByteArrayDataInput>(in_))->getPosition()
            : (std::static_pointer_cast<IndexInput>(in_))->getFilePointer());
    expectThrows(IOException::typeid, [&]() { it->next(); });

    if (std::dynamic_pointer_cast<ByteArrayDataInput>(in_) != nullptr) {
      (std::static_pointer_cast<ByteArrayDataInput>(in_))->setPosition(0);
    } else {
      (std::static_pointer_cast<IndexInput>(in_))->seek(0LL);
    }
    shared_ptr<BlockPackedReaderIterator> *const it2 =
        make_shared<BlockPackedReaderIterator>(in_, PackedInts::VERSION_CURRENT,
                                               blockSize, valueCount);
    int i = 0;
    while (true) {
      constexpr int skip = TestUtil::nextInt(random(), 0, valueCount - i);
      it2->skip(skip);
      i += skip;
      TestUtil::assertEquals(i, it2->ord());
      if (i == valueCount) {
        break;
      } else {
        TestUtil::assertEquals(values[i], it2->next());
        ++i;
      }
    }
    TestUtil::assertEquals(
        fp,
        std::dynamic_pointer_cast<ByteArrayDataInput>(in_) != nullptr
            ? (std::static_pointer_cast<ByteArrayDataInput>(in_))->getPosition()
            : (std::static_pointer_cast<IndexInput>(in_))->getFilePointer());
    expectThrows(IOException::typeid, [&]() { it2->skip(1); });

    in1->seek(0LL);
    shared_ptr<BlockPackedReader> *const reader =
        make_shared<BlockPackedReader>(in1, PackedInts::VERSION_CURRENT,
                                       blockSize, valueCount,
                                       random()->nextBoolean());
    TestUtil::assertEquals(in1->getFilePointer(), in1->length());
    for (i = 0; i < valueCount; ++i) {
      assertEquals(L"i=" + to_wstring(i), values[i], reader->get(i));
    }
    delete in1;
    delete dir;
  }
}

void TestPackedInts::testMonotonicBlockPackedReaderWriter() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int blockSize = 1 << TestUtil::nextInt(random(), 6, 18);
    constexpr int valueCount = random()->nextInt(1 << 18);
    const std::deque<int64_t> values = std::deque<int64_t>(valueCount);
    if (valueCount > 0) {
      values[0] = random()->nextBoolean()
                      ? random()->nextInt(10)
                      : random()->nextInt(numeric_limits<int>::max());
      int maxDelta = random()->nextInt(64);
      for (int i = 1; i < valueCount; ++i) {
        if (random()->nextDouble() < 0.1) {
          maxDelta = random()->nextInt(64);
        }
        values[i] =
            max(0, values[i - 1] + TestUtil::nextInt(random(), -16, maxDelta));
      }
    }

    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<IndexOutput> *const out =
        dir->createOutput(L"out.bin", IOContext::DEFAULT);
    shared_ptr<MonotonicBlockPackedWriter> *const writer =
        make_shared<MonotonicBlockPackedWriter>(out, blockSize);
    for (int i = 0; i < valueCount; ++i) {
      TestUtil::assertEquals(i, writer->ord());
      writer->add(values[i]);
    }
    TestUtil::assertEquals(valueCount, writer->ord());
    writer->finish();
    TestUtil::assertEquals(valueCount, writer->ord());
    constexpr int64_t fp = out->getFilePointer();
    delete out;

    shared_ptr<IndexInput> *const in_ =
        dir->openInput(L"out.bin", IOContext::DEFAULT);
    shared_ptr<MonotonicBlockPackedReader> *const reader =
        MonotonicBlockPackedReader::of(in_, PackedInts::VERSION_CURRENT,
                                       blockSize, valueCount,
                                       random()->nextBoolean());
    TestUtil::assertEquals(fp, in_->getFilePointer());
    for (int i = 0; i < valueCount; ++i) {
      assertEquals(L"i=" + to_wstring(i), values[i], reader->get(i));
    }
    delete in_;
    delete dir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testBlockReaderOverflow() throws
// java.io.IOException
void TestPackedInts::testBlockReaderOverflow() 
{
  constexpr int64_t valueCount = TestUtil::nextLong(
      random(), 1LL + numeric_limits<int>::max(),
      static_cast<int64_t>(numeric_limits<int>::max()) * 2);
  constexpr int blockSize = 1 << TestUtil::nextInt(random(), 20, 22);
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexOutput> *const out =
      dir->createOutput(L"out.bin", IOContext::DEFAULT);
  shared_ptr<BlockPackedWriter> *const writer =
      make_shared<BlockPackedWriter>(out, blockSize);
  int64_t value = random()->nextInt() & 0xFFFFFFFFLL;
  int64_t valueOffset = TestUtil::nextLong(random(), 0, valueCount - 1);
  for (int64_t i = 0; i < valueCount;) {
    TestUtil::assertEquals(i, writer->ord());
    if ((i & (blockSize - 1)) == 0 &&
        (i + blockSize < valueOffset ||
         i > valueOffset && i + blockSize < valueCount)) {
      writer->addBlockOfZeros();
      i += blockSize;
    } else if (i == valueOffset) {
      writer->add(value);
      ++i;
    } else {
      writer->add(0);
      ++i;
    }
  }
  writer->finish();
  delete out;
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"out.bin", IOContext::DEFAULT);
  shared_ptr<BlockPackedReaderIterator> *const it =
      make_shared<BlockPackedReaderIterator>(in_, PackedInts::VERSION_CURRENT,
                                             blockSize, valueCount);
  it->skip(valueOffset);
  TestUtil::assertEquals(value, it->next());
  in_->seek(0LL);
  shared_ptr<BlockPackedReader> *const reader = make_shared<BlockPackedReader>(
      in_, PackedInts::VERSION_CURRENT, blockSize, valueCount,
      random()->nextBoolean());
  TestUtil::assertEquals(value, reader->get(valueOffset));
  for (int i = 0; i < 5; ++i) {
    constexpr int64_t offset =
        TestUtil::nextLong(random(), 0, valueCount - 1);
    if (offset == valueOffset) {
      TestUtil::assertEquals(value, reader->get(offset));
    } else {
      TestUtil::assertEquals(0, reader->get(offset));
    }
  }
  delete in_;
  delete dir;
}
} // namespace org::apache::lucene::util::packed