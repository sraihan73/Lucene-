using namespace std;

#include "CompressingTermVectorsFormat.h"
#include "../../index/FieldInfos.h"
#include "../../index/SegmentInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../TermVectorsReader.h"
#include "../TermVectorsWriter.h"
#include "CompressingTermVectorsReader.h"
#include "CompressingTermVectorsWriter.h"
#include "CompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CompressingTermVectorsFormat::CompressingTermVectorsFormat(
    const wstring &formatName, const wstring &segmentSuffix,
    shared_ptr<CompressionMode> compressionMode, int chunkSize, int blockSize)
    : formatName(formatName), segmentSuffix(segmentSuffix),
      compressionMode(compressionMode), chunkSize(chunkSize),
      blockSize(blockSize)
{
  if (chunkSize < 1) {
    throw invalid_argument(L"chunkSize must be >= 1");
  }
  if (blockSize < 1) {
    throw invalid_argument(L"blockSize must be >= 1");
  }
}

shared_ptr<TermVectorsReader> CompressingTermVectorsFormat::vectorsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<IOContext> context) 
{
  return make_shared<CompressingTermVectorsReader>(
      directory, segmentInfo, segmentSuffix, fieldInfos, context, formatName,
      compressionMode);
}

shared_ptr<TermVectorsWriter> CompressingTermVectorsFormat::vectorsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<IOContext> context) 
{
  return make_shared<CompressingTermVectorsWriter>(
      directory, segmentInfo, segmentSuffix, context, formatName,
      compressionMode, chunkSize, blockSize);
}

wstring CompressingTermVectorsFormat::toString()
{
  return getClass().getSimpleName() + L"(compressionMode=" + compressionMode +
         L", chunkSize=" + to_wstring(chunkSize) + L", blockSize=" +
         to_wstring(blockSize) + L")";
}
} // namespace org::apache::lucene::codecs::compressing