#pragma once
#include "../MultiLevelSkipListReader.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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

using MultiLevelSkipListReader =
    org::apache::lucene::codecs::MultiLevelSkipListReader;
using IndexInput = org::apache::lucene::store::IndexInput;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;

/**
 * Implements the skip deque reader for block postings format
 * that stores positions and payloads.
 *
 * Although this skipper uses MultiLevelSkipListReader as an interface,
 * its definition of skip position will be a little different.
 *
 * For example, when skipInterval = blockSize = 3, df = 2*skipInterval = 6,
 *
 * 0 1 2 3 4 5
 * d d d d d d    (posting deque)
 *     ^     ^    (skip point in MultiLeveSkipWriter)
 *       ^        (skip point in Lucene50SkipWriter)
 *
 * In this case, MultiLevelSkipListReader will use the last document as a skip
 * point, while Lucene50SkipReader should assume no skip point will comes.
 *
 * If we use the interface directly in Lucene50SkipReader, it may silly try to
 * read another skip data after the only skip point is loaded.
 *
 * To illustrate this, we can call skipTo(d[5]), since skip point d[3] has
 * smaller docId, and numSkipped+blockSize== df, the MultiLevelSkipListReader
 * will assume the skip deque isn't exhausted yet, and try to load a non-existed
 * skip point
 *
 * Therefore, we'll trim df before passing it to the interface. see trim(int)
 *
 */
class Lucene50SkipReader final : public MultiLevelSkipListReader
{
  GET_CLASS_NAME(Lucene50SkipReader)
private:
  std::deque<int64_t> docPointer;
  std::deque<int64_t> posPointer;
  std::deque<int64_t> payPointer;
  std::deque<int> posBufferUpto;
  std::deque<int> payloadByteUpto;

  int64_t lastPosPointer = 0;
  int64_t lastPayPointer = 0;
  int lastPayloadByteUpto = 0;
  int64_t lastDocPointer = 0;
  int lastPosBufferUpto = 0;

public:
  Lucene50SkipReader(std::shared_ptr<IndexInput> skipStream, int maxSkipLevels,
                     bool hasPos, bool hasOffsets, bool hasPayloads);

  /**
   * Trim original docFreq to tell skipReader read proper number of skip points.
   *
   * Since our definition in Lucene50Skip* is a little different from
   * MultiLevelSkip* This trimmed docFreq will prevent skipReader from:
   * 1. silly reading a non-existed skip point after the last block boundary
   * 2. moving into the vInt block
   *
   */
protected:
  int trim(int df);

public:
  void init(int64_t skipPointer, int64_t docBasePointer,
            int64_t posBasePointer, int64_t payBasePointer,
            int df) ;

  /** Returns the doc pointer of the doc to which the last call of
   * {@link MultiLevelSkipListReader#skipTo(int)} has skipped.  */
  int64_t getDocPointer();

  int64_t getPosPointer();

  int getPosBufferUpto();

  int64_t getPayPointer();

  int getPayloadByteUpto();

  int getNextSkipDoc();

protected:
  void seekChild(int level)  override;

  void setLastSkipData(int level) override;

  int readSkipData(int level, std::shared_ptr<IndexInput> skipStream) throw(
      IOException) override;

protected:
  std::shared_ptr<Lucene50SkipReader> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50SkipReader>(
        org.apache.lucene.codecs.MultiLevelSkipListReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene50/
