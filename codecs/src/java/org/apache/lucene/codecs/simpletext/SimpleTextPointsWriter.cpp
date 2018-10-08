using namespace std;

#include "SimpleTextPointsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "SimpleTextBKDWriter.h"
#include "SimpleTextPointsFormat.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::NUM_DIMS =
        make_shared<org::apache::lucene::util::BytesRef>(L"num dims ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::BYTES_PER_DIM =
        make_shared<org::apache::lucene::util::BytesRef>(L"bytes per dim ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::MAX_LEAF_POINTS =
        make_shared<org::apache::lucene::util::BytesRef>(L"max leaf points ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::INDEX_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"index count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::BLOCK_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"block count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::BLOCK_DOC_ID =
        make_shared<org::apache::lucene::util::BytesRef>(L"  doc ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::BLOCK_FP =
        make_shared<org::apache::lucene::util::BytesRef>(L"  block fp ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::BLOCK_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"  block value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::SPLIT_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"split count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::SPLIT_DIM =
        make_shared<org::apache::lucene::util::BytesRef>(L"  split dim ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::SPLIT_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"  split value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::FIELD_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"field count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::FIELD_FP_NAME =
        make_shared<org::apache::lucene::util::BytesRef>(L"  field fp name ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::FIELD_FP =
        make_shared<org::apache::lucene::util::BytesRef>(L"  field fp ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::MIN_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"min value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::MAX_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"max value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::POINT_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"point count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::DOC_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"doc count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextPointsWriter::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");

SimpleTextPointsWriter::SimpleTextPointsWriter(
    shared_ptr<SegmentWriteState> writeState) 
    : writeState(writeState)
{
  wstring fileName = IndexFileNames::segmentFileName(
      writeState->segmentInfo->name, writeState->segmentSuffix,
      SimpleTextPointsFormat::POINT_EXTENSION);
  dataOut = writeState->directory->createOutput(fileName, writeState->context);
}

void SimpleTextPointsWriter::writeField(
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<PointsReader> reader) 
{

  shared_ptr<PointValues> values = reader->getValues(fieldInfo->name);
  bool singleValuePerDoc = values->size() == values->getDocCount();

  // We use our own fork of the BKDWriter to customize how it writes the index
  // and blocks to disk:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (SimpleTextBKDWriter writer = new
  // SimpleTextBKDWriter(writeState.segmentInfo.maxDoc(), writeState.directory,
  // writeState.segmentInfo.name, fieldInfo.getPointDimensionCount(),
  // fieldInfo.getPointNumBytes(),
  // SimpleTextBKDWriter.DEFAULT_MAX_POINTS_IN_LEAF_NODE,
  // SimpleTextBKDWriter.DEFAULT_MAX_MB_SORT_IN_HEAP, values.size(),
  // singleValuePerDoc))
  {
    SimpleTextBKDWriter writer = SimpleTextBKDWriter(
        writeState->segmentInfo->maxDoc(), writeState->directory,
        writeState->segmentInfo->name, fieldInfo->getPointDimensionCount(),
        fieldInfo->getPointNumBytes(),
        SimpleTextBKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE,
        SimpleTextBKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP, values->size(),
        singleValuePerDoc);

    values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
        shared_from_this(), writer));

    // We could have 0 points on merge since all docs with points may be
    // deleted:
    if (writer->getPointCount() > 0) {
      indexFPs.emplace(fieldInfo->name, writer->finish(dataOut));
    }
  }
}

SimpleTextPointsWriter::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<SimpleTextPointsWriter> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::SimpleTextBKDWriter>
            writer)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
}

void SimpleTextPointsWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID)
{
  throw make_shared<IllegalStateException>();
}

void SimpleTextPointsWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  writer->add(packedValue, docID);
}

PointValues::Relation
SimpleTextPointsWriter::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

void SimpleTextPointsWriter::finish() 
{
  SimpleTextUtil::write(dataOut, END);
  SimpleTextUtil::writeNewline(dataOut);
  SimpleTextUtil::writeChecksum(dataOut, scratch);
}

SimpleTextPointsWriter::~SimpleTextPointsWriter()
{
  if (dataOut != nullptr) {
    delete dataOut;
    dataOut.reset();

    // Write index file
    wstring fileName = IndexFileNames::segmentFileName(
        writeState->segmentInfo->name, writeState->segmentSuffix,
        SimpleTextPointsFormat::POINT_INDEX_EXTENSION);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput
    // indexOut = writeState.directory.createOutput(fileName,
    // writeState.context))
    {
      org::apache::lucene::store::IndexOutput indexOut =
          writeState->directory->createOutput(fileName, writeState->context);
      int count = indexFPs.size();
      write(indexOut, FIELD_COUNT);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      write(indexOut, Integer::toString(count));
      newline(indexOut);
      for (auto ent : indexFPs) {
        write(indexOut, FIELD_FP_NAME);
        write(indexOut, ent.first);
        newline(indexOut);
        write(indexOut, FIELD_FP);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        write(indexOut, Long::toString(ent.second));
        newline(indexOut);
      }
      SimpleTextUtil::writeChecksum(indexOut, scratch);
    }
  }
}

void SimpleTextPointsWriter::write(shared_ptr<IndexOutput> out,
                                   const wstring &s) 
{
  SimpleTextUtil::write(out, s, scratch);
}

void SimpleTextPointsWriter::writeInt(shared_ptr<IndexOutput> out,
                                      int x) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(out, Integer::toString(x), scratch);
}

void SimpleTextPointsWriter::writeLong(shared_ptr<IndexOutput> out,
                                       int64_t x) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(out, Long::toString(x), scratch);
}

void SimpleTextPointsWriter::write(shared_ptr<IndexOutput> out,
                                   shared_ptr<BytesRef> b) 
{
  SimpleTextUtil::write(out, b);
}

void SimpleTextPointsWriter::newline(shared_ptr<IndexOutput> out) throw(
    IOException)
{
  SimpleTextUtil::writeNewline(out);
}
} // namespace org::apache::lucene::codecs::simpletext