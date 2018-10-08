using namespace std;

#include "BinaryDictionaryWriter.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/OutputStreamDataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/dict/BinaryDictionary.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/util/CSVUtil.h"

namespace org::apache::lucene::analysis::ja::util
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataOutput = org::apache::lucene::store::DataOutput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BinaryDictionary =
    org::apache::lucene::analysis::ja::dict::BinaryDictionary;

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

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();

  // build up the POS string
  for (int i = 4; i < 8; i++) {
    wstring part = entry[i];
    assert(part.length() > 0);
    if (L"*" != part) {
      if (sb->length() > 0) {
        sb->append(L'-');
      }
      sb->append(part);
    }
  }

  wstring posData = sb->toString();

  sb->setLength(0);
  sb->append(CSVUtil::quoteEscape(posData));
  sb->append(L',');
  if (L"*" != entry[8]) {
    sb->append(CSVUtil::quoteEscape(entry[8]));
  }
  sb->append(L',');
  if (L"*" != entry[9]) {
    sb->append(CSVUtil::quoteEscape(entry[9]));
  }
  wstring fullPOSData = sb->toString();

  wstring baseForm = entry[10];
  wstring reading = entry[11];
  wstring pronunciation = entry[12];

  // extend buffer if necessary
  int left = buffer->remaining();
  // worst case: two short, 3 bytes, and features (all as utf-16)
  int worstCase =
      4 + 3 +
      2 * (baseForm.length() + reading.length() + pronunciation.length());
  if (worstCase > left) {
    shared_ptr<ByteBuffer> newBuffer = ByteBuffer::allocate(
        ArrayUtil::oversize(buffer->limit() + worstCase - left, 1));
    buffer->flip();
    newBuffer->put(buffer);
    buffer = newBuffer;
  }

  int flags = 0;
  if (!(L"*" == baseForm || baseForm == entry[0])) {
    flags |= BinaryDictionary::HAS_BASEFORM;
  }
  if (reading != toKatakana(entry[0])) {
    flags |= BinaryDictionary::HAS_READING;
  }
  if (pronunciation != reading) {
    flags |= BinaryDictionary::HAS_PRONUNCIATION;
  }

  assert(leftId == rightId);
  assert(leftId < 4096); // there are still unused bits
  // add pos mapping
  int toFill = 1 + leftId - posDict.size();
  for (int i = 0; i < toFill; i++) {
    posDict.push_back(nullptr);
  }

  wstring existing = posDict[leftId];
  assert(existing == L"" || existing == fullPOSData);
  posDict[leftId] = fullPOSData;

  buffer->putShort(static_cast<short>(leftId << 3 | flags));
  buffer->putShort(wordCost);

  if ((flags & BinaryDictionary::HAS_BASEFORM) != 0) {
    assert(baseForm.length() < 16);
    int shared = sharedPrefix(entry[0], baseForm);
    int suffix = baseForm.length() - shared;
    buffer->put(static_cast<char>(shared << 4 | suffix));
    for (int i = shared; i < baseForm.length(); i++) {
      buffer->putChar(baseForm[i]);
    }
  }

  if ((flags & BinaryDictionary::HAS_READING) != 0) {
    if (isKatakana(reading)) {
      buffer->put(static_cast<char>(reading.length() << 1 | 1));
      writeKatakana(reading);
    } else {
      buffer->put(static_cast<char>(reading.length() << 1));
      for (int i = 0; i < reading.length(); i++) {
        buffer->putChar(reading[i]);
      }
    }
  }

  if ((flags & BinaryDictionary::HAS_PRONUNCIATION) != 0) {
    // we can save 150KB here, but it makes the reader a little complicated.
    // int shared = sharedPrefix(reading, pronunciation);
    // buffer.put((byte) shared);
    // pronunciation = pronunciation.substring(shared);
    if (isKatakana(pronunciation)) {
      buffer->put(static_cast<char>(pronunciation.length() << 1 | 1));
      writeKatakana(pronunciation);
    } else {
      buffer->put(static_cast<char>(pronunciation.length() << 1));
      for (int i = 0; i < pronunciation.length(); i++) {
        buffer->putChar(pronunciation[i]);
      }
    }
  }

  return buffer->position();
}

bool BinaryDictionaryWriter::isKatakana(const wstring &s)
{
  for (int i = 0; i < s.length(); i++) {
    wchar_t ch = s[i];
    if (ch < 0x30A0 || ch > 0x30FF) {
      return false;
    }
  }
  return true;
}

void BinaryDictionaryWriter::writeKatakana(const wstring &s)
{
  for (int i = 0; i < s.length(); i++) {
    buffer->put(static_cast<char>(s[i] - 0x30A0));
  }
}

wstring BinaryDictionaryWriter::toKatakana(const wstring &s)
{
  std::deque<wchar_t> text(s.length());
  for (int i = 0; i < s.length(); i++) {
    wchar_t ch = s[i];
    if (ch > 0x3040 && ch < 0x3097) {
      text[i] = static_cast<wchar_t>(ch + 0x60);
    } else {
      text[i] = ch;
    }
  }
  return wstring(text);
}

int BinaryDictionaryWriter::sharedPrefix(const wstring &left,
                                         const wstring &right)
{
  int len = left.length() < right.length() ? left.length() : right.length();
  for (int i = 0; i < len; i++) {
    if (left[i] != right[i]) {
      return i;
    }
  }
  return len;
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
        out->writeByte(static_cast<char>(0));
        out->writeByte(static_cast<char>(0));
        out->writeByte(static_cast<char>(0));
      } else {
        std::deque<wstring> data = CSVUtil::parse(s);
        assert((data.size() == 3, L"malformed pos/inflection: " + s));
        out->writeString(data[0]);
        out->writeString(data[1]);
        out->writeString(data[2]);
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
} // namespace org::apache::lucene::analysis::ja::util