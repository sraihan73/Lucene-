using namespace std;

#include "RandomCodec.h"

namespace org::apache::lucene::index
{
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using AssertingDocValuesFormat =
    org::apache::lucene::codecs::asserting::AssertingDocValuesFormat;
using AssertingPointsFormat =
    org::apache::lucene::codecs::asserting::AssertingPointsFormat;
using AssertingPostingsFormat =
    org::apache::lucene::codecs::asserting::AssertingPostingsFormat;
using LuceneFixedGap = org::apache::lucene::codecs::blockterms::LuceneFixedGap;
using LuceneVarGapDocFreqInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapDocFreqInterval;
using LuceneVarGapFixedInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapFixedInterval;
using BlockTreeOrdsPostingsFormat =
    org::apache::lucene::codecs::blocktreeords::BlockTreeOrdsPostingsFormat;
using TestBloomFilteredLucenePostings =
    org::apache::lucene::codecs::bloom::TestBloomFilteredLucenePostings;
using Lucene60PointsReader =
    org::apache::lucene::codecs::lucene60::Lucene60PointsReader;
using Lucene60PointsWriter =
    org::apache::lucene::codecs::lucene60::Lucene60PointsWriter;
using DirectDocValuesFormat =
    org::apache::lucene::codecs::memory::DirectDocValuesFormat;
using DirectPostingsFormat =
    org::apache::lucene::codecs::memory::DirectPostingsFormat;
using FSTOrdPostingsFormat =
    org::apache::lucene::codecs::memory::FSTOrdPostingsFormat;
using FSTPostingsFormat =
    org::apache::lucene::codecs::memory::FSTPostingsFormat;
using MemoryDocValuesFormat =
    org::apache::lucene::codecs::memory::MemoryDocValuesFormat;
using MemoryPostingsFormat =
    org::apache::lucene::codecs::memory::MemoryPostingsFormat;
using MockRandomPostingsFormat =
    org::apache::lucene::codecs::mockrandom::MockRandomPostingsFormat;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

shared_ptr<PointsFormat> RandomCodec::pointsFormat()
{
  return make_shared<AssertingPointsFormat>(
      make_shared<PointsFormatAnonymousInnerClass>(shared_from_this()));
}

RandomCodec::PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
    shared_ptr<RandomCodec> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointsWriter>
RandomCodec::PointsFormatAnonymousInnerClass::fieldsWriter(
    shared_ptr<SegmentWriteState> writeState) 
{

  // Randomize how BKDWriter chooses its splis:

  return make_shared<Lucene60PointsWriterAnonymousInnerClass>(
      shared_from_this(), writeState, outerInstance->maxPointsInLeafNode,
      outerInstance->maxMBSortInHeap);
}

RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::
        Lucene60PointsWriterAnonymousInnerClass(
            shared_ptr<PointsFormatAnonymousInnerClass> outerInstance,
            shared_ptr<org::apache::lucene::index::SegmentWriteState>
                writeState,
            int maxPointsInLeafNode, double maxMBSortInHeap)
    : org::apache::lucene::codecs::lucene60::Lucene60PointsWriter(
          writeState, maxPointsInLeafNode, maxMBSortInHeap)
{
  this->outerInstance = outerInstance;
  this->writeState = writeState;
}

void RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::writeField(
        shared_ptr<FieldInfo> fieldInfo,
        shared_ptr<PointsReader> reader) 
{

  shared_ptr<PointValues> values = reader->getValues(fieldInfo->name);
  bool singleValuePerDoc = values->size() == values->getDocCount();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.util.bkd.BKDWriter writer
  // = new RandomlySplittingBKDWriter(writeState.segmentInfo.maxDoc(),
  // writeState.directory, writeState.segmentInfo.name,
  // fieldInfo.getPointDimensionCount(), fieldInfo.getPointNumBytes(),
  // maxPointsInLeafNode, maxMBSortInHeap, values.size(), singleValuePerDoc,
  // bkdSplitRandomSeed^fieldInfo.name.hashCode()))
  {
    org::apache::lucene::util::bkd::BKDWriter writer =
        RandomlySplittingBKDWriter(
            writeState->segmentInfo->maxDoc(), writeState->directory,
            writeState->segmentInfo->name, fieldInfo->getPointDimensionCount(),
            fieldInfo->getPointNumBytes(),
            outerInstance->outerInstance.maxPointsInLeafNode,
            outerInstance->outerInstance.maxMBSortInHeap, values->size(),
            singleValuePerDoc,
            outerInstance->outerInstance.bkdSplitRandomSeed ^
                fieldInfo->name.hashCode());
    values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
        shared_from_this(), writer));

