using namespace std;

#include "SimpleTextPointsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PointValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/BufferedChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextBKDReader.h"
#include "SimpleTextPointsFormat.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using PointsReader = org::apache::lucene::codecs::PointsReader;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_FP;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BYTES_PER_DIM;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.DOC_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_FP;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_FP_NAME;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.INDEX_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_LEAF_POINTS;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_VALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MIN_VALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.NUM_DIMS;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.POINT_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_DIM;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_VALUE;

SimpleTextPointsReader::SimpleTextPointsReader(
    shared_ptr<SegmentReadState> readState) 
    : dataIn(readState->directory->openInput(fileName, IOContext::DEFAULT)),
      readState(readState)
{
  // Initialize readers now:

  // Read index:
  unordered_map<wstring, int64_t> fieldToFileOffset =
      unordered_map<wstring, int64_t>();

  wstring indexFileName = IndexFileNames::segmentFileName(
      readState->segmentInfo->name, readState->segmentSuffix,
      SimpleTextPointsFormat::POINT_INDEX_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // in = readState.directory.openChecksumInput(indexFileName,
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::ChecksumIndexInput in_ =
        readState->directory->openChecksumInput(
            indexFileName, org::apache::lucene::store::IOContext::DEFAULT);
    readLine(in_);
    int count = parseInt(FIELD_COUNT);
    for (int i = 0; i < count; i++) {
      readLine(in_);
      wstring fieldName = stripPrefix(FIELD_FP_NAME);
      readLine(in_);
      int64_t fp = parseLong(FIELD_FP);
      fieldToFileOffset.emplace(fieldName, fp);
    }
    SimpleTextUtil::checkFooter(in_);
  }

  bool success = false;
  wstring fileName = IndexFileNames::segmentFileName(
      readState->segmentInfo->name, readState->segmentSuffix,
      SimpleTextPointsFormat::POINT_EXTENSION);
  try {
    for (auto ent : fieldToFileOffset) {
      readers.emplace(ent.first, initReader(ent.second));
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

shared_ptr<SimpleTextBKDReader>
SimpleTextPointsReader::initReader(int64_t fp) 
{
  // NOTE: matches what writeIndex does in SimpleTextPointsWriter
  dataIn->seek(fp);
  readLine(dataIn);
  int numDims = parseInt(NUM_DIMS);

  readLine(dataIn);
  int bytesPerDim = parseInt(BYTES_PER_DIM);

  readLine(dataIn);
  int maxPointsInLeafNode = parseInt(MAX_LEAF_POINTS);

  readLine(dataIn);
  int count = parseInt(INDEX_COUNT);

  readLine(dataIn);
  assert(startsWith(MIN_VALUE));
  shared_ptr<BytesRef> minValue =
      SimpleTextUtil::fromBytesRefString(stripPrefix(MIN_VALUE));
  assert(minValue->length == numDims * bytesPerDim);

  readLine(dataIn);
  assert(startsWith(MAX_VALUE));
  shared_ptr<BytesRef> maxValue =
      SimpleTextUtil::fromBytesRefString(stripPrefix(MAX_VALUE));
  assert(maxValue->length == numDims * bytesPerDim);

  readLine(dataIn);
  assert(startsWith(POINT_COUNT));
  int64_t pointCount = parseLong(POINT_COUNT);

  readLine(dataIn);
  assert(startsWith(DOC_COUNT));
  int docCount = parseInt(DOC_COUNT);

  std::deque<int64_t> leafBlockFPs(count);
  for (int i = 0; i < count; i++) {
    readLine(dataIn);
    leafBlockFPs[i] = parseLong(BLOCK_FP);
  }
  readLine(dataIn);
  count = parseInt(SPLIT_COUNT);

  std::deque<char> splitPackedValues;
  int bytesPerIndexEntry;
  if (numDims == 1) {
    bytesPerIndexEntry = bytesPerDim;
  } else {
    bytesPerIndexEntry = 1 + bytesPerDim;
  }
  splitPackedValues = std::deque<char>(count * bytesPerIndexEntry);
  for (int i = 0; i < count; i++) {
    readLine(dataIn);
    int address = bytesPerIndexEntry * i;
    int splitDim = parseInt(SPLIT_DIM);
    if (numDims != 1) {
      splitPackedValues[address++] = static_cast<char>(splitDim);
    }
    readLine(dataIn);
    assert(startsWith(SPLIT_VALUE));
    shared_ptr<BytesRef> br =
        SimpleTextUtil::fromBytesRefString(stripPrefix(SPLIT_VALUE));
    assert(br->length == bytesPerDim);
    System::arraycopy(br->bytes, br->offset, splitPackedValues, address,
                      bytesPerDim);
  }

  return make_shared<SimpleTextBKDReader>(
      dataIn, numDims, maxPointsInLeafNode, bytesPerDim, leafBlockFPs,
      splitPackedValues, minValue->bytes, maxValue->bytes, pointCount,
      docCount);
}

void SimpleTextPointsReader::readLine(shared_ptr<IndexInput> in_) throw(
    IOException)
{
  SimpleTextUtil::readLine(in_, scratch);
}

bool SimpleTextPointsReader::startsWith(shared_ptr<BytesRef> prefix)
{
  return StringHelper::startsWith(scratch->get(), prefix);
}

int SimpleTextPointsReader::parseInt(shared_ptr<BytesRef> prefix)
{
  assert(startsWith(prefix));
  return stoi(stripPrefix(prefix));
}

int64_t SimpleTextPointsReader::parseLong(shared_ptr<BytesRef> prefix)
{
  assert(startsWith(prefix));
  return StringHelper::fromString<int64_t>(stripPrefix(prefix));
}

wstring SimpleTextPointsReader::stripPrefix(shared_ptr<BytesRef> prefix)
{
  return wstring(scratch->bytes(), prefix->length,
                 scratch->length() - prefix->length, StandardCharsets::UTF_8);
}

shared_ptr<PointValues>
SimpleTextPointsReader::getValues(const wstring &fieldName) 
{
  shared_ptr<FieldInfo> fieldInfo = readState->fieldInfos->fieldInfo(fieldName);
  if (fieldInfo == nullptr) {
    throw invalid_argument(L"field=\"" + fieldName + L"\" is unrecognized");
  }
  if (fieldInfo->getPointDimensionCount() == 0) {
    throw invalid_argument(L"field=\"" + fieldName +
                           L"\" did not index points");
  }
  return readers[fieldName];
}

void SimpleTextPointsReader::checkIntegrity() 
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<IndexInput> clone = dataIn->clone();
  clone->seek(0);

  // checksum is fixed-width encoded with 20 bytes, plus 1 byte for newline (the
  // space is included in SimpleTextUtil.CHECKSUM):
  int64_t footerStartPos =
      dataIn->length() - (SimpleTextUtil::CHECKSUM->length + 21);
  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(clone);
  while (true) {
    SimpleTextUtil::readLine(input, scratch);
    if (input->getFilePointer() >= footerStartPos) {
      // Make sure we landed at precisely the right location:
      if (input->getFilePointer() != footerStartPos) {
        throw make_shared<CorruptIndexException>(
            L"SimpleText failure: footer does not start at expected position "
            L"current=" +
                to_wstring(input->getFilePointer()) + L" vs expected=" +
                to_wstring(footerStartPos),
            input);
      }
      SimpleTextUtil::checkFooter(input);
      break;
    }
  }
}

int64_t SimpleTextPointsReader::ramBytesUsed() { return 0LL; }

SimpleTextPointsReader::~SimpleTextPointsReader() { delete dataIn; }

wstring SimpleTextPointsReader::toString()
{
  return L"SimpleTextPointsReader(segment=" + readState->segmentInfo->name +
         L" maxDoc=" + to_wstring(readState->segmentInfo->maxDoc()) + L")";
}
} // namespace org::apache::lucene::codecs::simpletext