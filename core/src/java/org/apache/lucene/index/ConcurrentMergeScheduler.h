#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergeThread.h"

#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/RateLimiter.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/MergeRateLimiter.h"

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

using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using Directory = org::apache::lucene::store::Directory;

/** A {@link MergeScheduler} that runs each merge using a
 *  separate thread.
 *
 *  <p>Specify the max number of threads that may run at
 *  once, and the maximum number of simultaneous merges
 *  with {@link #setMaxMergesAndThreads}.</p>
 *
 *  <p>If the number of merges exceeds the max number of threads
 *  then the largest merges are paused until one of the smaller
 *  merges completes.</p>
 *
 *  <p>If more than {@link #getMaxMergeCount} merges are
 *  requested then this class will forcefully throttle the
 *  incoming threads by pausing until one more more merges
 *  complete.</p>
 *
 *  <p>This class attempts to detect whether the index is
 *  on rotational storage (traditional hard drive) or not
 *  (e.g. solid-state disk) and changes the default max merge
 *  and thread count accordingly.  This detection is currently
 *  Linux-only, and relies on the OS to put the right value
 *  into /sys/block/&lt;dev&gt;/block/rotational.  For all
 *  other operating systems it currently assumes a rotational
 *  disk for backwards compatibility.  To enable default
 *  settings for spinning or solid state disks for such
 *  operating systems, use {@link #setDefaultMaxMergesAndThreads(bool)}.
 */

class ConcurrentMergeScheduler : public MergeScheduler
{
  GET_CLASS_NAME(ConcurrentMergeScheduler)

  /** Dynamic default for {@code maxThreadCount} and {@code maxMergeCount},
   *  used to detect whether the index is backed by an SSD or rotational disk
   * and set {@code maxThreadCount} accordingly.  If it's an SSD,
   *  {@code maxThreadCount} is set to {@code max(1, min(4, cpuCoreCount/2))},
   *  otherwise 1.  Note that detection only currently works on
   *  Linux; other platforms will assume the index is not on an SSD. */
public:
  static constexpr int AUTO_DETECT_MERGES_AND_THREADS = -1;

  /** Used for testing.
   *
   * @lucene.internal */
  static const std::wstring DEFAULT_CPU_CORE_COUNT_PROPERTY;

  /** Used for testing.
   *
   * @lucene.internal */
  static const std::wstring DEFAULT_SPINS_PROPERTY;

  /** List of currently active {@link MergeThread}s. */
protected:
  const std::deque<std::shared_ptr<MergeThread>> mergeThreads =
      std::deque<std::shared_ptr<MergeThread>>();

  // Max number of merge threads allowed to be running at
  // once.  When there are more merges then this, we
  // forcefully pause the larger ones, letting the smaller
  // ones run, up until maxMergeCount merges at which point
  // we forcefully pause incoming threads (that presumably
  // are the ones causing so much merging).
private:
  int maxThreadCount = AUTO_DETECT_MERGES_AND_THREADS;

  // Max number of merges we accept before forcefully
  // throttling the incoming threads
  int maxMergeCount = AUTO_DETECT_MERGES_AND_THREADS;

  /** How many {@link MergeThread}s have kicked off (this is use
   *  to name them). */
protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  int mergeThreadCount_ = 0;

  /** Floor for IO write rate limit (we will never go any lower than this) */
private:
  static constexpr double MIN_MERGE_MB_PER_SEC = 5.0;

  /** Ceiling for IO write rate limit (we will never go any higher than this) */
  static constexpr double MAX_MERGE_MB_PER_SEC = 10240.0;

  /** Initial value for IO write rate limit when doAutoIOThrottle is true */
  static constexpr double START_MB_PER_SEC = 20.0;

  /** Merges below this size are not counted in the maxThreadCount, i.e. they
   * can freely run in their own thread (up until maxMergeCount). */
  static constexpr double MIN_BIG_MERGE_MB = 50.0;

  /** Current IO writes throttle rate */
protected:
  double targetMBPerSec = START_MB_PER_SEC;

  /** true if we should rate-limit writes for each merge */
private:
  bool doAutoIOThrottle = true;

  double forceMergeMBPerSec = std::numeric_limits<double>::infinity();

  /** Sole constructor, with all settings set to default
   *  values. */
public:
  ConcurrentMergeScheduler();

  /**
   * Expert: directly set the maximum number of merge threads and
   * simultaneous merges allowed.
   *
   * @param maxMergeCount the max # simultaneous merges that are allowed.
   *       If a merge is necessary yet we already have this many
   *       threads running, the incoming thread (that is calling
   *       add/updateDocument) will block until a merge thread
   *       has completed.  Note that we will only run the
   *       smallest <code>maxThreadCount</code> merges at a time.
   * @param maxThreadCount the max # simultaneous merge threads that should
   *       be running at once.  This must be &lt;= <code>maxMergeCount</code>
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setMaxMergesAndThreads(int maxMergeCount, int maxThreadCount);

  /** Sets max merges and threads to proper defaults for rotational
   *  or non-rotational storage.
   *
   * @param spins true to set defaults best for traditional rotatational storage
   * (spinning disks), else false (e.g. for solid-state disks)
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setDefaultMaxMergesAndThreads(bool spins);

  /** Set the per-merge IO throttle rate for forced merges (default: {@code
   * Double.POSITIVE_INFINITY}). */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setForceMergeMBPerSec(double v);

  /** Get the per-merge IO throttle rate for forced merges. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual double getForceMergeMBPerSec();

  /** Turn on dynamic IO throttling, to adaptively rate limit writes
   *  bytes/sec to the minimal rate necessary so merges do not fall behind.
   *  By default this is enabled. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void enableAutoIOThrottle();

  /** Turn off auto IO throttling.
   *
   * @see #enableAutoIOThrottle */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void disableAutoIOThrottle();