    // We could have 0 points on merge since all docs with dimensional fields
    // may be deleted:
    if (writer->getPointCount() > 0) {
      indexFPs::put(fieldInfo->name, writer->finish(dataOut));
    }
  }
}

RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::
        IntersectVisitorAnonymousInnerClass::
            IntersectVisitorAnonymousInnerClass(
                shared_ptr<Lucene60PointsWriterAnonymousInnerClass>
                    outerInstance,
                shared_ptr<BKDWriter> writer)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
}

void RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::
        IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  throw make_shared<IllegalStateException>();
}

void RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::
        IntersectVisitorAnonymousInnerClass::visit(
            int docID, std::deque<char> &packedValue) 
{
  writer->add(packedValue, docID);
}

PointValues::Relation RandomCodec::PointsFormatAnonymousInnerClass::
    Lucene60PointsWriterAnonymousInnerClass::
        IntersectVisitorAnonymousInnerClass::compare(
            std::deque<char> &minPackedValue,
            std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

shared_ptr<PointsReader>
RandomCodec::PointsFormatAnonymousInnerClass::fieldsReader(
    shared_ptr<SegmentReadState> readState) 
{
  return make_shared<Lucene60PointsReader>(readState);
}

shared_ptr<PostingsFormat>
RandomCodec::getPostingsFormatForField(const wstring &name)
{
  shared_ptr<PostingsFormat> codec = previousMappings[name];
  if (codec == nullptr) {
    codec = formats[abs(perFieldSeed ^ name.hashCode()) % formats.size()];
    previousMappings.emplace(name, codec);
    // Safety:
    assert((previousMappings.size() < 10000, L"test went insane"));
  }
  return codec;
}

shared_ptr<DocValuesFormat>
RandomCodec::getDocValuesFormatForField(const wstring &name)
{
  shared_ptr<DocValuesFormat> codec = previousDVMappings[name];
  if (codec == nullptr) {
    codec = dvFormats[abs(perFieldSeed ^ name.hashCode()) % dvFormats.size()];
    previousDVMappings.emplace(name, codec);
    // Safety:
    assert((previousDVMappings.size() < 10000, L"test went insane"));
  }
  return codec;
}

RandomCodec::RandomCodec(shared_ptr<Random> random,
                         shared_ptr<Set<wstring>> avoidCodecs)
    : avoidCodecs(avoidCodecs), perFieldSeed(random->nextInt()),
      maxPointsInLeafNode(TestUtil::nextInt(random, 16, 2048)),
      maxMBSortInHeap(5.0 + (3 * random->nextDouble())),
      bkdSplitRandomSeed(random->nextInt())
{
  // TODO: make it possible to specify min/max iterms per
  // block via CL:
  int minItemsPerBlock = TestUtil::nextInt(random, 2, 100);
  int maxItemsPerBlock =
      2 * (max(2, minItemsPerBlock - 1)) + random->nextInt(100);
  int lowFreqCutoff = TestUtil::nextInt(random, 2, 100);

  add(avoidCodecs,
      {TestUtil::getDefaultPostingsFormat(minItemsPerBlock, maxItemsPerBlock),
       make_shared<FSTPostingsFormat>(), make_shared<FSTOrdPostingsFormat>(),
       make_shared<DirectPostingsFormat>(
           LuceneTestCase::rarely(random)
               ? 1
               : (LuceneTestCase::rarely(random) ? numeric_limits<int>::max()
                                                 : maxItemsPerBlock),
           LuceneTestCase::rarely(random)
               ? 1
               : (LuceneTestCase::rarely(random) ? numeric_limits<int>::max()
                                                 : lowFreqCutoff)),
       make_shared<TestBloomFilteredLucenePostings>(),
       make_shared<MockRandomPostingsFormat>(random),
       make_shared<BlockTreeOrdsPostingsFormat>(minItemsPerBlock,
                                                maxItemsPerBlock),
       make_shared<LuceneFixedGap>(TestUtil::nextInt(random, 1, 1000)),
       make_shared<LuceneVarGapFixedInterval>(
           TestUtil::nextInt(random, 1, 1000)),
       make_shared<LuceneVarGapDocFreqInterval>(
           TestUtil::nextInt(random, 1, 100),
           TestUtil::nextInt(random, 1, 1000)),
       TestUtil::getDefaultPostingsFormat(),
       make_shared<AssertingPostingsFormat>(),
       make_shared<MemoryPostingsFormat>(true, random->nextFloat()),
       make_shared<MemoryPostingsFormat>(false, random->nextFloat())});

  addDocValues(avoidCodecs, {TestUtil::getDefaultDocValuesFormat(),
                             make_shared<DirectDocValuesFormat>(),
                             make_shared<MemoryDocValuesFormat>(),
                             TestUtil::getDefaultDocValuesFormat(),
                             make_shared<AssertingDocValuesFormat>()});

  Collections::shuffle(formats, random);
  Collections::shuffle(dvFormats, random);

  // Avoid too many open files:
  if (formats.size() > 4) {
    formats = formats.subList(0, 4);
  }
  if (dvFormats.size() > 4) {
    dvFormats = dvFormats.subList(0, 4);
  }
}

RandomCodec::RandomCodec(shared_ptr<Random> random)
    : RandomCodec(random, Collections::emptySet<std::wstring>())
{
}

void RandomCodec::add(shared_ptr<Set<wstring>> avoidCodecs,
                      deque<PostingsFormat> &postings)
{
  for (shared_ptr<PostingsFormat> p : postings) {
    if (!avoidCodecs->contains(p->getName())) {
      formats.push_back(p);
      formatNames->add(p->getName());
    }
  }
}

void RandomCodec::addDocValues(shared_ptr<Set<wstring>> avoidCodecs,
                               deque<DocValuesFormat> &docvalues)
{
  for (shared_ptr<DocValuesFormat> d : docvalues) {
    if (!avoidCodecs->contains(d->getName())) {
      dvFormats.push_back(d);
      dvFormatNames->add(d->getName());
    }
  }
}

wstring RandomCodec::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return AssertingCodec::toString() + L": " + previousMappings.toString() +
         // C++ TODO: There is no native C++ equivalent to 'toString':
         L", docValues:" + previousDVMappings.toString() +
         L", maxPointsInLeafNode=" + to_wstring(maxPointsInLeafNode) +
         L", maxMBSortInHeap=" + to_wstring(maxMBSortInHeap);
}

