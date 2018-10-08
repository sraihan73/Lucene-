using namespace std;

#include "Lucene50FieldInfosFormat.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../CodecUtil.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

Lucene50FieldInfosFormat::Lucene50FieldInfosFormat() {}

shared_ptr<FieldInfos>
Lucene50FieldInfosFormat::read(shared_ptr<Directory> directory,
                               shared_ptr<SegmentInfo> segmentInfo,
                               const wstring &segmentSuffix,
                               shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segmentInfo->name, segmentSuffix, EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = directory.openChecksumInput(fileName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        directory->openChecksumInput(fileName, context);
    runtime_error priorE = nullptr;
    std::deque<std::shared_ptr<FieldInfo>> infos;
    try {
      CodecUtil::checkIndexHeader(input, Lucene50FieldInfosFormat::CODEC_NAME,
                                  Lucene50FieldInfosFormat::FORMAT_START,
                                  Lucene50FieldInfosFormat::FORMAT_CURRENT,
                                  segmentInfo->getId(), segmentSuffix);

      constexpr int size = input->readVInt(); // read in the size
      infos = std::deque<std::shared_ptr<FieldInfo>>(size);

      // previous field's attribute map_obj, we share when possible:
      unordered_map<wstring, wstring> lastAttributes = Collections::emptyMap();

      for (int i = 0; i < size; i++) {
        wstring name = input->readString();
        constexpr int fieldNumber = input->readVInt();
        if (fieldNumber < 0) {
          throw make_shared<CorruptIndexException>(
              L"invalid field number for field: " + name + L", fieldNumber=" +
                  to_wstring(fieldNumber),
              input);
        }
        char bits = input->readByte();
        bool storeTermVector = (bits & STORE_TERMVECTOR) != 0;
        bool omitNorms = (bits & OMIT_NORMS) != 0;
        bool storePayloads = (bits & STORE_PAYLOADS) != 0;

        constexpr IndexOptions indexOptions =
            getIndexOptions(input, input->readByte());

        // DV Types are packed in one byte
        constexpr DocValuesType docValuesType =
            getDocValuesType(input, input->readByte());
        constexpr int64_t dvGen = input->readLong();
        unordered_map<wstring, wstring> attributes = input->readMapOfStrings();

        // just use the last field's map_obj if its the same
        if (attributes.equals(lastAttributes)) {
          attributes = lastAttributes;
        }
        lastAttributes = attributes;
        try {
          infos[i] = make_shared<FieldInfo>(
              name, fieldNumber, storeTermVector, omitNorms, storePayloads,
              indexOptions, docValuesType, dvGen, attributes, 0, 0, false);
          infos[i]->checkConsistency();
        } catch (const IllegalStateException &e) {
          throw make_shared<CorruptIndexException>(
              L"invalid fieldinfo for field: " + name + L", fieldNumber=" +
                  to_wstring(fieldNumber),
              input, e);
        }
      }
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(input, priorE);
    }
    return make_shared<FieldInfos>(infos);
  }
}

Lucene50FieldInfosFormat::StaticConstructor::StaticConstructor()
{
  // We "mirror" DocValues enum values with the constants below; let's try to
  // ensure if we add a new DocValuesType while this format is still used for
  // writing, we remember to fix this encoding:
  assert(DocValuesType::values()->length == 6);
  // We "mirror" IndexOptions enum values with the constants below; let's try to
  // ensure if we add a new IndexOption while this format is still used for
  // writing, we remember to fix this encoding:
  assert(IndexOptions::values()->length == 5);
}

Lucene50FieldInfosFormat::StaticConstructor
    Lucene50FieldInfosFormat::staticConstructor;

