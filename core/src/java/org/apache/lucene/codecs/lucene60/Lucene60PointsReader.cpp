using namespace std;

#include "Lucene60PointsReader.h"
#include "../../index/FieldInfo.h"
#include "../../index/IndexFileNames.h"
#include "../../index/PointValues.h"
#include "../../index/SegmentReadState.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountable.h"
#include "../../util/Accountables.h"
#include "../../util/IOUtils.h"
#include "../../util/bkd/BKDReader.h"
#include "../CodecUtil.h"
#include "Lucene60PointsFormat.h"

namespace org::apache::lucene::codecs::lucene60
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using IOUtils = org::apache::lucene::util::IOUtils;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;

Lucene60PointsReader::Lucene60PointsReader(
    shared_ptr<SegmentReadState> readState) 
    : dataIn(readState->directory->openInput(dataFileName, readState->context)),
      readState(readState)
{

  wstring indexFileName = IndexFileNames::segmentFileName(
      readState->segmentInfo->name, readState->segmentSuffix,
      Lucene60PointsFormat::INDEX_EXTENSION);

  unordered_map<int, int64_t> fieldToFileOffset =
      unordered_map<int, int64_t>();

  // Read index file
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // indexIn = readState.directory.openChecksumInput(indexFileName,
  // readState.context))
  {
    org::apache::lucene::store::ChecksumIndexInput indexIn =
        readState->directory->openChecksumInput(indexFileName,
                                                readState->context);
    runtime_error priorE = nullptr;
    try {
      CodecUtil::checkIndexHeader(
          indexIn, Lucene60PointsFormat::META_CODEC_NAME,
          Lucene60PointsFormat::INDEX_VERSION_START,
          Lucene60PointsFormat::INDEX_VERSION_START,
          readState->segmentInfo->getId(), readState->segmentSuffix);
      int count = indexIn->readVInt();
      for (int i = 0; i < count; i++) {
        int fieldNumber = indexIn->readVInt();
        int64_t fp = indexIn->readVLong();
        fieldToFileOffset.emplace(fieldNumber, fp);
      }
    } catch (const runtime_error &t) {
      priorE = t;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(indexIn, priorE);
    }
  }

  wstring dataFileName = IndexFileNames::segmentFileName(
      readState->segmentInfo->name, readState->segmentSuffix,
      Lucene60PointsFormat::DATA_EXTENSION);
  bool success = false;
  try {

    CodecUtil::checkIndexHeader(dataIn, Lucene60PointsFormat::DATA_CODEC_NAME,
                                Lucene60PointsFormat::DATA_VERSION_START,
                                Lucene60PointsFormat::DATA_VERSION_START,
                                readState->segmentInfo->getId(),
                                readState->segmentSuffix);

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(dataIn);

    for (auto ent : fieldToFileOffset) {
      int fieldNumber = ent.first;
      int64_t fp = ent.second;
      dataIn->seek(fp);
      shared_ptr<BKDReader> reader = make_shared<BKDReader>(dataIn);
      readers.emplace(fieldNumber, reader);
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

shared_ptr<PointValues>
Lucene60PointsReader::getValues(const wstring &fieldName)
{
  shared_ptr<FieldInfo> fieldInfo = readState->fieldInfos->fieldInfo(fieldName);
  if (fieldInfo == nullptr) {
    throw invalid_argument(L"field=\"" + fieldName + L"\" is unrecognized");
  }
  if (fieldInfo->getPointDimensionCount() == 0) {
    throw invalid_argument(L"field=\"" + fieldName +
                           L"\" did not index point values");
  }

  return readers[fieldInfo->number];
}

int64_t Lucene60PointsReader::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto reader : readers) {
    sizeInBytes += reader->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
Lucene60PointsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  for (auto ent : readers) {
    resources.push_back(Accountables::namedAccountable(
        readState->fieldInfos->fieldInfo(ent.first)->name, ent.second));
  }
  return Collections::unmodifiableList(resources);
}

void Lucene60PointsReader::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(dataIn);
}

Lucene60PointsReader::~Lucene60PointsReader()
{
  delete dataIn;
  // Free up heap:
  readers.clear();
}
} // namespace org::apache::lucene::codecs::lucene60