RandomCodec::RandomlySplittingBKDWriter::RandomlySplittingBKDWriter(
    int maxDoc, shared_ptr<Directory> tempDir,
    const wstring &tempFileNamePrefix, int numDims, int bytesPerDim,
    int maxPointsInLeafNode, double maxMBSortInHeap, int64_t totalPointCount,
    bool singleValuePerDoc, int randomSeed) 
    : org::apache::lucene::util::bkd::BKDWriter(
          maxDoc, tempDir, tempFileNamePrefix, numDims, bytesPerDim,
          maxPointsInLeafNode, maxMBSortInHeap, totalPointCount,
          getRandomSingleValuePerDoc(singleValuePerDoc, randomSeed),
          getRandomLongOrds(totalPointCount, singleValuePerDoc, randomSeed),
          getRandomOfflineSorterBufferMB(randomSeed),
          getRandomOfflineSorterMaxTempFiles(randomSeed)),
      random(make_shared<Random>(randomSeed))
{
}

bool RandomCodec::RandomlySplittingBKDWriter::getRandomSingleValuePerDoc(
    bool singleValuePerDoc, int randomSeed)
{
  // If we are single valued, sometimes pretend we aren't:
  return singleValuePerDoc &&
         ((make_shared<Random>(randomSeed))->nextBoolean());
}

bool RandomCodec::RandomlySplittingBKDWriter::getRandomLongOrds(
    int64_t totalPointCount, bool singleValuePerDoc, int randomSeed)
{
  // Always use long ords if we have too many points, but sometimes randomly use
  // it anyway when singleValuePerDoc is false:
  return totalPointCount > numeric_limits<int>::max() ||
         (getRandomSingleValuePerDoc(singleValuePerDoc, randomSeed) == false &&
          (make_shared<Random>(randomSeed))->nextBoolean());
}

int64_t
RandomCodec::RandomlySplittingBKDWriter::getRandomOfflineSorterBufferMB(
    int randomSeed)
{
  return TestUtil::nextInt(make_shared<Random>(randomSeed), 1, 8);
}

int RandomCodec::RandomlySplittingBKDWriter::getRandomOfflineSorterMaxTempFiles(
    int randomSeed)
{
  return TestUtil::nextInt(make_shared<Random>(randomSeed), 2, 20);
}

int RandomCodec::RandomlySplittingBKDWriter::split(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue,
    std::deque<int> &parentDims)
{
  // BKD normally defaults by the widest dimension, to try to make as squarish
  // cells as possible, but we just pick a random one ;)
  return random->nextInt(numDims);
}
} // namespace org::apache::lucene::index