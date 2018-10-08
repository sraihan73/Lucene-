using namespace std;

#include "Lucene60PointsWriter.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/IndexFileNames.h"
#include "../../index/MergeState.h"
#include "../../index/SegmentWriteState.h"
#include "../../store/IndexOutput.h"
#include "../../util/IOUtils.h"
#include "../../util/bkd/BKDReader.h"
#include "../../util/bkd/BKDWriter.h"
#include "../CodecUtil.h"
#include "../MutablePointValues.h"
#include "../PointsReader.h"
#include "Lucene60PointsFormat.h"
#include "Lucene60PointsReader.h"

namespace org::apache::lucene::codecs::lucene60
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

Lucene60PointsWriter::Lucene60PointsWriter(
    shared_ptr<SegmentWriteState> writeState, int maxPointsInLeafNode,
    double maxMBSortInHeap) 
    : dataOut(writeState->directory->createOutput(dataFileName,
                                                  writeState->context)),
      writeState(writeState), maxPointsInLeafNode(maxPointsInLeafNode),
      maxMBSortInHeap(maxMBSortInHeap)
{
  assert(writeState->fieldInfos->hasPointValues());
  wstring dataFileName = IndexFileNames::segmentFileName(
      writeState->segmentInfo->name, writeState->segmentSuffix,
      Lucene60PointsFormat::DATA_EXTENSION);
  bool success = false;
  try {
    CodecUtil::writeIndexHeader(dataOut, Lucene60PointsFormat::DATA_CODEC_NAME,
                                Lucene60PointsFormat::DATA_VERSION_CURRENT,
                                writeState->segmentInfo->getId(),
                                writeState->segmentSuffix);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::closeWhileHandlingException({dataOut});
    }
  }
}

Lucene60PointsWriter::Lucene60PointsWriter(
    shared_ptr<SegmentWriteState> writeState) 
    : Lucene60PointsWriter(writeState,
                           BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE,
                           BKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP)
{
}

void Lucene60PointsWriter::writeField(
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<PointsReader> reader) 
{

  shared_ptr<PointValues> values = reader->getValues(fieldInfo->name);
  bool singleValuePerDoc = values->size() == values->getDocCount();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.util.bkd.BKDWriter writer
  // = new org.apache.lucene.util.bkd.BKDWriter(writeState.segmentInfo.maxDoc(),
  // writeState.directory, writeState.segmentInfo.name,
  // fieldInfo.getPointDimensionCount(), fieldInfo.getPointNumBytes(),
  // maxPointsInLeafNode, maxMBSortInHeap, values.size(), singleValuePerDoc))
  {
    org::apache::lucene::util::bkd::BKDWriter writer =
        org::apache::lucene::util::bkd::BKDWriter(
            writeState->segmentInfo->maxDoc(), writeState->directory,
            writeState->segmentInfo->name, fieldInfo->getPointDimensionCount(),
            fieldInfo->getPointNumBytes(), maxPointsInLeafNode, maxMBSortInHeap,
            values->size(), singleValuePerDoc);

    if (std::dynamic_pointer_cast<MutablePointValues>(values) != nullptr) {
      constexpr int64_t fp = writer->writeField(
          dataOut, fieldInfo->name,
          std::static_pointer_cast<MutablePointValues>(values));
      if (fp != -1) {
        indexFPs.emplace(fieldInfo->name, fp);
      }
      return;
    }

    values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
        shared_from_this(), writer));

    // We could have 0 points on merge since all docs with dimensional fields
    // may be deleted:
    if (writer->getPointCount() > 0) {
      indexFPs.emplace(fieldInfo->name, writer->finish(dataOut));
    }
  }
}

Lucene60PointsWriter::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<Lucene60PointsWriter> outerInstance,
        shared_ptr<BKDWriter> writer)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
}

void Lucene60PointsWriter::IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  throw make_shared<IllegalStateException>();
}

void Lucene60PointsWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  writer->add(packedValue, docID);
}

PointValues::Relation
Lucene60PointsWriter::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

