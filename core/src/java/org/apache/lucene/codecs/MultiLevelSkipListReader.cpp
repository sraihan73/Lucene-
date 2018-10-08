using namespace std;

#include "MultiLevelSkipListReader.h"
#include "../store/BufferedIndexInput.h"
#include "../util/MathUtil.h"

namespace org::apache::lucene::codecs
{
using BufferedIndexInput = org::apache::lucene::store::BufferedIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using MathUtil = org::apache::lucene::util::MathUtil;

MultiLevelSkipListReader::MultiLevelSkipListReader(
    shared_ptr<IndexInput> skipStream, int maxSkipLevels, int skipInterval,
    int skipMultiplier)
    : skipMultiplier(skipMultiplier)
{
  this->skipStream = std::deque<std::shared_ptr<IndexInput>>(maxSkipLevels);
  this->skipPointer = std::deque<int64_t>(maxSkipLevels);
  this->childPointer = std::deque<int64_t>(maxSkipLevels);
  this->numSkipped = std::deque<int>(maxSkipLevels);
  this->maxNumberOfSkipLevels = maxSkipLevels;
  this->skipInterval = std::deque<int>(maxSkipLevels);
  this->skipStream[0] = skipStream;
  this->inputIsBuffered =
      (std::dynamic_pointer_cast<BufferedIndexInput>(skipStream) != nullptr);
  this->skipInterval[0] = skipInterval;
  for (int i = 1; i < maxSkipLevels; i++) {
    // cache skip intervals
    this->skipInterval[i] = this->skipInterval[i - 1] * skipMultiplier;
  }
  skipDoc = std::deque<int>(maxSkipLevels);
}

MultiLevelSkipListReader::MultiLevelSkipListReader(
    shared_ptr<IndexInput> skipStream, int maxSkipLevels, int skipInterval)
    : MultiLevelSkipListReader(skipStream, maxSkipLevels, skipInterval,
                               skipInterval)
{
}

int MultiLevelSkipListReader::getDoc() { return lastDoc; }

int MultiLevelSkipListReader::skipTo(int target) 
{

  // walk up the levels until highest level is found that has a skip
  // for this target
  int level = 0;
  while (level < numberOfSkipLevels - 1 && target > skipDoc[level + 1]) {
    level++;
  }

  while (level >= 0) {
    if (target > skipDoc[level]) {
      if (!loadNextSkip(level)) {
        continue;
      }
    } else {
      // no more skips on this level, go down one level
      if (level > 0 &&
          lastChildPointer > skipStream[level - 1]->getFilePointer()) {
        seekChild(level - 1);
      }
      level--;
    }
  }

  return numSkipped[0] - skipInterval[0] - 1;
}

bool MultiLevelSkipListReader::loadNextSkip(int level) 
{
  // we have to skip, the target document is greater than the current
  // skip deque entry
  setLastSkipData(level);

  numSkipped[level] += skipInterval[level];

  // numSkipped may overflow a signed int, so compare as unsigned.
  if (Integer::compareUnsigned(numSkipped[level], docCount) > 0) {
    // this skip deque is exhausted
    skipDoc[level] = numeric_limits<int>::max();
    if (numberOfSkipLevels > level) {
      numberOfSkipLevels = level;
    }
    return false;
  }

  // read next skip entry
  skipDoc[level] += readSkipData(level, skipStream[level]);

  if (level != 0) {
    // read the child pointer if we are not on the leaf level
    childPointer[level] =
        skipStream[level]->readVLong() + skipPointer[level - 1];
  }

  return true;
}

void MultiLevelSkipListReader::seekChild(int level) 
{
  skipStream[level]->seek(lastChildPointer);
  numSkipped[level] = numSkipped[level + 1] - skipInterval[level + 1];
  skipDoc[level] = lastDoc;
  if (level > 0) {
    childPointer[level] =
        skipStream[level]->readVLong() + skipPointer[level - 1];
  }
}

MultiLevelSkipListReader::~MultiLevelSkipListReader()
{
  for (int i = 1; i < skipStream.size(); i++) {
    if (skipStream[i] != nullptr) {
      delete skipStream[i];
    }
  }
}

void MultiLevelSkipListReader::init(int64_t skipPointer,
                                    int df) 
{
  this->skipPointer[0] = skipPointer;
  this->docCount = df;
  assert((skipPointer >= 0 && skipPointer <= skipStream[0]->length(),
          L"invalid skip pointer: " + to_wstring(skipPointer) + L", length=" +
              to_wstring(skipStream[0]->length())));
  Arrays::fill(skipDoc, 0);
  Arrays::fill(numSkipped, 0);
  Arrays::fill(childPointer, 0);

  for (int i = 1; i < numberOfSkipLevels; i++) {
    skipStream[i].reset();
  }
  loadSkipLevels();
}

void MultiLevelSkipListReader::loadSkipLevels() 
{
  if (docCount <= skipInterval[0]) {
    numberOfSkipLevels = 1;
  } else {
    numberOfSkipLevels =
        1 + MathUtil::log(docCount / skipInterval[0], skipMultiplier);
  }

  if (numberOfSkipLevels > maxNumberOfSkipLevels) {
    numberOfSkipLevels = maxNumberOfSkipLevels;
  }

  skipStream[0]->seek(skipPointer[0]);

  int toBuffer = numberOfLevelsToBuffer;

  for (int i = numberOfSkipLevels - 1; i > 0; i--) {
    // the length of the current level
    int64_t length = skipStream[0]->readVLong();

    // the start pointer of the current level
    skipPointer[i] = skipStream[0]->getFilePointer();
    if (toBuffer > 0) {
      // buffer this level
      skipStream[i] =
          make_shared<SkipBuffer>(skipStream[0], static_cast<int>(length));
      toBuffer--;
    } else {
      // clone this stream, it is already at the start of the current level
      skipStream[i] = skipStream[0]->clone();
      if (inputIsBuffered && length < BufferedIndexInput::BUFFER_SIZE) {
        (std::static_pointer_cast<BufferedIndexInput>(skipStream[i]))
            ->setBufferSize(max(BufferedIndexInput::MIN_BUFFER_SIZE,
                                static_cast<int>(length)));
      }

      // move base stream beyond the current level
      skipStream[0]->seek(skipStream[0]->getFilePointer() + length);
    }
  }

  // use base stream for the lowest level
  skipPointer[0] = skipStream[0]->getFilePointer();
}

void MultiLevelSkipListReader::setLastSkipData(int level)
{
  lastDoc = skipDoc[level];
  lastChildPointer = childPointer[level];
}

MultiLevelSkipListReader::SkipBuffer::SkipBuffer(shared_ptr<IndexInput> input,
                                                 int length) 
    : org::apache::lucene::store::IndexInput(L"SkipBuffer on " + input)
{
  data = std::deque<char>(length);
  pointer = input->getFilePointer();
  input->readBytes(data, 0, length);
}

MultiLevelSkipListReader::SkipBuffer::~SkipBuffer() { data.clear(); }

int64_t MultiLevelSkipListReader::SkipBuffer::getFilePointer()
{
  return pointer + pos;
}

int64_t MultiLevelSkipListReader::SkipBuffer::length() { return data.size(); }

char MultiLevelSkipListReader::SkipBuffer::readByte() { return data[pos++]; }

void MultiLevelSkipListReader::SkipBuffer::readBytes(std::deque<char> &b,
                                                     int offset, int len)
{
  System::arraycopy(data, pos, b, offset, len);
  pos += len;
}

void MultiLevelSkipListReader::SkipBuffer::seek(int64_t pos)
{
  this->pos = static_cast<int>(pos - pointer);
}

shared_ptr<IndexInput>
MultiLevelSkipListReader::SkipBuffer::slice(const wstring &sliceDescription,
                                            int64_t offset,
                                            int64_t length) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs