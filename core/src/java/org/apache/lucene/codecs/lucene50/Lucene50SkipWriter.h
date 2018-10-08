#pragma once
#include "../MultiLevelSkipListWriter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::codecs::lucene50
{

using IndexOutput = org::apache::lucene::store::IndexOutput;
using MultiLevelSkipListWriter =
    org::apache::lucene::codecs::MultiLevelSkipListWriter;

/**
 * Write skip lists with multiple levels, and support skip within block ints.
 *
 * Assume that docFreq = 28, skipInterval = blockSize = 12
 *
 *  |       block#0       | |      block#1        | |vInts|
 *  d d d d d d d d d d d d d d d d d d d d d d d d d d d d (posting deque)
 *                          ^                       ^       (level 0 skip point)
 *
 * Note that skipWriter will ignore first document in block#0, since
 * it is useless as a skip point.  Also, we'll never skip into the vInts
 * block, only record skip data at the start its start point(if it exist).
 *
 * For each skip point, we will record:
 * 1. docID in former position, i.e. for position 12, record docID[11], etc.
 * 2. its related file points(position, payload),
 * 3. related numbers or uptos(position, payload).
 * 4. start offset.
 *
 */
class Lucene50SkipWriter final : public MultiLevelSkipListWriter
{
  GET_CLASS_NAME(Lucene50SkipWriter)
private:
  std::deque<int> lastSkipDoc;
  std::deque<int64_t> lastSkipDocPointer;
  std::deque<int64_t> lastSkipPosPointer;
  std::deque<int64_t> lastSkipPayPointer;
  std::deque<int> lastPayloadByteUpto;

  const std::shared_ptr<IndexOutput> docOut;
  const std::shared_ptr<IndexOutput> posOut;
  const std::shared_ptr<IndexOutput> payOut;

  int curDoc = 0;
  int64_t curDocPointer = 0;
  int64_t curPosPointer = 0;
  int64_t curPayPointer = 0;
  int curPosBufferUpto = 0;
  int curPayloadByteUpto = 0;
  bool fieldHasPositions = false;
  bool fieldHasOffsets = false;
  bool fieldHasPayloads = false;

public:
  Lucene50SkipWriter(int maxSkipLevels, int blockSize, int docCount,
                     std::shared_ptr<IndexOutput> docOut,
                     std::shared_ptr<IndexOutput> posOut,
                     std::shared_ptr<IndexOutput> payOut);

  void setField(bool fieldHasPositions, bool fieldHasOffsets,
                bool fieldHasPayloads);

  // tricky: we only skip data for blocks (terms with more than 128 docs), but
  // re-init'ing the skipper is pretty slow for rare terms in large segments as
  // we have to fill O(log #docs in segment) of junk. this is the vast majority
  // of terms (worst case: ID field or similar).  so in resetSkip() we save away
  // the previous pointers, and lazy-init only if we need to buffer skip data
  // for the term.
private:
  bool initialized = false;

public:
  int64_t lastDocFP = 0;
  int64_t lastPosFP = 0;
  int64_t lastPayFP = 0;

  void resetSkip() override;

  void initSkip();

  /**
   * Sets the values for the current skip data.
   */
  void bufferSkip(int doc, int numDocs, int64_t posFP, int64_t payFP,
                  int posBufferUpto, int payloadByteUpto) ;

protected:
  void writeSkipData(int level, std::shared_ptr<IndexOutput> skipBuffer) throw(
      IOException) override;

protected:
  std::shared_ptr<Lucene50SkipWriter> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50SkipWriter>(
        org.apache.lucene.codecs.MultiLevelSkipListWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene50/
