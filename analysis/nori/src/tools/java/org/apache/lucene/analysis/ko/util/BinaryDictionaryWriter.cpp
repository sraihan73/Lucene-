using namespace std;

#include "BinaryDictionaryWriter.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/OutputStreamDataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/POS.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/dict/BinaryDictionary.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/dict/Dictionary.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/util/CSVUtil.h"

namespace org::apache::lucene::analysis::ko::util
{
using POS = org::apache::lucene::analysis::ko::POS;
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataOutput = org::apache::lucene::store::DataOutput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BinaryDictionary =
    org::apache::lucene::analysis::ko::dict::BinaryDictionary;

BinaryDictionaryWriter::BinaryDictionaryWriter(type_info implClazz, int size)
    : implClazz(implClazz)
{
  buffer = ByteBuffer::allocate(size);
}

int BinaryDictionaryWriter::put(std::deque<wstring> &entry)
{
  short leftId = StringHelper::fromString<short>(entry[1]);
  short rightId = StringHelper::fromString<short>(entry[2]);
  short wordCost = StringHelper::fromString<short>(entry[3]);

  constexpr POS::Type posType = POS::resolveType(entry[8]);
  constexpr POS::Tag leftPOS;
  constexpr POS::Tag rightPOS;
  if (posType == POS::Type::MORPHEME || posType == POS::Type::COMPOUND ||
      entry[9] == L"*") {
    leftPOS = POS::resolveTag(entry[4]);
    assert(entry[9] == L"*" && entry[10] == L"*");
    rightPOS = leftPOS;
  } else {
    leftPOS = POS::resolveTag(entry[9]);
    rightPOS = POS::resolveTag(entry[10]);
  }
  const wstring reading =
      entry[7] == L"*" ? L"" : entry[0] == entry[7] ? L"" : entry[7];
  const wstring expression = entry[11] == L"*" ? L"" : entry[11];

  // extend buffer if necessary
  int left = buffer->remaining();
  // worst case, 3 short + 4 bytes and features (all as utf-16)
  int worstCase = 9 + 2 * (expression.length() + reading.length());
  if (worstCase > left) {
    shared_ptr<ByteBuffer> newBuffer = ByteBuffer::allocate(
        ArrayUtil::oversize(buffer->limit() + worstCase - left, 1));
    buffer->flip();
    newBuffer->put(buffer);
    buffer = newBuffer;
  }

  // add pos mapping
  int toFill = 1 + leftId - posDict.size();
  for (int i = 0; i < toFill; i++) {
    posDict.push_back(nullptr);
  }
  wstring fullPOSData = leftPOS.name() + L"," + entry[5];
  wstring existing = posDict[leftId];
  assert(existing == L"" || existing == fullPOSData);
  posDict[leftId] = fullPOSData;

  std::deque<std::shared_ptr<Dictionary::Morpheme>> morphemes;
  // true if the POS and decompounds of the token are all the same.
  bool hasSinglePOS = (leftPOS == rightPOS);
  if (posType != POS::Type::MORPHEME && expression.length() > 0) {
    std::deque<wstring> exprTokens = expression.split(L"\\+");
    morphemes =
        std::deque<std::shared_ptr<Dictionary::Morpheme>>(exprTokens.size());
    for (int i = 0; i < exprTokens.size(); i++) {
      std::deque<wstring> tokenSplit = exprTokens[i].split(L"\\/");
      assert(tokenSplit.size() == 3);
      POS::Tag exprTag = POS::resolveTag(tokenSplit[1]);
      morphemes[i] = make_shared<Dictionary::Morpheme>(exprTag, tokenSplit[0]);
      if (leftPOS != exprTag) {
        hasSinglePOS = false;
      }
    }
  } else {
    morphemes = std::deque<std::shared_ptr<Dictionary::Morpheme>>(0);
  }

  int flags = 0;
  if (hasSinglePOS) {
    flags |= BinaryDictionary::HAS_SINGLE_POS;
  }
  if (posType == POS::Type::MORPHEME && reading.length() > 0) {
    flags |= BinaryDictionary::HAS_READING;
  }

  assert(leftId < 8192); // there are still unused bits
  assert(static_cast<int>(posType) < 4);
  buffer->putShort(static_cast<short>(leftId << 2 | static_cast<int>(posType)));
  buffer->putShort(static_cast<short>(rightId << 2 | flags));
  buffer->putShort(wordCost);

  if (posType == POS::Type::MORPHEME) {
    assert(leftPOS == rightPOS);
    if (reading.length() > 0) {
      writeString(reading);
    }
  } else {
    if (hasSinglePOS == false) {
      buffer->put(static_cast<char>(rightPOS.ordinal()));
    }
    buffer->put(static_cast<char>(morphemes.size()));
    int compoundOffset = 0;
    for (int i = 0; i < morphemes.size(); i++) {
      if (hasSinglePOS == false) {
        buffer->put(static_cast<char>(morphemes[i]->posTag.ordinal()));
      }
      if (posType != POS::Type::INFLECT) {
        buffer->put(static_cast<char>(morphemes[i]->surfaceForm->length()));
        compoundOffset += morphemes[i]->surfaceForm->length();
      } else {
        writeString(morphemes[i]->surfaceForm);
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assert((compoundOffset <= entry[0].length(), Arrays->toString(entry)));
    }
  }
  return buffer->position();
}

void BinaryDictionaryWriter::writeString(const wstring &s)
{
  buffer->put(static_cast<char>(s.length()));
  for (int i = 0; i < s.length(); i++) {
    buffer->putChar(s[i]);
  }
}

void BinaryDictionaryWriter::addMapping(int sourceId, int wordId)
{
  assert((wordId > lastWordId, L"words out of order: " + to_wstring(wordId) +
                                   L" vs lastID: " + to_wstring(lastWordId)));

  if (sourceId > lastSourceId) {
    assert((sourceId > lastSourceId, L"source ids out of order: lastSourceId=" +
                                         to_wstring(lastSourceId) +
                                         L" vs sourceId=" +
                                         to_wstring(sourceId)));
    targetMapOffsets = ArrayUtil::grow(targetMapOffsets, sourceId + 1);
    for (int i = lastSourceId + 1; i <= sourceId; i++) {
      targetMapOffsets[i] = targetMapEndOffset;
    }
  } else {
    assert(sourceId == lastSourceId);
  }

  targetMap = ArrayUtil::grow(targetMap, targetMapEndOffset + 1);
  targetMap[targetMapEndOffset] = wordId;
  targetMapEndOffset++;

  lastSourceId = sourceId;
  lastWordId = wordId;
}

wstring BinaryDictionaryWriter::getBaseFileName(const wstring &baseDir)
{
  return baseDir + File::separator +
         implClazz.getName()->replace(L'.', FileSystem::preferredSeparator());
}

void BinaryDictionaryWriter::write(const wstring &baseDir) 
{
  const wstring baseName = getBaseFileName(baseDir);
  writeDictionary(baseName + BinaryDictionary::DICT_FILENAME_SUFFIX);
  writeTargetMap(baseName + BinaryDictionary::TARGETMAP_FILENAME_SUFFIX);
  writePosDict(baseName + BinaryDictionary::POSDICT_FILENAME_SUFFIX);
}

void BinaryDictionaryWriter::writeTargetMap(const wstring &filename) throw(
    IOException)
{
  (make_shared<File>(filename))->getParentFile().mkdirs();
  shared_ptr<OutputStream> os = make_shared<FileOutputStream>(filename);
  try {
    os = make_shared<BufferedOutputStream>(os);
    shared_ptr<DataOutput> *const out = make_shared<OutputStreamDataOutput>(os);
    CodecUtil::writeHeader(out, BinaryDictionary::TARGETMAP_HEADER,
                           BinaryDictionary::VERSION);

    constexpr int numSourceIds = lastSourceId + 1;
    out->writeVInt(targetMapEndOffset); // <-- size of main array
    out->writeVInt(numSourceIds +
                   1); // <-- size of offset array (+ 1 more entry)
    int prev = 0, sourceId = 0;
    for (int ofs = 0; ofs < targetMapEndOffset; ofs++) {
      constexpr int val = targetMap[ofs], delta = val - prev;
      assert(delta >= 0);
      if (ofs == targetMapOffsets[sourceId]) {
        out->writeVInt((delta << 1) | 0x01);
        sourceId++;
      } else {
        out->writeVInt((delta << 1));
      }
      prev += delta;
    }
    assert((sourceId == numSourceIds, L"sourceId:" + to_wstring(sourceId) +
                                          L" != numSourceIds:" +
                                          to_wstring(numSourceIds)));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    os->close();
  }
}

void BinaryDictionaryWriter::writePosDict(const wstring &filename) throw(
    IOException)
{
  (make_shared<File>(filename))->getParentFile().mkdirs();
  shared_ptr<OutputStream> os = make_shared<FileOutputStream>(filename);
  try {
    os = make_shared<BufferedOutputStream>(os);
    shared_ptr<DataOutput> *const out = make_shared<OutputStreamDataOutput>(os);
    CodecUtil::writeHeader(out, BinaryDictionary::POSDICT_HEADER,
                           BinaryDictionary::VERSION);
    out->writeVInt(posDict.size());
    for (auto s : posDict) {
      if (s == L"") {
        out->writeByte(static_cast<char>(POS::Tag::UNKNOWN.ordinal()));
      } else {
        std::deque<wstring> data = CSVUtil::parse(s);
        assert((data.size() == 2, L"malformed pos/semanticClass: " + s));
        out->writeByte(static_cast<char>(POS::Tag::valueOf(data[0]).ordinal()));
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    os->close();
  }
}

void BinaryDictionaryWriter::writeDictionary(const wstring &filename) throw(
    IOException)
{
  (make_shared<File>(filename))->getParentFile().mkdirs();
  shared_ptr<FileOutputStream> *const os =
      make_shared<FileOutputStream>(filename);
  try {
    shared_ptr<DataOutput> *const out = make_shared<OutputStreamDataOutput>(os);
    CodecUtil::writeHeader(out, BinaryDictionary::DICT_HEADER,
                           BinaryDictionary::VERSION);
    out->writeVInt(buffer->position());
    shared_ptr<WritableByteChannel> *const channel = Channels::newChannel(os);
    // Write Buffer
    buffer->flip(); // set position to 0, set limit to current position
    channel->write(buffer);
    assert(buffer->remaining() == 0LL);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    os->close();
  }
}
} // namespace org::apache::lucene::analysis::ko::util