#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/OneMergeProgress.h"
#include  "core/src/java/org/apache/lucene/index/MergeAbortedException.h"

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

using RateLimiter = org::apache::lucene::store::RateLimiter;

using OneMergeProgress =
    org::apache::lucene::index::MergePolicy::OneMergeProgress;

/** This is the {@link RateLimiter} that {@link IndexWriter} assigns to each
 * running merge, to give {@link MergeScheduler}s ionice like control.
 *
 *  @lucene.internal */

class MergeRateLimiter : public RateLimiter
{
  GET_CLASS_NAME(MergeRateLimiter)

private:
  static constexpr int MIN_PAUSE_CHECK_MSEC = 25;

  static const int64_t MIN_PAUSE_NS = TimeUnit::MILLISECONDS::toNanos(2);
  static const int64_t MAX_PAUSE_NS = TimeUnit::MILLISECONDS::toNanos(250);

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile double mbPerSec;
  double mbPerSec = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long minPauseCheckBytes;
  int64_t minPauseCheckBytes = 0;

  int64_t lastNS = 0;

  std::shared_ptr<AtomicLong> totalBytesWritten =
      std::make_shared<AtomicLong>();

  const std::shared_ptr<OneMergeProgress> mergeProgress;

  /** Sole constructor. */
public:
  MergeRateLimiter(std::shared_ptr<OneMergeProgress> mergeProgress);

  void setMBPerSec(double mbPerSec) override;

  double getMBPerSec() override;

  /** Returns total bytes written by this merge. */
  virtual int64_t getTotalBytesWritten();

  int64_t
  pause(int64_t bytes)  override;

  /** Total NS merge was stopped. */
  virtual int64_t getTotalStoppedNS();

  /** Total NS merge was paused to rate limit IO. */
  virtual int64_t getTotalPausedNS();

  /**
   * Returns the number of nanoseconds spent in a paused state or
   * <code>-1</code> if no pause was applied. If the thread needs pausing, this
   * method delegates to the linked {@link OneMergeProgress}.
   */
private:
  int64_t
  maybePause(int64_t bytes,
             int64_t curNS) ;

public:
  int64_t getMinPauseCheckBytes() override;

protected:
  std::shared_ptr<MergeRateLimiter> shared_from_this()
  {
    return std::static_pointer_cast<MergeRateLimiter>(
        org.apache.lucene.store.RateLimiter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
