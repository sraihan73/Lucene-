#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>

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

using IndexWriter = org::apache::lucene::index::IndexWriter;

/** Utility class that runs a thread to manage periodicc
 *  reopens of a {@link ReferenceManager}, with methods to wait for a specific
GET_CLASS_NAME(that)
 *  index changes to become visible.  When a given search request needs to see a
specific
 *  index change, call the {#waitForGeneration} to wait for
 *  that change to be visible.  Note that this will only
 *  scale well if most searches do not need to wait for a
 *  specific index generation.
 *
 * @lucene.experimental */

template <typename T>
class ControlledRealTimeReopenThread : public Thread
{
  GET_CLASS_NAME(ControlledRealTimeReopenThread)
private:
  const std::shared_ptr<ReferenceManager<T>> manager;
  const int64_t targetMaxStaleNS;
  const int64_t targetMinStaleNS;
  const std::shared_ptr<IndexWriter> writer;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool finish;
  bool finish = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long waitingGen;
  int64_t waitingGen = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long searchingGen;
  int64_t searchingGen = 0;
  int64_t refreshStartGen = 0;

  const std::shared_ptr<ReentrantLock> reopenLock =
      std::make_shared<ReentrantLock>();
  const std::shared_ptr<Condition> reopenCond = reopenLock->newCondition();

  /**
   * Create ControlledRealTimeReopenThread, to periodically
   * reopen the a {@link ReferenceManager}.
   *
   * @param targetMaxStaleSec Maximum time until a new
   *        reader must be opened; this sets the upper bound
   *        on how slowly reopens may occur, when no
   *        caller is waiting for a specific generation to
   *        become visible.
   *
   * @param targetMinStaleSec Mininum time until a new
   *        reader can be opened; this sets the lower bound
   *        on how quickly reopens may occur, when a caller
   *        is waiting for a specific generation to
   *        become visible.
   */
public:
  ControlledRealTimeReopenThread(std::shared_ptr<IndexWriter> writer,
                                 std::shared_ptr<ReferenceManager<T>> manager,
                                 double targetMaxStaleSec,
                                 double targetMinStaleSec)
      : manager(manager), targetMaxStaleNS(static_cast<int64_t>(
                              1000000000 * targetMaxStaleSec)),
        targetMinStaleNS(
            static_cast<int64_t>(1000000000 * targetMinStaleSec)),
        writer(writer)
  {
    if (targetMaxStaleSec < targetMinStaleSec) {
      throw std::invalid_argument(
          "targetMaxScaleSec (= " + std::to_wstring(targetMaxStaleSec) +
          L") < targetMinStaleSec (=" + std::to_wstring(targetMinStaleSec) +
          L")");
    }
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    manager->addListener(std::make_shared<HandleRefresh>(shared_from_this()));
  }

private:
  class HandleRefresh : public std::enable_shared_from_this<HandleRefresh>,
                        public ReferenceManager::RefreshListener
  {
    GET_CLASS_NAME(HandleRefresh)
  private:
    std::shared_ptr<ControlledRealTimeReopenThread<std::shared_ptr<T>>>
        outerInstance;

  public:
    HandleRefresh(
        std::shared_ptr<ControlledRealTimeReopenThread<T>> outerInstance);

    void beforeRefresh() override;

    void afterRefresh(bool didRefresh) override;
  };

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void refreshDone()
  {
    searchingGen = refreshStartGen;
    notifyAll();
  }

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~ControlledRealTimeReopenThread()
  {
    // System.out.println("NRT: set finish");

    finish = true;

    // So thread wakes up and notices it should finish:
    reopenLock->lock();
    try {
      reopenCond->signal();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      reopenLock->unlock();
    }

    try {
      join();
    } catch (const InterruptedException &ie) {
      throw std::make_shared<ThreadInterruptedException>(ie);
    }

    // Max it out so any waiting search threads will return:
    searchingGen = std::numeric_limits<int64_t>::max();
    notifyAll();
  }

