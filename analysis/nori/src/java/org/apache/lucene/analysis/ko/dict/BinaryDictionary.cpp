using namespace std;

#include "BinaryDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"

namespace org::apache::lucene::analysis::ko::dict
{
using POS = org::apache::lucene::analysis::ko::POS;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataInput = org::apache::lucene::store::DataInput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
const wstring BinaryDictionary::TARGETMAP_FILENAME_SUFFIX = L"$targetMap.dat";
const wstring BinaryDictionary::DICT_FILENAME_SUFFIX = L"$buffer.dat";
const wstring BinaryDictionary::POSDICT_FILENAME_SUFFIX = L"$posDict.dat";
const wstring BinaryDictionary::DICT_HEADER = L"ko_dict";
const wstring BinaryDictionary::TARGETMAP_HEADER = L"ko_dict_map";
const wstring BinaryDictionary::POSDICT_HEADER = L"ko_dict_pos";

BinaryDictionary::BinaryDictionary() 
{
  shared_ptr<InputStream> mapIS = nullptr, dictIS = nullptr, posIS = nullptr;
  std::deque<int> targetMapOffsets, targetMap;
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
    posDict = std::deque<POS::Tag>(posSize);
    for (int j = 0; j < posSize; j++) {
      posDict[j] = POS::resolveTag(in_->readByte());
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
      IOUtils::close({mapIS, dictIS});
    } else {
      IOUtils::closeWhileHandlingException({mapIS, dictIS});
    }
  }

  this->targetMap = targetMap;
  this->targetMapOffsets = targetMapOffsets;
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
                          2);
}

int BinaryDictionary::getRightId(int wordId)
{
  return static_cast<int>(
      static_cast<unsigned int>(buffer->getShort(wordId + 2)) >>
      2); // Skip left id
}

int BinaryDictionary::getWordCost(int wordId)
{
  return buffer->getShort(wordId + 4); // Skip left and right id
}

POS::Type BinaryDictionary::getPOSType(int wordId)
{
  char value = static_cast<char>(buffer->getShort(wordId) & 3);
  return POS::resolveType(value);
}

POS::Tag BinaryDictionary::getLeftPOS(int wordId)
{
  return posDict[getLeftId(wordId)];
}

POS::Tag BinaryDictionary::getRightPOS(int wordId)
{
  POS::Type type = getPOSType(wordId);
  if (type == POS::Type::MORPHEME || type == POS::Type::COMPOUND ||
      hasSinglePOS(wordId)) {
    return getLeftPOS(wordId);
  } else {
    char value = buffer->get(wordId + 6);
    return POS::resolveTag(value);
  }
}

wstring BinaryDictionary::getReading(int wordId)
{
  if (hasReadingData(wordId)) {
    int offset = wordId + 6;
    return readString(offset);
  }
  return L"";
}

std::deque<std::shared_ptr<Morpheme>>
BinaryDictionary::getMorphemes(int wordId, std::deque<wchar_t> &surfaceForm,
                               int off, int len)
{
  POS::Type posType = getPOSType(wordId);
  if (posType == POS::Type::MORPHEME) {
    return nullptr;
  }
  int offset = wordId + 6;
  bool hasSinglePos = hasSinglePOS(wordId);
  if (hasSinglePos == false) {
    offset++; // skip rightPOS
  }
  int length = buffer->get(offset++);
  if (length == 0) {
    return nullptr;
  }
  std::deque<std::shared_ptr<Morpheme>> morphemes(length);
  int surfaceOffset = 0;
  constexpr POS::Tag leftPOS = getLeftPOS(wordId);
  for (int i = 0; i < length; i++) {
    const wstring form;
    constexpr POS::Tag tag =
        hasSinglePos ? leftPOS : POS::resolveTag(buffer->get(offset++));
    if (posType == POS::Type::INFLECT) {
      form = readString(offset);
      offset += form.length() * 2 + 1;
    } else {
      int formLen = buffer->get(offset++);
      form = wstring(surfaceForm, off + surfaceOffset, formLen);
      surfaceOffset += formLen;
    }
    morphemes[i] = make_shared<Morpheme>(tag, form);
  }
  return morphemes;
}

wstring BinaryDictionary::readString(int offset)
{
  int strOffset = offset;
  int len = buffer->get(strOffset++);
  std::deque<wchar_t> text(len);
  for (int i = 0; i < len; i++) {
    text[i] = buffer->getChar(strOffset + (i << 1));
  }
  return wstring(text);
}

bool BinaryDictionary::hasSinglePOS(int wordId)
{
  return (buffer->getShort(wordId + 2) & HAS_SINGLE_POS) != 0;
}

bool BinaryDictionary::hasReadingData(int wordId)
{
  return (buffer->getShort(wordId + 2) & HAS_READING) != 0;
}
} // namespace org::apache::lucene::analysis::ko::dict