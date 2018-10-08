#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Collector.h"

#include  "core/src/java/org/apache/lucene/util/Counter.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/TimerThread.h"

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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Counter = org::apache::lucene::util::Counter;

/**
 * The {@link TimeLimitingCollector} is used to timeout search requests that
 * take longer than the maximum allowed search time limit. After this time is
 * exceeded, the search thread is stopped by throwing a
 * {@link TimeExceededException}.
 */
class TimeLimitingCollector
    : public std::enable_shared_from_this<TimeLimitingCollector>,
      public Collector
{
  GET_CLASS_NAME(TimeLimitingCollector)

  /** Thrown when elapsed search time exceeds allowed search time. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") public static class
  // TimeExceededException extends RuntimeException
  class TimeExceededException : public std::runtime_error
  {
  private:
    int64_t timeAllowed = 0;
    int64_t timeElapsed = 0;
    int lastDocCollected = 0;
    TimeExceededException(int64_t timeAllowed, int64_t timeElapsed,
                          int lastDocCollected);
    /** Returns allowed time (milliseconds). */
  public:
    virtual int64_t getTimeAllowed();
    /** Returns elapsed time (milliseconds). */
    virtual int64_t getTimeElapsed();
    /** Returns last doc (absolute doc id) that was collected when the search
     * time exceeded. */
    virtual int getLastDocCollected();

  protected:
    std::shared_ptr<TimeExceededException> shared_from_this()
    {
      return std::static_pointer_cast<TimeExceededException>(
          RuntimeException::shared_from_this());
    }
  };

private:
  int64_t t0 = std::numeric_limits<int64_t>::min();
  int64_t timeout = std::numeric_limits<int64_t>::min();
  std::shared_ptr<Collector> collector;
  const std::shared_ptr<Counter> clock;
  const int64_t ticksAllowed;
  bool greedy = false;
  int docBase = 0;

  /**
   * Create a TimeLimitedCollector wrapper over another {@link Collector} with a
   * specified timeout.
   * @param collector the wrapped {@link Collector}
   * @param clock the timer clock
   * @param ticksAllowed max time allowed for collecting
   * hits after which {@link TimeExceededException} is thrown
   */
public:
  TimeLimitingCollector(std::shared_ptr<Collector> collector,
                        std::shared_ptr<Counter> clock,
                        int64_t const ticksAllowed);

  /**
   * Sets the baseline for this collector. By default the collectors baseline is
   * initialized once the first reader is passed to the collector.
   * To include operations executed in prior to the actual document collection
   * set the baseline through this method in your prelude.
   * <p>
   * Example usage:
   * <pre class="prettyprint">
   *   Counter clock = ...;
   *   long baseline = clock.get();
   *   // ... prepare search
   *   TimeLimitingCollector collector = new TimeLimitingCollector(c, clock,
   * numTicks); collector.setBaseline(baseline); indexSearcher.search(query,
   * collector);
   * </pre>
   * @see #setBaseline()
   */
  virtual void setBaseline(int64_t clockTime);

  /**
   * Syntactic sugar for {@link #setBaseline(long)} using {@link Counter#get()}
   * on the clock passed to the constructor.
   */
  virtual void setBaseline();

  /**
   * Checks if this time limited collector is greedy in collecting the last hit.
   * A non greedy collector, upon a timeout, would throw a {@link
   * TimeExceededException} without allowing the wrapped collector to collect
   * current doc. A greedy one would first allow the wrapped hit collector to
   * collect current doc and only then throw a {@link TimeExceededException}.
   * However, if the timeout is detected in
   * {@link #getLeafCollector} then no current document is collected.
   * @see #setGreedy(bool)
   */
  virtual bool isGreedy();

  /**
   * Sets whether this time limited collector is greedy.
   * @param greedy true to make this time limited greedy
   * @see #isGreedy()
   */
  virtual void setGreedy(bool greedy);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
  {
    GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TimeLimitingCollector> outerInstance;

    int64_t time = 0;

  public:
    FilterLeafCollectorAnonymousInnerClass(
        std::shared_ptr<TimeLimitingCollector> outerInstance,
        std::shared_ptr<org::apache::lucene::search::LeafCollector>
            getLeafCollector,
        int64_t time);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
          FilterLeafCollector::shared_from_this());
    }
  };

