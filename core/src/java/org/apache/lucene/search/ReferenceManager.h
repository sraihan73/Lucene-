#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/RefreshListener.h"

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

/**
 * Utility class to safely share instances of a certain type across multiple
 * threads, while periodically refreshing them. This class ensures each
 * reference is closed only once all threads have finished using it. It is
 * recommended to consult the documentation of {@link ReferenceManager}
 * implementations for their {@link #maybeRefresh()} semantics.
 *
 * @param <G>
 *          the concrete type that will be {@link #acquire() acquired} and
 *          {@link #release(Object) released}.
 *
 * @lucene.experimental
 */
template <typename G>
class ReferenceManager : public std::enable_shared_from_this<ReferenceManager>
{
  GET_CLASS_NAME(ReferenceManager)

private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring REFERENCE_MANAGER_IS_CLOSED_MSG =
      L"this ReferenceManager is closed";

protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile G current;
  G current;

private:
  const std::shared_ptr<Lock> refreshLock = std::make_shared<ReentrantLock>();

  const std::deque<std::shared_ptr<RefreshListener>> refreshListeners =
      std::make_shared<
          CopyOnWriteArrayList<std::shared_ptr<RefreshListener>>>();

  void ensureOpen()
  {
    if (current == nullptr) {
      throw std::make_shared<AlreadyClosedException>(
          REFERENCE_MANAGER_IS_CLOSED_MSG);
    }
  }

  // C++ WARNING: The following method was originally marked 'synchronized':
  void swapReference(G newReference) 
  {
    ensureOpen();
    constexpr G oldReference = current;
    current = newReference;
    release(oldReference);
  }

  /**
   * Decrement reference counting on the given reference.
   * @throws IOException if reference decrement on the given resource failed.
   * */
protected:
  virtual void decRef(G reference) = 0;

  /**
   * Refresh the given reference if needed. Returns {@code null} if no refresh
   * was needed, otherwise a new refreshed reference.
   * @throws AlreadyClosedException if the reference manager has been {@link
   * #close() closed}.
   * @throws IOException if the refresh operation failed
   */
  virtual G refreshIfNeeded(G referenceToRefresh) = 0;

  /**
   * Try to increment reference counting on the given reference. Return true if
   * the operation was successful.
   * @throws AlreadyClosedException if the reference manager has been {@link
   * #close() closed}.
   */
  virtual bool tryIncRef(G reference) = 0;

  /**
   * Obtain the current reference. You must match every call to acquire with one
   * call to {@link #release}; it's best to do so in a finally clause, and set
   * the reference to {@code null} to prevent accidental usage after it has been
   * released.
   * @throws AlreadyClosedException if the reference manager has been {@link
   * #close() closed}.
   */
public:
  G acquire() 
  {
    G ref;

    do {
      if ((ref = current) == nullptr) {
        throw std::make_shared<AlreadyClosedException>(
            REFERENCE_MANAGER_IS_CLOSED_MSG);
      }
      if (tryIncRef(ref)) {
        return ref;
      }
      if (getRefCount(ref) == 0 && current == ref) {
        assert(ref != nullptr);
        /* if we can't increment the reader but we are
           still the current reference the RM is in a
           illegal states since we can't make any progress
           anymore. The reference is closed but the RM still
           holds on to it as the actual instance.
           This can only happen if somebody outside of the RM
           decrements the refcount without a corresponding increment
           since the RM assigns the new reference before counting down
           the reference. */
        throw std::make_shared<IllegalStateException>(
            L"The managed reference has already closed - this is likely a bug "
            L"when the reference count is modified outside of the "
            L"ReferenceManager");
      }
    } while (true);
  }

  /**
   * <p>
   * Closes this ReferenceManager to prevent future {@link #acquire()
   * acquiring}. A reference manager should be closed if the reference to the
   * managed resource should be disposed or the application using the {@link
   * ReferenceManager} is shutting down. The managed resource might not be
   * released immediately, if the {@link ReferenceManager} user is holding on to
   * a previously
   * {@link #acquire() acquired} reference. The resource will be released once
   * when the last reference is {@link #release(Object) released}. Those
   * references can still be used as if the manager was still active.
   * </p>
   * <p>
   * Applications should not {@link #acquire() acquire} new references from this
   * manager once this method has been called. {@link #acquire() Acquiring} a
   * resource on a closed {@link ReferenceManager} will throw an
   * {@link AlreadyClosedException}.
   * </p>
   *
   * @throws IOException
   *           if the underlying reader of the current reference could not be
   * closed
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  ~ReferenceManager()
  {
    if (current != nullptr) {
      // make sure we can call this more than once
      // closeable javadoc says:
      // if this is already closed then invoking this method has no effect.
      swapReference(nullptr);
      afterClose();
    }
  }

  /**
   * Returns the current reference count of the given reference.
   */
protected:
  virtual int getRefCount(G reference) = 0;

