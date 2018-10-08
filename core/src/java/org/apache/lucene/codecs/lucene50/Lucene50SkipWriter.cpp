using namespace std;

#include "Lucene50SkipWriter.h"
#include "../../store/IndexOutput.h"

namespace org::apache::lucene::codecs::lucene50
{
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MultiLevelSkipListWriter =
    org::apache::lucene::codecs::MultiLevelSkipListWriter;

Lucene50SkipWriter::Lucene50SkipWriter(int maxSkipLevels, int blockSize,
                                       int docCount,
                                       shared_ptr<IndexOutput> docOut,
                                       shared_ptr<IndexOutput> posOut,
                                       shared_ptr<IndexOutput> payOut)
    : org::apache::lucene::codecs::MultiLevelSkipListWriter(
          blockSize, 8, maxSkipLevels, docCount),
      docOut(docOut), posOut(posOut), payOut(payOut)
{

  lastSkipDoc = std::deque<int>(maxSkipLevels);
  lastSkipDocPointer = std::deque<int64_t>(maxSkipLevels);
  if (posOut != nullptr) {
    lastSkipPosPointer = std::deque<int64_t>(maxSkipLevels);
    if (payOut != nullptr) {
      lastSkipPayPointer = std::deque<int64_t>(maxSkipLevels);
    }
    lastPayloadByteUpto = std::deque<int>(maxSkipLevels);
  }
}

void Lucene50SkipWriter::setField(bool fieldHasPositions, bool fieldHasOffsets,
                                  bool fieldHasPayloads)
{
  this->fieldHasPositions = fieldHasPositions;
  this->fieldHasOffsets = fieldHasOffsets;
  this->fieldHasPayloads = fieldHasPayloads;
}

void Lucene50SkipWriter::resetSkip()
{
  lastDocFP = docOut->getFilePointer();
  if (fieldHasPositions) {
    lastPosFP = posOut->getFilePointer();
    if (fieldHasOffsets || fieldHasPayloads) {
      lastPayFP = payOut->getFilePointer();
    }
  }
  initialized = false;
}

void Lucene50SkipWriter::initSkip()
{
  if (!initialized) {
    MultiLevelSkipListWriter::resetSkip();
    Arrays::fill(lastSkipDoc, 0);
    Arrays::fill(lastSkipDocPointer, lastDocFP);
    if (fieldHasPositions) {
      Arrays::fill(lastSkipPosPointer, lastPosFP);
      if (fieldHasPayloads) {
        Arrays::fill(lastPayloadByteUpto, 0);
      }
      if (fieldHasOffsets || fieldHasPayloads) {
        Arrays::fill(lastSkipPayPointer, lastPayFP);
      }
    }
    initialized = true;
  }
}

void Lucene50SkipWriter::bufferSkip(int doc, int numDocs, int64_t posFP,
                                    int64_t payFP, int posBufferUpto,
                                    int payloadByteUpto) 
{
  initSkip();
  this->curDoc = doc;
  this->curDocPointer = docOut->getFilePointer();
  this->curPosPointer = posFP;
  this->curPayPointer = payFP;
  this->curPosBufferUpto = posBufferUpto;
  this->curPayloadByteUpto = payloadByteUpto;
  bufferSkip(numDocs);
}

void Lucene50SkipWriter::writeSkipData(
    int level, shared_ptr<IndexOutput> skipBuffer) 
{
  int delta = curDoc - lastSkipDoc[level];

  skipBuffer->writeVInt(delta);
  lastSkipDoc[level] = curDoc;

  skipBuffer->writeVLong(curDocPointer - lastSkipDocPointer[level]);
  lastSkipDocPointer[level] = curDocPointer;

  if (fieldHasPositions) {

    skipBuffer->writeVLong(curPosPointer - lastSkipPosPointer[level]);
    lastSkipPosPointer[level] = curPosPointer;
    skipBuffer->writeVInt(curPosBufferUpto);

    if (fieldHasPayloads) {
      skipBuffer->writeVInt(curPayloadByteUpto);
    }

    if (fieldHasOffsets || fieldHasPayloads) {
      skipBuffer->writeVLong(curPayPointer - lastSkipPayPointer[level]);
      lastSkipPayPointer[level] = curPayPointer;
    }
  }
}
} // namespace org::apache::lucene::codecs::lucene50