char Lucene50FieldInfosFormat::docValuesByte(DocValuesType type)
{
  switch (type) {
  case DocValuesType::NONE:
    return 0;
  case DocValuesType::NUMERIC:
    return 1;
  case DocValuesType::BINARY:
    return 2;
  case DocValuesType::SORTED:
    return 3;
  case DocValuesType::SORTED_SET:
    return 4;
  case DocValuesType::SORTED_NUMERIC:
    return 5;
  default:
    // BUG
    throw make_shared<AssertionError>(L"unhandled DocValuesType: " + type);
  }
}

DocValuesType
Lucene50FieldInfosFormat::getDocValuesType(shared_ptr<IndexInput> input,
                                           char b) 
{
  switch (b) {
  case 0:
    return DocValuesType::NONE;
  case 1:
    return DocValuesType::NUMERIC;
  case 2:
    return DocValuesType::BINARY;
  case 3:
    return DocValuesType::SORTED;
  case 4:
    return DocValuesType::SORTED_SET;
  case 5:
    return DocValuesType::SORTED_NUMERIC;
  default:
    throw make_shared<CorruptIndexException>(
        L"invalid docvalues byte: " + to_wstring(b), input);
  }
}

char Lucene50FieldInfosFormat::indexOptionsByte(IndexOptions indexOptions)
{
  switch (indexOptions) {
  case IndexOptions::NONE:
    return 0;
  case IndexOptions::DOCS:
    return 1;
  case IndexOptions::DOCS_AND_FREQS:
    return 2;
  case IndexOptions::DOCS_AND_FREQS_AND_POSITIONS:
    return 3;
  case IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS:
    return 4;
  default:
    // BUG:
    throw make_shared<AssertionError>(L"unhandled IndexOptions: " +
                                      indexOptions);
  }
}

IndexOptions
Lucene50FieldInfosFormat::getIndexOptions(shared_ptr<IndexInput> input,
                                          char b) 
{
  switch (b) {
  case 0:
    return IndexOptions::NONE;
  case 1:
    return IndexOptions::DOCS;
  case 2:
    return IndexOptions::DOCS_AND_FREQS;
  case 3:
    return IndexOptions::DOCS_AND_FREQS_AND_POSITIONS;
  case 4:
    return IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS;
  default:
    // BUG
    throw make_shared<CorruptIndexException>(
        L"invalid IndexOptions byte: " + to_wstring(b), input);
  }
}

void Lucene50FieldInfosFormat::write(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    const wstring &segmentSuffix, shared_ptr<FieldInfos> infos,
    shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segmentInfo->name, segmentSuffix, EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // directory.createOutput(fileName, context))
  {
    org::apache::lucene::store::IndexOutput output =
        directory->createOutput(fileName, context);
    CodecUtil::writeIndexHeader(output, Lucene50FieldInfosFormat::CODEC_NAME,
                                Lucene50FieldInfosFormat::FORMAT_CURRENT,
                                segmentInfo->getId(), segmentSuffix);
    output->writeVInt(infos->size());
    for (auto fi : infos) {
      fi->checkConsistency();

      output->writeString(fi->name);
      output->writeVInt(fi->number);

      char bits = 0x0;
      if (fi->hasVectors()) {
        bits |= STORE_TERMVECTOR;
      }
      if (fi->omitsNorms()) {
        bits |= OMIT_NORMS;
      }
      if (fi->hasPayloads()) {
        bits |= STORE_PAYLOADS;
      }
      output->writeByte(bits);

      output->writeByte(indexOptionsByte(fi->getIndexOptions()));

      // pack the DV type and hasNorms in one byte
      output->writeByte(docValuesByte(fi->getDocValuesType()));
      output->writeLong(fi->getDocValuesGen());
      output->writeMapOfStrings(fi->attributes());
    }
    CodecUtil::writeFooter(output);
  }
}

const wstring Lucene50FieldInfosFormat::EXTENSION = L"fnm";
const wstring Lucene50FieldInfosFormat::CODEC_NAME = L"Lucene50FieldInfos";
} // namespace org::apache::lucene::codecs::lucene50