  /**
   *  Called after close(), so subclass can free any resources.
   *  @throws IOException if the after close operation in a sub-class throws an
{@link IOException} GET_CLASS_NAME(can)
   * */
  virtual void afterClose()  {}

private:
  void doMaybeRefresh() 
  {
    // it's ok to call lock() here (blocking) because we're supposed to get here
    // from either maybeRefreh() or maybeRefreshBlocking(), after the lock has
    // already been obtained. Doing that protects us from an accidental bug
    // where this method will be called outside the scope of refreshLock.
    // Per ReentrantLock's javadoc, calling lock() by the same thread more than
    // once is ok, as long as unlock() is called a matching number of times.
    refreshLock->lock();
    bool refreshed = false;
    try {
      constexpr G reference = acquire();
      try {
        notifyRefreshListenersBefore();
        G newReference = refreshIfNeeded(reference);
        if (newReference != nullptr) {
          assert(
              (newReference != reference,
               L"refreshIfNeeded should return null if refresh wasn't needed"));
          try {
            swapReference(newReference);
            refreshed = true;
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            if (!refreshed) {
              release(newReference);
            }
          }
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        release(reference);
        notifyRefreshListenersRefreshed(refreshed);
      }
      afterMaybeRefresh();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      refreshLock->unlock();
    }
  }

  /**
   * You must call this (or {@link #maybeRefreshBlocking()}), periodically, if
   * you want that {@link #acquire()} will return refreshed instances.
   *
   * <p>
   * <b>Threads</b>: it's fine for more than one thread to call this at once.
   * Only the first thread will attempt the refresh; subsequent threads will see
   * that another thread is already handling refresh and will return
   * immediately. Note that this means if another thread is already refreshing
   * then subsequent threads will return right away without waiting for the
   * refresh to complete.
   *
   * <p>
   * If this method returns true it means the calling thread either refreshed or
   * that there were no changes to refresh. If it returns false it means another
   * thread is currently refreshing.
   * </p>
   * @throws IOException if refreshing the resource causes an {@link
   * IOException}
   * @throws AlreadyClosedException if the reference manager has been {@link
   * #close() closed}.
   */
public:
  bool maybeRefresh() 
  {
    ensureOpen();

    // Ensure only 1 thread does refresh at once; other threads just return
    // immediately:
    constexpr bool doTryRefresh = refreshLock->tryLock();
    if (doTryRefresh) {
      try {
        doMaybeRefresh();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        refreshLock->unlock();
      }
    }

    return doTryRefresh;
  }

  /**
   * You must call this (or {@link #maybeRefresh()}), periodically, if you want
   * that {@link #acquire()} will return refreshed instances.
   *
   * <p>
   * <b>Threads</b>: unlike {@link #maybeRefresh()}, if another thread is
   * currently refreshing, this method blocks until that thread completes. It is
   * useful if you want to guarantee that the next call to {@link #acquire()}
   * will return a refreshed instance. Otherwise, consider using the
   * non-blocking {@link #maybeRefresh()}.
   * @throws IOException if refreshing the resource causes an {@link
   * IOException}
   * @throws AlreadyClosedException if the reference manager has been {@link
   * #close() closed}.
   */
  void maybeRefreshBlocking() 
  {
    ensureOpen();

    // Ensure only 1 thread does refresh at once
    refreshLock->lock();
    try {
      doMaybeRefresh();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      refreshLock->unlock();
    }
  }

  /** Called after a refresh was attempted, regardless of
   *  whether a new reference was in fact created.
   *  @throws IOException if a low level I/O exception occurs
   **/
protected:
  virtual void afterMaybeRefresh()  {}

  /**
   * Release the reference previously obtained via {@link #acquire()}.
   * <p>
   * <b>NOTE:</b> it's safe to call this after {@link #close()}.
   * @throws IOException if the release operation on the given resource throws
   * an {@link IOException}
   */
public:
  void release(G reference) 
  {
    assert(reference != nullptr);
    decRef(reference);
  }

private:
  void notifyRefreshListenersBefore() 
  {
    for (auto refreshListener : refreshListeners) {
      refreshListener->beforeRefresh();
    }
  }

  void notifyRefreshListenersRefreshed(bool didRefresh) 
  {
    for (auto refreshListener : refreshListeners) {
      refreshListener->afterRefresh(didRefresh);
    }
  }

  /**
   * Adds a listener, to be notified when a reference is refreshed/swapped.
   */
public:
  virtual void addListener(std::shared_ptr<RefreshListener> listener)
  {
    if (listener == nullptr) {
      throw std::make_shared<NullPointerException>(
          L"Listener must not be null");
    }
    refreshListeners.push_back(listener);
  }

  /**
   * Remove a listener added with {@link #addListener(RefreshListener)}.
   */
  virtual void removeListener(std::shared_ptr<RefreshListener> listener)
  {
    if (listener == nullptr) {
      throw std::make_shared<NullPointerException>(
          L"Listener must not be null");
    }
    // C++ TODO: The Java std::deque 'remove(Object)' method is not converted:
    refreshListeners.remove(listener);
  }

  /** Use to receive notification when a refresh has
   *  finished.  See {@link #addListener}. */
public:
  class RefreshListener
  {
    GET_CLASS_NAME(RefreshListener)

    /** Called right before a refresh attempt starts. */
  public:
    virtual void beforeRefresh() = 0;

    /** Called after the attempted refresh; if the refresh
     * did open a new reference then didRefresh will be true
     * and {@link #acquire()} is guaranteed to return the new
     * reference. */
    virtual void afterRefresh(bool didRefresh) = 0;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