  /**
   * Waits for the target generation to become visible in
   * the searcher.
   * If the current searcher is older than the
   * target generation, this method will block
   * until the searcher is reopened, by another via
   * {@link ReferenceManager#maybeRefresh} or until the {@link ReferenceManager}
   * is closed.
   *
   * @param targetGen the generation to wait for
   */
  virtual void
  waitForGeneration(int64_t targetGen) 
  {
    waitForGeneration(targetGen, -1);
  }

  /**
   * Waits for the target generation to become visible in
   * the searcher, up to a maximum specified milli-seconds.
   * If the current searcher is older than the target
   * generation, this method will block until the
   * searcher has been reopened by another thread via
   * {@link ReferenceManager#maybeRefresh}, the given waiting time has elapsed,
   * or until the {@link ReferenceManager} is closed. <p> NOTE: if the waiting
   * time elapses before the requested target generation is available the
   * current {@link SearcherManager} is returned instead.
   *
   * @param targetGen
   *          the generation to wait for
   * @param maxMS
   *          maximum milliseconds to wait, or -1 to wait indefinitely
   * @return true if the targetGeneration is now available,
   *         or false if maxMS wait time was exceeded
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool waitForGeneration(int64_t targetGen,
                                 int maxMS) 
  {
    if (targetGen > searchingGen) {
      // Notify the reopen thread that the waitingGen has
      // changed, so it may wake up and realize it should
      // not sleep for much or any longer before reopening:
      reopenLock->lock();

      // Need to find waitingGen inside lock as it's used to determine
      // stale time
      waitingGen = std::max(waitingGen, targetGen);

      try {
        reopenCond->signal();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reopenLock->unlock();
      }

      int64_t startMS = System::nanoTime() / 1000000;

      while (targetGen > searchingGen) {
        if (maxMS < 0) {
          wait();
        } else {
          int64_t msLeft = (startMS + maxMS) - System::nanoTime() / 1000000;
          if (msLeft <= 0) {
            return false;
          } else {
            wait(msLeft);
          }
        }
      }
    }

    return true;
  }

  void run() override
  {
    // TODO: maybe use private thread ticktock timer, in
    // case clock shift messes up nanoTime?
    int64_t lastReopenStartNS = System::nanoTime();

    // System.out.println("reopen: start");
    while (!finish) {

      // TODO: try to guestimate how long reopen might
      // take based on past data?

      // Loop until we've waiting long enough before the
      // next reopen:
      while (!finish) {

        // Need lock before finding out if has waiting
        reopenLock->lock();
        try {
          // True if we have someone waiting for reopened searcher:
          bool hasWaiting = waitingGen > searchingGen;
          constexpr int64_t nextReopenStartNS =
              lastReopenStartNS +
              (hasWaiting ? targetMinStaleNS : targetMaxStaleNS);

          constexpr int64_t sleepNS = nextReopenStartNS - System::nanoTime();

          if (sleepNS > 0) {
            reopenCond->awaitNanos(sleepNS);
          } else {
            break;
          }
        } catch (const InterruptedException &ie) {
          Thread::currentThread().interrupt();
          return;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          reopenLock->unlock();
        }
      }

      if (finish) {
        break;
      }

      lastReopenStartNS = System::nanoTime();
      // Save the gen as of when we started the reopen; the
      // listener (HandleRefresh above) copies this to
      // searchingGen once the reopen completes:
      refreshStartGen = writer->getMaxCompletedSequenceNumber();
      try {
        manager->maybeRefreshBlocking();
      } catch (const IOException &ioe) {
        throw std::runtime_error(ioe);
      }
    }
  }

  /** Returns which {@code generation} the current searcher is guaranteed to
   * include. */
  virtual int64_t getSearchingGen() { return searchingGen; }

protected:
  std::shared_ptr<ControlledRealTimeReopenThread> shared_from_this()
  {
    return std::static_pointer_cast<ControlledRealTimeReopenThread>(
        Thread::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
