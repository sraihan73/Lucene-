#pragma once
#include "stringhelper.h"
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PrefixCodedTerms.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/BufferedUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/BufferedUpdatesStream.h"
#include  "core/src/java/org/apache/lucene/index/SegmentState.h"
#include  "core/src/java/org/apache/lucene/index/ApplyDeletesResult.h"

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
namespace org::apache::lucene::index
{

using Query = org::apache::lucene::search::Query;
using InfoStream = org::apache::lucene::util::InfoStream;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * Holds buffered deletes and updates by term or query, once pushed. Pushed
 * deletes/updates are write-once, so we shift to more memory efficient data
 * structure to hold them.  We don't hold docIDs because these are applied on
 * flush.
 */
class FrozenBufferedUpdates final
    : public std::enable_shared_from_this<FrozenBufferedUpdates>
{
  GET_CLASS_NAME(FrozenBufferedUpdates)

  /* NOTE: we now apply this frozen packet immediately on creation, yet this
   * process is heavy, and runs in multiple threads, and this compression is
   * sizable (~8.3% of the original size), so it's important
   * we run this before applying the deletes/updates. */

  /* Query we often undercount (say 24 bytes), plus int. */
public:
  static const int BYTES_PER_DEL_QUERY =
      RamUsageEstimator::NUM_BYTES_OBJECT_REF + Integer::BYTES + 24;

  // Terms, in sorted order:
  const std::shared_ptr<PrefixCodedTerms> deleteTerms;

  // Parallel array of deleted query, and the docIDUpto for each
  std::deque<std::shared_ptr<Query>> const deleteQueries;
  std::deque<int> const deleteQueryLimits;

  // numeric DV update term and their updates
  std::deque<char> const numericDVUpdates;

  // binary DV update term and their updates
  std::deque<char> const binaryDVUpdates;

private:
  const int numericDVUpdateCount;
  const int binaryDVUpdateCount;

  /** Counts down once all deletes/updates have been applied */
public:
  const std::shared_ptr<CountDownLatch> applied =
      std::make_shared<CountDownLatch>(1);

  /** How many total documents were deleted/updated. */
  int64_t totalDelCount = 0;

  const int bytesUsed;
  const int numTermDeletes;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t delGen_ = -1; // assigned by BufferedUpdatesStream once pushed

public:
  const std::shared_ptr<SegmentCommitInfo>
      privateSegment; // non-null iff this frozen packet represents
                      // a segment private deletes. in that case is should
                      // only have Queries and doc values updates
private:
  const std::shared_ptr<InfoStream> infoStream;

public:
  FrozenBufferedUpdates(
      std::shared_ptr<InfoStream> infoStream,
      std::shared_ptr<BufferedUpdates> updates,
      std::shared_ptr<SegmentCommitInfo> privateSegment) ;

private:
  template <typename T>
  static std::deque<char> freezeDVUpdates(
      std::unordered_map<std::wstring, LinkedHashMap<std::shared_ptr<Term>, T>>
          &dvUpdates,
      std::function<void(int)> &updateSizeConsumer) ;

  /** Returns the {@link SegmentCommitInfo} that this packet is supposed to
   * apply its deletes to, or null if the private segment was already merged
   * away. */
  std::deque<std::shared_ptr<SegmentCommitInfo>>
  getInfosToApply(std::shared_ptr<IndexWriter> writer);

  /** Translates a frozen packet of delete term/query, or doc values
   *  updates, into their actual docIDs in the index, and applies the change.
   * This is a heavy operation and is done concurrently by incoming indexing
   * threads. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") public synchronized void
  // apply(IndexWriter writer) throws java.io.IOException C++ WARNING: The
  // following method was originally marked 'synchronized':
  void apply(std::shared_ptr<IndexWriter> writer) ;

  /** Opens SegmentReader and inits SegmentState for each segment. */
private:
  static std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
  openSegmentStates(std::shared_ptr<IndexWriter> writer,
                    std::deque<std::shared_ptr<SegmentCommitInfo>> &infos,
                    std::shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
                        alreadySeenSegments,
                    int64_t delGen) ;

  /** Close segment states previously opened with openSegmentStates. */
public:
  static std::shared_ptr<BufferedUpdatesStream::ApplyDeletesResult>
  closeSegmentStates(
      std::shared_ptr<IndexWriter> writer,
      std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
          &segStates,
      bool success) ;

private:
  void
  finishApply(std::shared_ptr<IndexWriter> writer,
              std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
                  &segStates,
              bool success,
              std::shared_ptr<Set<std::wstring>> delFiles) ;

  /** Applies pending delete-by-term, delete-by-query and doc values updates to
   * all segments in the index, returning the number of new deleted or updated
   * documents. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t
  apply(std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
            &segStates) ;

  int64_t applyDocValuesUpdates(
      std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
          &segStates) ;

  static int64_t applyDocValuesUpdates(
      std::shared_ptr<BufferedUpdatesStream::SegmentState> segState,
      std::deque<char> &updates, bool isNumeric, int64_t delGen,
      bool segmentPrivateDeletes) ;

  // Delete by query
  int64_t applyQueryDeletes(
      std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
          &segStates) ;

  int64_t applyTermDeletes(
      std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
          &segStates) ;

public:
  void setDelGen(int64_t delGen);

  int64_t delGen();

  virtual std::wstring toString();

  bool any();
};

} // #include  "core/src/java/org/apache/lucene/index/
