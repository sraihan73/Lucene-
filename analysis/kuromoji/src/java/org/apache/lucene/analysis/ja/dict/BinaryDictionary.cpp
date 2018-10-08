using namespace std;

#include "BinaryDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"

namespace org::apache::lucene::analysis::ja::dict
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataInput = org::apache::lucene::store::DataInput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IntsRef = org::apache::lucene::util::IntsRef;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring BinaryDictionary::DICT_FILENAME_SUFFIX = L"$buffer.dat";
const wstring BinaryDictionary::TARGETMAP_FILENAME_SUFFIX = L"$targetMap.dat";
const wstring BinaryDictionary::POSDICT_FILENAME_SUFFIX = L"$posDict.dat";
const wstring BinaryDictionary::DICT_HEADER = L"kuromoji_dict";
const wstring BinaryDictionary::TARGETMAP_HEADER = L"kuromoji_dict_map";
const wstring BinaryDictionary::POSDICT_HEADER = L"kuromoji_dict_pos";

BinaryDictionary::BinaryDictionary() 
{
  shared_ptr<InputStream> mapIS = nullptr, dictIS = nullptr, posIS = nullptr;
  std::deque<int> targetMapOffsets, targetMap;
  std::deque<wstring> posDict;
  std::deque<wstring> inflFormDict;
  std::deque<wstring> inflTypeDict;
  shared_ptr<ByteBuffer> buffer = nullptr;
  bool success = false;
  try {
    mapIS = getResource(TARGETMAP_FILENAME_SUFFIX);
    mapIS = make_shared<BufferedInputStream>(mapIS);
    shared_ptr<DataInput> in_ = make_shared<InputStreamDataInput>(mapIS);
    CodecUtil::checkHeader(in_, TARGETMAP_HEADER, VERSION, VERSION);
    targetMap = std::deque<int>(in_->readVInt());
    targetMapOffsets = std::deque<int>(in_->readVInt());
    int accum = 0, sourceId = 0;
    for (int ofs = 0; ofs < targetMap.size(); ofs++) {
      constexpr int val = in_->readVInt();
      if ((val & 0x01) != 0) {
        targetMapOffsets[sourceId] = ofs;
        sourceId++;
      }
      accum += static_cast<int>(static_cast<unsigned int>(val) >> 1);
      targetMap[ofs] = accum;
    }
    if (sourceId + 1 != targetMapOffsets.size()) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"targetMap file format broken");
    }
    targetMapOffsets[sourceId] = targetMap.size();
    mapIS->close();
    mapIS.reset();

    posIS = getResource(POSDICT_FILENAME_SUFFIX);
    posIS = make_shared<BufferedInputStream>(posIS);
    in_ = make_shared<InputStreamDataInput>(posIS);
    CodecUtil::checkHeader(in_, POSDICT_HEADER, VERSION, VERSION);
    int posSize = in_->readVInt();
    posDict = std::deque<wstring>(posSize);
    inflTypeDict = std::deque<wstring>(posSize);
    inflFormDict = std::deque<wstring>(posSize);
    for (int j = 0; j < posSize; j++) {
      posDict[j] = in_->readString();
      inflTypeDict[j] = in_->readString();
      inflFormDict[j] = in_->readString();
      // this is how we encode null inflections
      if (inflTypeDict[j].length() == 0) {
        inflTypeDict[j] = L"";
      }
      if (inflFormDict[j].length() == 0) {
        inflFormDict[j] = L"";
      }
    }
    posIS->close();
    posIS.reset();

    dictIS = getResource(DICT_FILENAME_SUFFIX);
    // no buffering here, as we load in one large buffer
    in_ = make_shared<InputStreamDataInput>(dictIS);
    CodecUtil::checkHeader(in_, DICT_HEADER, VERSION, VERSION);
    constexpr int size = in_->readVInt();
    shared_ptr<ByteBuffer> *const tmpBuffer = ByteBuffer::allocateDirect(size);
    shared_ptr<ReadableByteChannel> *const channel =
        Channels::newChannel(dictIS);
    constexpr int read = channel->read(tmpBuffer);
    if (read != size) {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"Cannot read whole dictionary");
    }
    dictIS->close();
    dictIS.reset();
    buffer = tmpBuffer->asReadOnlyBuffer();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({mapIS, posIS, dictIS});
    } else {
      IOUtils::closeWhileHandlingException({mapIS, posIS, dictIS});
    }
  }

  this->targetMap = targetMap;
  this->targetMapOffsets = targetMapOffsets;
  this->posDict = posDict;
  this->inflTypeDict = inflTypeDict;
  this->inflFormDict = inflFormDict;
  this->buffer = buffer;
}

shared_ptr<InputStream>
BinaryDictionary::getResource(const wstring &suffix) 
{
  return getClassResource(getClass(), suffix);
}

shared_ptr<InputStream>
BinaryDictionary::getClassResource(type_info clazz,
                                   const wstring &suffix) 
{
  shared_ptr<InputStream> *const is =
      clazz.getResourceAsStream(clazz.name() + suffix);
  if (is == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(
        L"Not in classpath: " + clazz.getName().replace(L'.', L'/') + suffix);
  }
  return is;
}