public:
  bool needsScores() override;

  /**
   * This is so the same timer can be used with a multi-phase search process
   * such as grouping. We don't want to create a new TimeLimitingCollector for
   * each phase because that would reset the timer for each phase.  Once time is
   * up subsequent phases need to timeout quickly.
   *
   * @param collector The actual collector performing search functionality
   */
  virtual void setCollector(std::shared_ptr<Collector> collector);

  /**
   * Returns the global TimerThreads {@link Counter}
   * <p>
   * Invoking this creates may create a new instance of {@link TimerThread} iff
   * the global {@link TimerThread} has never been accessed before. The thread
   * returned from this method is started on creation and will be alive unless
   * you stop the {@link TimerThread} via {@link TimerThread#stopTimer()}.
   * </p>
   * @return the global TimerThreads {@link Counter}
   * @lucene.experimental
   */
  static std::shared_ptr<Counter> getGlobalCounter();

  /**
   * Returns the global {@link TimerThread}.
   * <p>
   * Invoking this creates may create a new instance of {@link TimerThread} iff
   * the global {@link TimerThread} has never been accessed before. The thread
   * returned from this method is started on creation and will be alive unless
   * you stop the {@link TimerThread} via {@link TimerThread#stopTimer()}.
   * </p>
   *
   * @return the global {@link TimerThread}
   * @lucene.experimental
   */
  static std::shared_ptr<TimerThread> getGlobalTimerThread();

private:
  class TimerThreadHolder final
      : public std::enable_shared_from_this<TimerThreadHolder>
  {
    GET_CLASS_NAME(TimerThreadHolder)
  public:
    static const std::shared_ptr<TimerThread> THREAD;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static TimerThreadHolder::StaticConstructor staticConstructor;
  };

  /**
   * Thread used to timeout search requests.
   * Can be stopped completely with {@link TimerThread#stopTimer()}
   * @lucene.experimental
   */
public:
  class TimerThread final : public Thread
  {
    GET_CLASS_NAME(TimerThread)

  public:
    static const std::wstring THREAD_NAME;
    static constexpr int DEFAULT_RESOLUTION = 20;
    // NOTE: we can avoid explicit synchronization here for several reasons:
    // * updates to volatile long variables are atomic
    // * only single thread modifies this value
    // * use of volatile keyword ensures that it does not reside in
    //   a register, but in main memory (so that changes are visible to
    //   other threads).
    // * visibility of changes does not need to be instantaneous, we can
    //   afford losing a tick or two.
    //
    // See section 17 of the Java Language Specification for details.
  private:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile long time = 0;
    int64_t time = 0;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile bool stop = false;
    bool stop = false;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile long resolution;
    int64_t resolution = 0;

  public:
    const std::shared_ptr<Counter> counter;

    TimerThread(int64_t resolution, std::shared_ptr<Counter> counter);

    TimerThread(std::shared_ptr<Counter> counter);

    void run() override;

    /**
     * Get the timer value in milliseconds.
     */
    int64_t getMilliseconds();

    /**
     * Stops the timer thread
     */
    void stopTimer();

    /**
     * Return the timer resolution.
     * @see #setResolution(long)
     */
    int64_t getResolution();

    /**
     * Set the timer resolution.
     * The default timer resolution is 20 milliseconds.
     * This means that a search required to take no longer than
     * 800 milliseconds may be stopped after 780 to 820 milliseconds.
     * <br>Note that:
     * <ul>
     * <li>Finer (smaller) resolution is more accurate but less efficient.</li>
     * <li>Setting resolution to less than 5 milliseconds will be silently
     * modified to 5 milliseconds.</li> <li>Setting resolution smaller than
     * current resolution might take effect only after current resolution.
     * (Assume current resolution of 20 milliseconds is modified to 5
     * milliseconds, then it can take up to 20 milliseconds for the change to
     * have effect.</li>
     * </ul>
     */
    void setResolution(int64_t resolution);

  protected:
    std::shared_ptr<TimerThread> shared_from_this()
    {
      return std::static_pointer_cast<TimerThread>(Thread::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
