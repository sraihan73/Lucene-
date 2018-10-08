using namespace std;

#include "Lucene50SkipReader.h"
#include "../../store/IndexInput.h"

namespace org::apache::lucene::codecs::lucene50
{
using MultiLevelSkipListReader =
    org::apache::lucene::codecs::MultiLevelSkipListReader;
using IndexInput = org::apache::lucene::store::IndexInput;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;

Lucene50SkipReader::Lucene50SkipReader(shared_ptr<IndexInput> skipStream,
                                       int maxSkipLevels, bool hasPos,
                                       bool hasOffsets, bool hasPayloads)
    : org::apache::lucene::codecs::MultiLevelSkipListReader(
          skipStream, maxSkipLevels, BLOCK_SIZE, 8)
{
  docPointer = std::deque<int64_t>(maxSkipLevels);
  if (hasPos) {
    posPointer = std::deque<int64_t>(maxSkipLevels);
    posBufferUpto = std::deque<int>(maxSkipLevels);
    if (hasPayloads) {
      payloadByteUpto = std::deque<int>(maxSkipLevels);
    } else {
      payloadByteUpto.clear();
    }
    if (hasOffsets || hasPayloads) {
      payPointer = std::deque<int64_t>(maxSkipLevels);
    } else {
      payPointer.clear();
    }
  } else {
    posPointer.clear();
  }
}

int Lucene50SkipReader::trim(int df)
{
  return df % BLOCK_SIZE == 0 ? df - 1 : df;
}

void Lucene50SkipReader::init(int64_t skipPointer, int64_t docBasePointer,
                              int64_t posBasePointer,
                              int64_t payBasePointer,
                              int df) 
{
  MultiLevelSkipListReader::init(skipPointer, trim(df));
  lastDocPointer = docBasePointer;
  lastPosPointer = posBasePointer;
  lastPayPointer = payBasePointer;

  Arrays::fill(docPointer, docBasePointer);
  if (posPointer.size() > 0) {
    Arrays::fill(posPointer, posBasePointer);
    if (payPointer.size() > 0) {
      Arrays::fill(payPointer, payBasePointer);
    }
  } else {
    assert(posBasePointer == 0);
  }
}

int64_t Lucene50SkipReader::getDocPointer() { return lastDocPointer; }

int64_t Lucene50SkipReader::getPosPointer() { return lastPosPointer; }

int Lucene50SkipReader::getPosBufferUpto() { return lastPosBufferUpto; }

int64_t Lucene50SkipReader::getPayPointer() { return lastPayPointer; }

int Lucene50SkipReader::getPayloadByteUpto() { return lastPayloadByteUpto; }

int Lucene50SkipReader::getNextSkipDoc() { return skipDoc[0]; }

void Lucene50SkipReader::seekChild(int level) 
{
  MultiLevelSkipListReader::seekChild(level);
  docPointer[level] = lastDocPointer;
  if (posPointer.size() > 0) {
    posPointer[level] = lastPosPointer;
    posBufferUpto[level] = lastPosBufferUpto;
    if (payloadByteUpto.size() > 0) {
      payloadByteUpto[level] = lastPayloadByteUpto;
    }
    if (payPointer.size() > 0) {
      payPointer[level] = lastPayPointer;
    }
  }
}

void Lucene50SkipReader::setLastSkipData(int level)
{
  MultiLevelSkipListReader::setLastSkipData(level);
  lastDocPointer = docPointer[level];

  if (posPointer.size() > 0) {
    lastPosPointer = posPointer[level];
    lastPosBufferUpto = posBufferUpto[level];
    if (payPointer.size() > 0) {
      lastPayPointer = payPointer[level];
    }
    if (payloadByteUpto.size() > 0) {
      lastPayloadByteUpto = payloadByteUpto[level];
    }
  }
}

int Lucene50SkipReader::readSkipData(
    int level, shared_ptr<IndexInput> skipStream) 
{
  int delta = skipStream->readVInt();
  docPointer[level] += skipStream->readVLong();

  if (posPointer.size() > 0) {
    posPointer[level] += skipStream->readVLong();
    posBufferUpto[level] = skipStream->readVInt();

    if (payloadByteUpto.size() > 0) {
      payloadByteUpto[level] = skipStream->readVInt();
    }

    if (payPointer.size() > 0) {
      payPointer[level] += skipStream->readVLong();
    }
  }
  return delta;
}
} // namespace org::apache::lucene::codecs::lucene50