void Lucene60PointsWriter::merge(shared_ptr<MergeState> mergeState) throw(
    IOException)
{
  /**
   * If indexSort is activated and some of the leaves are not sorted the next
   *test will catch that and the non-optimized merge will run. If the readers
   *are all sorted then it's safe to perform a bulk merge of the points.
   **/
  for (auto reader : mergeState->pointsReaders) {
    if (std::dynamic_pointer_cast<Lucene60PointsReader>(reader) != nullptr ==
        false) {
      // We can only bulk merge when all to-be-merged segments use our format:
      PointsWriter::merge(mergeState);
      return;
    }
  }
  for (auto reader : mergeState->pointsReaders) {
    if (reader != nullptr) {
      reader->checkIntegrity();
    }
  }

  for (auto fieldInfo : mergeState->mergeFieldInfos) {
    if (fieldInfo->getPointDimensionCount() != 0) {
      if (fieldInfo->getPointDimensionCount() == 1) {

        bool singleValuePerDoc = true;

        // Worst case total maximum size (if none of the points are deleted):
        int64_t totMaxSize = 0;
        for (int i = 0; i < mergeState->pointsReaders.size(); i++) {
          shared_ptr<PointsReader> reader = mergeState->pointsReaders[i];
          if (reader != nullptr) {
            shared_ptr<FieldInfos> readerFieldInfos = mergeState->fieldInfos[i];
            shared_ptr<FieldInfo> readerFieldInfo =
                readerFieldInfos->fieldInfo(fieldInfo->name);
            if (readerFieldInfo != nullptr &&
                readerFieldInfo->getPointDimensionCount() > 0) {
              shared_ptr<PointValues> values =
                  reader->getValues(fieldInfo->name);
              if (values != nullptr) {
                totMaxSize += values->size();
                singleValuePerDoc &= values->size() == values->getDocCount();
              }
            }
          }
        }

        // System.out.println("MERGE: field=" + fieldInfo.name);
        // Optimize the 1D case to use BKDWriter.merge, which does a single
        // merge sort of the already sorted incoming segments, instead of trying
        // to sort all points again as if we were simply reindexing them:
        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try
        // (org.apache.lucene.util.bkd.BKDWriter writer = new
        // org.apache.lucene.util.bkd.BKDWriter(writeState.segmentInfo.maxDoc(),
        // writeState.directory, writeState.segmentInfo.name,
        // fieldInfo.getPointDimensionCount(), fieldInfo.getPointNumBytes(),
        // maxPointsInLeafNode, maxMBSortInHeap, totMaxSize, singleValuePerDoc))
        {
          org::apache::lucene::util::bkd::BKDWriter writer =
              org::apache::lucene::util::bkd::BKDWriter(
                  writeState->segmentInfo->maxDoc(), writeState->directory,
                  writeState->segmentInfo->name,
                  fieldInfo->getPointDimensionCount(),
                  fieldInfo->getPointNumBytes(), maxPointsInLeafNode,
                  maxMBSortInHeap, totMaxSize, singleValuePerDoc);
          deque<std::shared_ptr<BKDReader>> bkdReaders =
              deque<std::shared_ptr<BKDReader>>();
          deque<std::shared_ptr<MergeState::DocMap>> docMaps =
              deque<std::shared_ptr<MergeState::DocMap>>();
          for (int i = 0; i < mergeState->pointsReaders.size(); i++) {
            shared_ptr<PointsReader> reader = mergeState->pointsReaders[i];

            if (reader != nullptr) {

              // we confirmed this up above
              assert(std::dynamic_pointer_cast<Lucene60PointsReader>(reader) !=
                     nullptr);
              shared_ptr<Lucene60PointsReader> reader60 =
                  std::static_pointer_cast<Lucene60PointsReader>(reader);

              // NOTE: we cannot just use the merged fieldInfo.number (instead
              // of resolving to this reader's FieldInfo as we do below) because
              // field numbers can easily be different when
              // addIndexes(Directory...) copies over segments from another
              // index:

              shared_ptr<FieldInfos> readerFieldInfos =
                  mergeState->fieldInfos[i];
              shared_ptr<FieldInfo> readerFieldInfo =
                  readerFieldInfos->fieldInfo(fieldInfo->name);
              if (readerFieldInfo != nullptr &&
                  readerFieldInfo->getPointDimensionCount() > 0) {
                shared_ptr<BKDReader> bkdReader =
                    reader60->readers[readerFieldInfo->number];
                if (bkdReader != nullptr) {
                  bkdReaders.push_back(bkdReader);
                  docMaps.push_back(mergeState->docMaps[i]);
                }
              }
            }
          }

          int64_t fp = writer->merge(dataOut, docMaps, bkdReaders);
          if (fp != -1) {
            indexFPs.emplace(fieldInfo->name, fp);
          }
        }
      } else {
        mergeOneField(mergeState, fieldInfo);
      }
    }
  }

  finish();
}

void Lucene60PointsWriter::finish() 
{
  if (finished) {
    throw make_shared<IllegalStateException>(L"already finished");
  }
  finished = true;
  CodecUtil::writeFooter(dataOut);

  wstring indexFileName = IndexFileNames::segmentFileName(
      writeState->segmentInfo->name, writeState->segmentSuffix,
      Lucene60PointsFormat::INDEX_EXTENSION);
  // Write index file
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput indexOut
  // = writeState.directory.createOutput(indexFileName, writeState.context))
  {
    org::apache::lucene::store::IndexOutput indexOut =
        writeState->directory->createOutput(indexFileName, writeState->context);
    CodecUtil::writeIndexHeader(indexOut, Lucene60PointsFormat::META_CODEC_NAME,
                                Lucene60PointsFormat::INDEX_VERSION_CURRENT,
                                writeState->segmentInfo->getId(),
                                writeState->segmentSuffix);
    int count = indexFPs.size();
    indexOut->writeVInt(count);
    for (auto ent : indexFPs) {
      shared_ptr<FieldInfo> fieldInfo =
          writeState->fieldInfos->fieldInfo(ent.first);
      if (fieldInfo == nullptr) {
        throw make_shared<IllegalStateException>(
            L"wrote field=\"" + ent.first +
            L"\" but that field doesn't exist in FieldInfos");
      }
      indexOut->writeVInt(fieldInfo->number);
      indexOut->writeVLong(ent.second);
    }
    CodecUtil::writeFooter(indexOut);
  }
}

Lucene60PointsWriter::~Lucene60PointsWriter() { delete dataOut; }
} // namespace org::apache::lucene::codecs::lucene60