using namespace std;

#include "BaseLiveDocsFormatTestCase.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void BaseLiveDocsFormatTestCase::setUp() 
{
  LuceneTestCase::setUp();
  // set the default codec, so adding test cases to this isn't fragile
  savedCodec = Codec::getDefault();
  Codec::setDefault(getCodec());
}

void BaseLiveDocsFormatTestCase::tearDown() 
{
  Codec::setDefault(savedCodec); // restore
  LuceneTestCase::tearDown();
}

void BaseLiveDocsFormatTestCase::testDenseLiveDocs() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 3, 1000);
  testSerialization(maxDoc, maxDoc - 1, false);
  testSerialization(maxDoc, maxDoc - 1, true);
}

void BaseLiveDocsFormatTestCase::testEmptyLiveDocs() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 3, 1000);
  testSerialization(maxDoc, 0, false);
  testSerialization(maxDoc, 0, true);
}

void BaseLiveDocsFormatTestCase::testSparseLiveDocs() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 3, 1000);
  testSerialization(maxDoc, 1, false);
  testSerialization(maxDoc, 1, true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Monster("Uses lots of memory") public void testOverflow()
// throws java.io.IOException
void BaseLiveDocsFormatTestCase::testOverflow() 
{
  testSerialization(IndexWriter::MAX_DOCS, IndexWriter::MAX_DOCS - 7, false);
}

void BaseLiveDocsFormatTestCase::testSerialization(
    int maxDoc, int numLiveDocs, bool fixedBitSet) 
{
  shared_ptr<Codec> *const codec = Codec::getDefault();
  shared_ptr<LiveDocsFormat> *const format = codec->liveDocsFormat();

  shared_ptr<FixedBitSet> *const liveDocs = make_shared<FixedBitSet>(maxDoc);
  if (numLiveDocs > maxDoc / 2) {
    liveDocs->set(0, maxDoc);
    for (int i = 0; i < maxDoc - numLiveDocs; ++i) {
      int clearBit;
      do {
        clearBit = random()->nextInt(maxDoc);
      } while (liveDocs->get(clearBit) == false);
      liveDocs->clear(clearBit);
    }
  } else {
    for (int i = 0; i < numLiveDocs; ++i) {
      int setBit;
      do {
        setBit = random()->nextInt(maxDoc);
      } while (liveDocs->get(setBit));
      liveDocs->set(setBit);
    }
  }

  shared_ptr<Bits> *const bits;
  if (fixedBitSet) {
    bits = liveDocs;
  } else {
    // Make sure the impl doesn't only work with a FixedBitSet
    bits = make_shared<BitsAnonymousInnerClass>(shared_from_this(), liveDocs);
  }

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"foo", maxDoc,
      random()->nextBoolean(), codec, Collections::emptyMap(),
      StringHelper::randomId(), Collections::emptyMap(), nullptr);
  shared_ptr<SegmentCommitInfo> sci =
      make_shared<SegmentCommitInfo>(si, 0, 0, 0, -1, -1);
  format->writeLiveDocs(bits, dir, sci, maxDoc - numLiveDocs,
                        IOContext::DEFAULT);

  sci = make_shared<SegmentCommitInfo>(si, maxDoc - numLiveDocs, 0, 1, -1, -1);
  shared_ptr<Bits> *const bits2 =
      format->readLiveDocs(dir, sci, IOContext::READONCE);
  TestUtil::assertEquals(maxDoc, bits2->length());
  for (int i = 0; i < maxDoc; ++i) {
    TestUtil::assertEquals(bits->get(i), bits2->get(i));
  }
  delete dir;
}

BaseLiveDocsFormatTestCase::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<BaseLiveDocsFormatTestCase> outerInstance,
    shared_ptr<FixedBitSet> liveDocs)
{
  this->outerInstance = outerInstance;
  this->liveDocs = liveDocs;
}

bool BaseLiveDocsFormatTestCase::BitsAnonymousInnerClass::get(int index)
{
  return liveDocs->get(index);
}

int BaseLiveDocsFormatTestCase::BitsAnonymousInnerClass::length()
{
  return liveDocs->length();
}
} // namespace org::apache::lucene::index