  /** Returns true if auto IO throttling is currently enabled. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool getAutoIOThrottle();

  /** Returns the currently set per-merge IO writes rate limit, if {@link
   * #enableAutoIOThrottle} was called, else {@code Double.POSITIVE_INFINITY}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual double getIORateLimitMBPerSec();

  /** Returns {@code maxThreadCount}.
   *
   * @see #setMaxMergesAndThreads(int, int) */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int getMaxThreadCount();

  /** See {@link #setMaxMergesAndThreads}. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int getMaxMergeCount();

  /** Removes the calling thread from the active merge threads. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void removeMergeThread();

  std::shared_ptr<Directory>
  wrapForMerge(std::shared_ptr<OneMerge> merge,
               std::shared_ptr<Directory> in_) override;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<ConcurrentMergeScheduler> outerInstance;

    std::shared_ptr<Directory> in_;
    std::shared_ptr<Thread> mergeThread;
    std::shared_ptr<RateLimiter> rateLimiter;

  public:
    FilterDirectoryAnonymousInnerClass(
        std::shared_ptr<ConcurrentMergeScheduler> outerInstance,
        std::shared_ptr<Directory> in_, std::shared_ptr<Thread> mergeThread,
        std::shared_ptr<RateLimiter> rateLimiter);

    std::shared_ptr<IndexOutput> createOutput(
        const std::wstring &name,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /**
   * Called whenever the running merges have changed, to set merge IO limits.
   * This method sorts the merge threads by their merge size in
   * descending order and then pauses/unpauses threads from first to last --
   * that way, smaller merges are guaranteed to run before larger ones.
   */

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void updateMergeThreads();

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void
  initDynamicDefaults(std::shared_ptr<IndexWriter> writer) ;

  static std::wstring rateToString(double mbPerSec);

public:
  virtual ~ConcurrentMergeScheduler();

  /** Wait for any running merge threads to finish. This call is not
   * interruptible as used by {@link #close()}. */
  virtual void sync();

  /**
   * Returns the number of merge threads that are alive, ignoring the calling
   * thread if it is a merge thread.  Note that this number is &le; {@link
   * #mergeThreads} size.
   *
   * @lucene.internal
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int mergeThreadCount();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
             bool newMergesFound)  override;

  /** This is invoked by {@link #merge} to possibly stall the incoming
   *  thread when there are too many merges running or pending.  The
   *  default behavior is to force this thread, which is producing too
   *  many segments for merging to keep up, to wait until merges catch
   *  up. Applications that can take other less drastic measures, such
   *  as limiting how many threads are allowed to index, can do nothing
   *  here and throttle elsewhere.
   *
   *  If this method wants to stall but the calling thread is a merge
   *  thread, it should return false to tell caller not to kick off
   *  any new merges. */

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool maybeStall(std::shared_ptr<IndexWriter> writer);

  /** Called from {@link #maybeStall} to pause the calling thread for a bit. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void doStall();

  /** Does the actual merge, by calling {@link IndexWriter#merge} */
  virtual void doMerge(std::shared_ptr<IndexWriter> writer,
                       std::shared_ptr<OneMerge> merge) ;

  /** Create and return a new MergeThread */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<MergeThread>
  getMergeThread(std::shared_ptr<IndexWriter> writer,
                 std::shared_ptr<OneMerge> merge) ;

  /** Runs a merge thread to execute a single merge, then exits. */
protected:
  class MergeThread : public Thread,
                      public Comparable<std::shared_ptr<MergeThread>>
  {
    GET_CLASS_NAME(MergeThread)
  private:
    std::shared_ptr<ConcurrentMergeScheduler> outerInstance;

  public:
    const std::shared_ptr<IndexWriter> writer;
    const std::shared_ptr<OneMerge> merge;
    const std::shared_ptr<MergeRateLimiter> rateLimiter;

    /** Sole constructor. */
    MergeThread(std::shared_ptr<ConcurrentMergeScheduler> outerInstance,
                std::shared_ptr<IndexWriter> writer,
                std::shared_ptr<OneMerge> merge);

    int compareTo(std::shared_ptr<MergeThread> other) override;

    void run() override;

  protected:
    std::shared_ptr<MergeThread> shared_from_this()
    {
      return std::static_pointer_cast<MergeThread>(Thread::shared_from_this());
    }
  };

  /** Called when an exception is hit in a background merge
   *  thread */
protected:
  virtual void handleMergeException(std::shared_ptr<Directory> dir,
                                    std::runtime_error exc);

private:
  bool suppressExceptions = false;

  /** Used for testing */
public:
  virtual void setSuppressExceptions();

  /** Used for testing */
  virtual void clearSuppressExceptions();

  virtual std::wstring toString();

private:
  bool isBacklog(int64_t now, std::shared_ptr<OneMerge> merge);

  /** Tunes IO throttle when a new merge starts. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void updateIOThrottle(
      std::shared_ptr<OneMerge> newMerge,
      std::shared_ptr<MergeRateLimiter> rateLimiter) ;

  /** Subclass can override to tweak targetMBPerSec. */
protected:
  virtual void targetMBPerSecChanged();

private:
  static double nsToSec(int64_t ns);

  static double bytesToMB(int64_t bytes);

protected:
  std::shared_ptr<ConcurrentMergeScheduler> shared_from_this()
  {
    return std::static_pointer_cast<ConcurrentMergeScheduler>(
        MergeScheduler::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
