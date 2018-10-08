using namespace std;

#include "CompressingStoredFieldsFormat.h"
#include "../../index/FieldInfos.h"
#include "../../index/SegmentInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../StoredFieldsReader.h"
#include "../StoredFieldsWriter.h"
#include "CompressingStoredFieldsReader.h"
#include "CompressingStoredFieldsWriter.h"
#include "CompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CompressingStoredFieldsFormat::CompressingStoredFieldsFormat(
    const wstring &formatName, shared_ptr<CompressionMode> compressionMode,
    int chunkSize, int maxDocsPerChunk, int blockSize)
    : CompressingStoredFieldsFormat(formatName, L"", compressionMode, chunkSize,
                                    maxDocsPerChunk, blockSize)
{
}

CompressingStoredFieldsFormat::CompressingStoredFieldsFormat(
    const wstring &formatName, const wstring &segmentSuffix,
    shared_ptr<CompressionMode> compressionMode, int chunkSize,
    int maxDocsPerChunk, int blockSize)
    : formatName(formatName), segmentSuffix(segmentSuffix),
      compressionMode(compressionMode), chunkSize(chunkSize),
      maxDocsPerChunk(maxDocsPerChunk), blockSize(blockSize)
{
  if (chunkSize < 1) {
    throw invalid_argument(L"chunkSize must be >= 1");
  }
  if (maxDocsPerChunk < 1) {
    throw invalid_argument(L"maxDocsPerChunk must be >= 1");
  }
  if (blockSize < 1) {
    throw invalid_argument(L"blockSize must be >= 1");
  }
}

shared_ptr<StoredFieldsReader> CompressingStoredFieldsFormat::fieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
{
  return make_shared<CompressingStoredFieldsReader>(
      directory, si, segmentSuffix, fn, context, formatName, compressionMode);
}

shared_ptr<StoredFieldsWriter> CompressingStoredFieldsFormat::fieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  return make_shared<CompressingStoredFieldsWriter>(
      directory, si, segmentSuffix, context, formatName, compressionMode,
      chunkSize, maxDocsPerChunk, blockSize);
}

wstring CompressingStoredFieldsFormat::toString()
{
  return getClass().getSimpleName() + L"(compressionMode=" + compressionMode +
         L", chunkSize=" + to_wstring(chunkSize) + L", maxDocsPerChunk=" +
         to_wstring(maxDocsPerChunk) + L", blockSize=" + to_wstring(blockSize) +
         L")";
}
} // namespace org::apache::lucene::codecs::compressing