#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FrozenBufferedUpdates.h"

#include  "core/src/java/org/apache/lucene/index/FinishedSegments.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/ReadersAndUpdates.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReader.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/IOUtils.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOConsumer;
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

namespace org::apache::lucene::index
{

using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

/** Tracks the stream of {@link FrozenBufferedUpdates}.
 * When DocumentsWriterPerThread flushes, its buffered
 * deletes and updates are appended to this stream and immediately
 * resolved (to actual docIDs, per segment) using the indexing
 * thread that triggered the flush for concurrency.  When a
 * merge kicks off, we sync to ensure all resolving packets
 * complete.  We also apply to all segments when NRT reader is pulled,
 * commit/close is called, or when too many deletes or updates are
 * buffered and must be flushed (by RAM usage or by count).
 *
 * Each packet is assigned a generation, and each flushed or
 * merged segment is also assigned a generation, so we can
 * track which BufferedDeletes packets to apply to any given
 * segment. */

class BufferedUpdatesStream final
    : public std::enable_shared_from_this<BufferedUpdatesStream>,
      public Accountable
{
  GET_CLASS_NAME(BufferedUpdatesStream)

private:
  const std::shared_ptr<Set<std::shared_ptr<FrozenBufferedUpdates>>> updates =
      std::unordered_set<std::shared_ptr<FrozenBufferedUpdates>>();

  // Starts at 1 so that SegmentInfos that have never had
  // deletes applied (whose bufferedDelGen defaults to 0)
  // will be correct:
  int64_t nextGen = 1;
  const std::shared_ptr<FinishedSegments> finishedSegments;
  const std::shared_ptr<InfoStream> infoStream;
  const std::shared_ptr<AtomicLong> bytesUsed = std::make_shared<AtomicLong>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicInteger> numTerms_ =
      std::make_shared<AtomicInteger>();

public:
  BufferedUpdatesStream(std::shared_ptr<InfoStream> infoStream);

  // Appends a new packet of buffered deletes to the stream,
  // setting its generation:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t push(std::shared_ptr<FrozenBufferedUpdates> packet);

  // C++ WARNING: The following method was originally marked 'synchronized':
  int getPendingUpdatesCount();

  /** Only used by IW.rollback */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void clear();

  bool any();

  int numTerms();

  int64_t ramBytesUsed() override;

public:
  class ApplyDeletesResult
      : public std::enable_shared_from_this<ApplyDeletesResult>
  {
    GET_CLASS_NAME(ApplyDeletesResult)

    // True if any actual deletes took place:
  public:
    const bool anyDeletes;

    // If non-null, contains segments that are 100% deleted
    const std::deque<std::shared_ptr<SegmentCommitInfo>> allDeleted;

    ApplyDeletesResult(
        bool anyDeletes,
        std::deque<std::shared_ptr<SegmentCommitInfo>> &allDeleted);
  };

  /** Waits for all in-flight packets, which are already being resolved
   * concurrently by indexing threads, to finish.  Returns true if there were
   * any new deletes or updates.  This is called for refresh, commit. */
public:
  void waitApplyAll(std::shared_ptr<IndexWriter> writer) ;

  /** Returns true if this delGen is still running. */
  bool stillRunning(int64_t delGen);

  void finishedSegment(int64_t delGen);

  /** Called by indexing threads once they are fully done resolving all deletes
   * for the provided delGen.  We track the completed delGens and record the
   * maximum delGen for which all prior delGens, inclusive, are completed, so
   * that it's safe for doc values updates to apply and write. */

  // C++ WARNING: The following method was originally marked 'synchronized':
  void finished(std::shared_ptr<FrozenBufferedUpdates> packet);

  /** All frozen packets up to and including this del gen are guaranteed to be
   * finished. */
  int64_t getCompletedDelGen();

  /** Waits only for those in-flight packets that apply to these merge segments.
   * This is called when a merge needs to finish and must ensure all deletes to
   * the merging segments are resolved. */
  void
  waitApplyForMerge(std::deque<std::shared_ptr<SegmentCommitInfo>> &mergeInfos,
                    std::shared_ptr<IndexWriter> writer) ;

private:
  void waitApply(
      std::shared_ptr<Set<std::shared_ptr<FrozenBufferedUpdates>>> waitFor,
      std::shared_ptr<IndexWriter> writer) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t getNextGen();

  /** Holds all per-segment internal state used while resolving deletions. */
public:
  class SegmentState final : public std::enable_shared_from_this<SegmentState>
  {
    GET_CLASS_NAME(SegmentState)
  public:
    const int64_t delGen;
    const std::shared_ptr<ReadersAndUpdates> rld;
    const std::shared_ptr<SegmentReader> reader;
    const int startDelCount;

  private:
    const IOUtils::IOConsumer<std::shared_ptr<ReadersAndUpdates>> onClose;

  public:
    std::shared_ptr<TermsEnum> termsEnum;
    std::shared_ptr<PostingsEnum> postingsEnum;
    std::shared_ptr<BytesRef> term;

    SegmentState(
        std::shared_ptr<ReadersAndUpdates> rld,
        IOUtils::IOConsumer<std::shared_ptr<ReadersAndUpdates>> onClose,
        std::shared_ptr<SegmentCommitInfo> info) ;

    virtual std::wstring toString();

    virtual ~SegmentState();
  };

  // only for assert
private:
  bool checkDeleteStats();

  /** Tracks the contiguous range of packets that have finished resolving.  We
   * need this because the packets are concurrently resolved, and we can only
   * write to disk the contiguous completed packets. */
private:
  class FinishedSegments : public std::enable_shared_from_this<FinishedSegments>
  {
    GET_CLASS_NAME(FinishedSegments)

    /** Largest del gen, inclusive, for which all prior packets have finished
     * applying. */
  private:
    int64_t completedDelGen = 0;

    /** This lets us track the "holes" in the current frontier of applying del
     *  gens; once the holes are filled in we can advance completedDelGen. */
    const std::shared_ptr<Set<int64_t>> finishedDelGens =
        std::unordered_set<int64_t>();

    const std::shared_ptr<InfoStream> infoStream;

  public:
    FinishedSegments(std::shared_ptr<InfoStream> infoStream);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual void clear();

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual bool stillRunning(int64_t delGen);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual int64_t getCompletedDelGen();

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual void finishedSegment(int64_t delGen);
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
