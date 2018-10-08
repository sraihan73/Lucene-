#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::blocktree
{
class SegmentTermsEnumFrame;
}

namespace org::apache::lucene::util
{
class BytesRef;
}

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
namespace org::apache::lucene::codecs::blocktree
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * BlockTree statistics for a single field
 * returned by {@link FieldReader#getStats()}.
 * @lucene.internal
 */
class Stats : public std::enable_shared_from_this<Stats>
{
  GET_CLASS_NAME(Stats)
  /** Byte size of the index. */
public:
  int64_t indexNumBytes = 0;

  /** Total number of terms in the field. */
  int64_t totalTermCount = 0;

  /** Total number of bytes (sum of term lengths) across all terms in the field.
   */
  int64_t totalTermBytes = 0;

  // TODO: add total auto-prefix term count

  /** The number of normal (non-floor) blocks in the terms file. */
  int nonFloorBlockCount = 0;

  /** The number of floor blocks (meta-blocks larger than the
   *  allowed {@code maxItemsPerBlock}) in the terms file. */
  int floorBlockCount = 0;

  /** The number of sub-blocks within the floor blocks. */
  int floorSubBlockCount = 0;

  /** The number of "internal" blocks (that have both
   *  terms and sub-blocks). */
  int mixedBlockCount = 0;

  /** The number of "leaf" blocks (blocks that have only
   *  terms). */
  int termsOnlyBlockCount = 0;

  /** The number of "internal" blocks that do not contain
   *  terms (have only sub-blocks). */
  int subBlocksOnlyBlockCount = 0;

  /** Total number of blocks. */
  int totalBlockCount = 0;

  /** Number of blocks at each prefix depth. */
  std::deque<int> blockCountByPrefixLen = std::deque<int>(10);

private:
  int startBlockCount = 0;
  int endBlockCount = 0;

  /** Total number of bytes used to store term suffixes. */
public:
  int64_t totalBlockSuffixBytes = 0;

  /** Total number of bytes used to store term stats (not
   *  including what the {@link PostingsReaderBase}
   *  stores. */
  int64_t totalBlockStatsBytes = 0;

  /** Total bytes stored by the {@link PostingsReaderBase},
   *  plus the other few vInts stored in the frame. */
  int64_t totalBlockOtherBytes = 0;

  /** Segment name. */
  const std::wstring segment;

  /** Field name. */
  const std::wstring field;

  Stats(const std::wstring &segment, const std::wstring &field);

  virtual void startBlock(std::shared_ptr<SegmentTermsEnumFrame> frame,
                          bool isFloor);

  virtual void endBlock(std::shared_ptr<SegmentTermsEnumFrame> frame);

  virtual void term(std::shared_ptr<BytesRef> term);

  virtual void finish();

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::codecs::blocktree