void BinaryDictionary::lookupWordIds(int sourceId, shared_ptr<IntsRef> ref)
{
  ref->ints = targetMap;
  ref->offset = targetMapOffsets[sourceId];
  // targetMapOffsets always has one more entry pointing behind last:
  ref->length = targetMapOffsets[sourceId + 1] - ref->offset;
}

int BinaryDictionary::getLeftId(int wordId)
{
  return static_cast<int>(static_cast<unsigned int>(buffer->getShort(wordId)) >>
                          3);
}

int BinaryDictionary::getRightId(int wordId)
{
  return static_cast<int>(static_cast<unsigned int>(buffer->getShort(wordId)) >>
                          3);
}

int BinaryDictionary::getWordCost(int wordId)
{
  return buffer->getShort(wordId + 2); // Skip id
}

wstring BinaryDictionary::getBaseForm(int wordId,
                                      std::deque<wchar_t> &surfaceForm,
                                      int off, int len)
{
  if (hasBaseFormData(wordId)) {
    int offset = baseFormOffset(wordId);
    int data = buffer->get(offset++) & 0xff;
    int prefix = static_cast<int>(static_cast<unsigned int>(data) >> 4);
    int suffix = data & 0xF;
    std::deque<wchar_t> text(prefix + suffix);
    System::arraycopy(surfaceForm, off, text, 0, prefix);
    for (int i = 0; i < suffix; i++) {
      text[prefix + i] = buffer->getChar(offset + (i << 1));
    }
    return wstring(text);
  } else {
    return L"";
  }
}

wstring BinaryDictionary::getReading(int wordId, std::deque<wchar_t> &surface,
                                     int off, int len)
{
  if (hasReadingData(wordId)) {
    int offset = readingOffset(wordId);
    int readingData = buffer->get(offset++) & 0xff;
    return readString(
        offset, static_cast<int>(static_cast<unsigned int>(readingData) >> 1),
        (readingData & 1) == 1);
  } else {
    // the reading is the surface form, with hiragana shifted to katakana
    std::deque<wchar_t> text(len);
    for (int i = 0; i < len; i++) {
      wchar_t ch = surface[off + i];
      if (ch > 0x3040 && ch < 0x3097) {
        text[i] = static_cast<wchar_t>(ch + 0x60);
      } else {
        text[i] = ch;
      }
    }
    return wstring(text);
  }
}

wstring BinaryDictionary::getPartOfSpeech(int wordId)
{
  return posDict[getLeftId(wordId)];
}

wstring BinaryDictionary::getPronunciation(int wordId,
                                           std::deque<wchar_t> &surface,
                                           int off, int len)
{
  if (hasPronunciationData(wordId)) {
    int offset = pronunciationOffset(wordId);
    int pronunciationData = buffer->get(offset++) & 0xff;
    return readString(
        offset,
        static_cast<int>(static_cast<unsigned int>(pronunciationData) >> 1),
        (pronunciationData & 1) == 1);
  } else {
    return getReading(wordId, surface, off, len); // same as the reading
  }
}

wstring BinaryDictionary::getInflectionType(int wordId)
{
  return inflTypeDict[getLeftId(wordId)];
}

wstring BinaryDictionary::getInflectionForm(int wordId)
{
  return inflFormDict[getLeftId(wordId)];
}

int BinaryDictionary::baseFormOffset(int wordId) { return wordId + 4; }

int BinaryDictionary::readingOffset(int wordId)
{
  int offset = baseFormOffset(wordId);
  if (hasBaseFormData(wordId)) {
    int baseFormLength = buffer->get(offset++) & 0xf;
    return offset + (baseFormLength << 1);
  } else {
    return offset;
  }
}

int BinaryDictionary::pronunciationOffset(int wordId)
{
  if (hasReadingData(wordId)) {
    int offset = readingOffset(wordId);
    int readingData = buffer->get(offset++) & 0xff;
    constexpr int readingLength;
    if ((readingData & 1) == 0) {
      readingLength = readingData & 0xfe; // UTF-16: mask off kana bit
    } else {
      readingLength =
          static_cast<int>(static_cast<unsigned int>(readingData) >> 1);
    }
    return offset + readingLength;
  } else {
    return readingOffset(wordId);
  }
}

bool BinaryDictionary::hasBaseFormData(int wordId)
{
  return (buffer->getShort(wordId) & HAS_BASEFORM) != 0;
}

bool BinaryDictionary::hasReadingData(int wordId)
{
  return (buffer->getShort(wordId) & HAS_READING) != 0;
}

bool BinaryDictionary::hasPronunciationData(int wordId)
{
  return (buffer->getShort(wordId) & HAS_PRONUNCIATION) != 0;
}

wstring BinaryDictionary::readString(int offset, int length, bool kana)
{
  std::deque<wchar_t> text(length);
  if (kana) {
    for (int i = 0; i < length; i++) {
      text[i] = static_cast<wchar_t>(0x30A0 + (buffer->get(offset + i) & 0xff));
    }
  } else {
    for (int i = 0; i < length; i++) {
      text[i] = buffer->getChar(offset + (i << 1));
    }
  }
  return wstring(text);
}
} // namespace org::apache::lucene::analysis::ja::dict