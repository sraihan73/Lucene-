using namespace std;

#include "MultiLevelSkipListWriter.h"
#include "../store/IndexOutput.h"
#include "../store/RAMOutputStream.h"
#include "../util/MathUtil.h"

namespace org::apache::lucene::codecs
{
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using MathUtil = org::apache::lucene::util::MathUtil;

MultiLevelSkipListWriter::MultiLevelSkipListWriter(int skipInterval,
                                                   int skipMultiplier,
                                                   int maxSkipLevels, int df)
{
  this->skipInterval = skipInterval;
  this->skipMultiplier = skipMultiplier;

  // calculate the maximum number of skip levels for this document frequency
  if (df <= skipInterval) {
    numberOfSkipLevels = 1;
  } else {
    numberOfSkipLevels = 1 + MathUtil::log(df / skipInterval, skipMultiplier);
  }

  // make sure it does not exceed maxSkipLevels
  if (numberOfSkipLevels > maxSkipLevels) {
    numberOfSkipLevels = maxSkipLevels;
  }
}

MultiLevelSkipListWriter::MultiLevelSkipListWriter(int skipInterval,
                                                   int maxSkipLevels, int df)
    : MultiLevelSkipListWriter(skipInterval, skipInterval, maxSkipLevels, df)
{
}

void MultiLevelSkipListWriter::init()
{
  skipBuffer =
      std::deque<std::shared_ptr<RAMOutputStream>>(numberOfSkipLevels);
  for (int i = 0; i < numberOfSkipLevels; i++) {
    skipBuffer[i] = make_shared<RAMOutputStream>();
  }
}

void MultiLevelSkipListWriter::resetSkip()
{
  if (skipBuffer.empty()) {
    init();
  } else {
    for (int i = 0; i < skipBuffer.size(); i++) {
      skipBuffer[i]->reset();
    }
  }
}

void MultiLevelSkipListWriter::bufferSkip(int df) 
{

  assert(df % skipInterval == 0);
  int numLevels = 1;
  df /= skipInterval;

  // determine max level
  while ((df % skipMultiplier) == 0 && numLevels < numberOfSkipLevels) {
    numLevels++;
    df /= skipMultiplier;
  }

  int64_t childPointer = 0;

  for (int level = 0; level < numLevels; level++) {
    writeSkipData(level, skipBuffer[level]);

    int64_t newChildPointer = skipBuffer[level]->getFilePointer();

    if (level != 0) {
      // store child pointers for all levels except the lowest
      skipBuffer[level]->writeVLong(childPointer);
    }

    // remember the childPointer for the next level
    childPointer = newChildPointer;
  }
}

int64_t MultiLevelSkipListWriter::writeSkip(
    shared_ptr<IndexOutput> output) 
{
  int64_t skipPointer = output->getFilePointer();
  // System.out.println("skipper.writeSkip fp=" + skipPointer);
  if (skipBuffer.empty() || skipBuffer.empty()) {
    return skipPointer;
  }

  for (int level = numberOfSkipLevels - 1; level > 0; level--) {
    int64_t length = skipBuffer[level]->getFilePointer();
    if (length > 0) {
      output->writeVLong(length);
      skipBuffer[level]->writeTo(output);
    }
  }
  skipBuffer[0]->writeTo(output);

  return skipPointer;
}
} // namespace org::apache::lucene::codecs