using namespace std;

#include "SimpleTextFieldInfosFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
const wstring SimpleTextFieldInfosFormat::FIELD_INFOS_EXTENSION = L"inf";
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::NUMFIELDS =
        make_shared<org::apache::lucene::util::BytesRef>(L"number of fields ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::NAME =
        make_shared<org::apache::lucene::util::BytesRef>(L"  name ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::NUMBER =
        make_shared<org::apache::lucene::util::BytesRef>(L"  number ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::STORETV =
        make_shared<org::apache::lucene::util::BytesRef>(L"  term vectors ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::STORETVPOS =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"  term deque positions ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::STORETVOFF =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"  term deque offsets ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::PAYLOADS =
        make_shared<org::apache::lucene::util::BytesRef>(L"  payloads ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::NORMS =
        make_shared<org::apache::lucene::util::BytesRef>(L"  norms ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::DOCVALUES =
        make_shared<org::apache::lucene::util::BytesRef>(L"  doc values ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::DOCVALUES_GEN =
        make_shared<org::apache::lucene::util::BytesRef>(L"  doc values gen ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::INDEXOPTIONS =
        make_shared<org::apache::lucene::util::BytesRef>(L"  index options ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::NUM_ATTS =
        make_shared<org::apache::lucene::util::BytesRef>(L"  attributes ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::ATT_KEY =
        make_shared<org::apache::lucene::util::BytesRef>(L"    key ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::ATT_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"    value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::DIM_COUNT =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"  dimensional count ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::DIM_NUM_BYTES =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"  dimensional num bytes ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldInfosFormat::SOFT_DELETES =
        make_shared<org::apache::lucene::util::BytesRef>(L"  soft-deletes ");

shared_ptr<FieldInfos> SimpleTextFieldInfosFormat::read(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    const wstring &segmentSuffix,
    shared_ptr<IOContext> iocontext) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segmentInfo->name, segmentSuffix, FIELD_INFOS_EXTENSION);
  shared_ptr<ChecksumIndexInput> input =
      directory->openChecksumInput(fileName, iocontext);
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  bool success = false;
  try {

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), NUMFIELDS)));
    constexpr int size = stoi(readString(NUMFIELDS->length, scratch));
    std::deque<std::shared_ptr<FieldInfo>> infos(size);

    for (int i = 0; i < size; i++) {
      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), NAME)));
      wstring name = readString(NAME->length, scratch);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), NUMBER)));
      int fieldNumber = stoi(readString(NUMBER->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), INDEXOPTIONS)));
      wstring s = readString(INDEXOPTIONS->length, scratch);
      constexpr IndexOptions indexOptions = IndexOptions::valueOf(s);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), STORETV)));
      bool storeTermVector =
          StringHelper::fromString<bool>(readString(STORETV->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), PAYLOADS)));
      bool storePayloads =
          StringHelper::fromString<bool>(readString(PAYLOADS->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), NORMS)));
      bool omitNorms =
          !StringHelper::fromString<bool>(readString(NORMS->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), DOCVALUES)));
      wstring dvType = readString(DOCVALUES->length, scratch);
      constexpr DocValuesType docValuesType = this->docValuesType(dvType);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), DOCVALUES_GEN)));
      constexpr int64_t dvGen = StringHelper::fromString<int64_t>(
          readString(DOCVALUES_GEN->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), NUM_ATTS)));
      int numAtts = stoi(readString(NUM_ATTS->length, scratch));
      unordered_map<wstring, wstring> atts = unordered_map<wstring, wstring>();

      for (int j = 0; j < numAtts; j++) {
        SimpleTextUtil::readLine(input, scratch);
        assert((StringHelper::startsWith(scratch->get(), ATT_KEY)));
        wstring key = readString(ATT_KEY->length, scratch);

        SimpleTextUtil::readLine(input, scratch);
        assert((StringHelper::startsWith(scratch->get(), ATT_VALUE)));
        wstring value = readString(ATT_VALUE->length, scratch);
        atts.emplace(key, value);
      }

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), DIM_COUNT)));
      int dimensionalCount = stoi(readString(DIM_COUNT->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), DIM_NUM_BYTES)));
      int dimensionalNumBytes =
          stoi(readString(DIM_NUM_BYTES->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SOFT_DELETES)));
      bool isSoftDeletesField = StringHelper::fromString<bool>(
          readString(SOFT_DELETES->length, scratch));

      infos[i] = make_shared<FieldInfo>(
          name, fieldNumber, storeTermVector, omitNorms, storePayloads,
          indexOptions, docValuesType, dvGen,
          atts, dimensionalCount,
          dimensionalNumBytes, isSoftDeletesField);
    }

    SimpleTextUtil::checkFooter(input);

    shared_ptr<FieldInfos> fieldInfos = make_shared<FieldInfos>(infos);
    success = true;
    return fieldInfos;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      delete input;
    } else {
      IOUtils::closeWhileHandlingException({input});
    }
  }
}

DocValuesType SimpleTextFieldInfosFormat::docValuesType(const wstring &dvType)
{
  return DocValuesType::valueOf(dvType);
}

wstring
SimpleTextFieldInfosFormat::readString(int offset,
                                       shared_ptr<BytesRefBuilder> scratch)
{
  return wstring(scratch->bytes(), offset, scratch->length() - offset,
                 StandardCharsets::UTF_8);
}

void SimpleTextFieldInfosFormat::write(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    const wstring &segmentSuffix, shared_ptr<FieldInfos> infos,
    shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segmentInfo->name, segmentSuffix, FIELD_INFOS_EXTENSION);
  shared_ptr<IndexOutput> out = directory->createOutput(fileName, context);
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  bool success = false;
  try {
    SimpleTextUtil::write(out, NUMFIELDS);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(out, Integer::toString(infos->size()), scratch);
    SimpleTextUtil::writeNewline(out);

    for (auto fi : infos) {
      SimpleTextUtil::write(out, NAME);
      SimpleTextUtil::write(out, fi->name, scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, NUMBER);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Integer::toString(fi->number), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, INDEXOPTIONS);
      IndexOptions indexOptions = fi->getIndexOptions();
      assert(indexOptions.compareTo(
                 IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0 ||
             !fi->hasPayloads());
      SimpleTextUtil::write(out, indexOptions.toString(), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, STORETV);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Boolean::toString(fi->hasVectors()), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, PAYLOADS);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Boolean::toString(fi->hasPayloads()), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, NORMS);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Boolean::toString(!fi->omitsNorms()), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, DOCVALUES);
      SimpleTextUtil::write(out, getDocValuesType(fi->getDocValuesType()),
                            scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, DOCVALUES_GEN);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Long::toString(fi->getDocValuesGen()),
                            scratch);
      SimpleTextUtil::writeNewline(out);

      unordered_map<wstring, wstring> atts = fi->attributes();
      int numAtts = atts.empty() ? 0 : atts.size();
      SimpleTextUtil::write(out, NUM_ATTS);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Integer::toString(numAtts), scratch);
      SimpleTextUtil::writeNewline(out);

      if (numAtts > 0) {
        for (auto entry : atts) {
          SimpleTextUtil::write(out, ATT_KEY);
          SimpleTextUtil::write(out, entry.first, scratch);
          SimpleTextUtil::writeNewline(out);

          SimpleTextUtil::write(out, ATT_VALUE);
          SimpleTextUtil::write(out, entry.second, scratch);
          SimpleTextUtil::writeNewline(out);
        }
      }

      SimpleTextUtil::write(out, DIM_COUNT);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(
          out, Integer::toString(fi->getPointDimensionCount()), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, DIM_NUM_BYTES);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Integer::toString(fi->getPointNumBytes()),
                            scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, SOFT_DELETES);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Boolean::toString(fi->isSoftDeletesField()),
                            scratch);
      SimpleTextUtil::writeNewline(out);
    }
    SimpleTextUtil::writeChecksum(out, scratch);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      delete out;
    } else {
      IOUtils::closeWhileHandlingException({out});
    }
  }
}

wstring SimpleTextFieldInfosFormat::getDocValuesType(DocValuesType type)
{
  return type.toString();
}
} // namespace org::apache::lucene::codecs::simpletext