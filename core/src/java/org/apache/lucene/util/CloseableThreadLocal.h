#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <unordered_map>

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
namespace org::apache::lucene::util
{

/** Java's builtin ThreadLocal has a serious flaw:
 *  it can take an arbitrarily long amount of time to
 *  dereference the things you had stored in it, even once the
 *  ThreadLocal instance itself is no longer referenced.
 *  This is because there is single, master map_obj stored for
 *  each thread, which all ThreadLocals share, and that
 *  master map_obj only periodically purges "stale" entries.
 *
 *  While not technically a memory leak, because eventually
 *  the memory will be reclaimed, it can take a long time
 *  and you can easily hit OutOfMemoryError because from the
 *  GC's standpoint the stale entries are not reclaimable.
 *
 *  This class works around that, by only enrolling
 *  WeakReference values into the ThreadLocal, and
 *  separately holding a hard reference to each stored
 *  value.  When you call {@link #close}, these hard
 *  references are cleared and then GC is freely able to
 *  reclaim space by objects stored in it.
 *
 *  We can not rely on {@link ThreadLocal#remove()} as it
 *  only removes the value for the caller thread, whereas
 *  {@link #close} takes care of all
 *  threads.  You should not call {@link #close} until all
 *  threads are done using the instance.
 *
 * @lucene.internal
 */

template <typename T>
class CloseableThreadLocal
    : public std::enable_shared_from_this<CloseableThreadLocal<T>>
{
  GET_CLASS_NAME(CloseableThreadLocal)

private:
  std::shared_ptr<ThreadLocal<WeakReference<T>>> t =
      std::make_shared<ThreadLocal<WeakReference<T>>>();

  // Use a WeakHashMap so that if a Thread exits and is
  // GC'able, its entry may be removed:
  std::unordered_map<std::shared_ptr<Thread>, T> hardRefs =
      std::make_shared<WeakHashMap<std::shared_ptr<Thread>, T>>();

  // Increase this to decrease frequency of purging in get:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static int PURGE_MULTIPLIER = 20;

  // On each get or set we decrement this; when it hits 0 we
  // purge.  After purge, we set this to
  // PURGE_MULTIPLIER * stillAliveCount.  This keeps
  // amortized cost of purging linear.
  const std::shared_ptr<AtomicInteger> countUntilPurge =
      std::make_shared<AtomicInteger>(PURGE_MULTIPLIER);

protected:
  virtual T initialValue() { return nullptr; }

public:
  virtual T get()
  {
    std::shared_ptr<WeakReference<T>> weakRef = t->get();
    if (weakRef == nullptr) {
      T iv = initialValue();
      if (iv != nullptr) {
        set(iv);
        return iv;
      } else {
        return nullptr;
      }
    } else {
      maybePurge();
      return weakRef->get();
    }
  }

  virtual void set(T object)
  {

    t->set(std::make_shared<WeakReference<>>(object));

    {
      std::lock_guard<std::mutex> lock(hardRefs);
      hardRefs.emplace(Thread::currentThread(), object);
      maybePurge();
    }
  }

private:
  void maybePurge()
  {
    if (countUntilPurge->getAndDecrement() == 0) {
      purge();
    }
  }

  // Purge dead threads
  void purge()
  {
    {
      std::lock_guard<std::mutex> lock(hardRefs);
      int stillAliveCount = 0;
      for (std::shared_ptr<Iterator<std::shared_ptr<Thread>>> it =
               hardRefs.keySet().begin();
           it->hasNext();) {
        std::shared_ptr<Thread> *const t = it->next();
        if (!t->isAlive()) {
          it->remove();
        } else {
          stillAliveCount++;
        }
      }
      int nextCount = (1 + stillAliveCount) * PURGE_MULTIPLIER;
      if (nextCount <= 0) {
        // defensive: int overflow!
        nextCount = 1000000;
      }

      countUntilPurge->set(nextCount);
    }
  }

public:
  virtual ~CloseableThreadLocal()
  {
    // Clear the hard refs; then, the only remaining refs to
    // all values we were storing are weak (unless somewhere
    // else is still using them) and so GC may reclaim them:
    hardRefs.clear();
    // Take care of the current thread right now; others will be
    // taken care of via the WeakReferences.
    if (t != nullptr) {
      t->remove();
    }
    t.reset();
  }
};

} // namespace org::apache::lucene::util
