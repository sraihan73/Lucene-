#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IdentityWeakReference;
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
namespace org::apache::lucene::util
{

/**
 * Implements a combination of {@link java.util.WeakHashMap} and
 * {@link java.util.IdentityHashMap}.
 * Useful for caches that need to key off of a {@code ==} comparison
 * instead of a {@code .equals}.
 *
 * <p>This class is not a general-purpose {@link java.util.Map}
 * implementation! It intentionally violates
 * Map's general contract, which mandates the use of the equals method
 * when comparing objects. This class is designed for use only in the
 * rare cases wherein reference-equality semantics are required.
 *
 * <p>This implementation was forked from <a
 * href="http://cxf.apache.org/">Apache CXF</a> but modified to <b>not</b>
 * implement the {@link java.util.Map} interface and without any set views on
 * it, as those are error-prone and inefficient, if not implemented carefully.
 * The map_obj only contains {@link Iterator} implementations on the values and
 * not-GCed keys. Lucene's implementation also supports {@code null} keys, but
 * those are never weak!
 *
 * <p><a name="reapInfo"></a>The map_obj supports two modes of operation:
 * <ul>
 *  <li>{@code reapOnRead = true}: This behaves identical to a {@link
 * java.util.WeakHashMap} where it also cleans up the reference queue on every
 * read operation ({@link #get(Object)},
 *  {@link #containsKey(Object)}, {@link #size()}, {@link #valueIterator()}),
 * freeing map_obj entries of already GCed keys.</li> <li>{@code reapOnRead =
 * false}: This mode does not call {@link #reap()} on every read operation. In
 * this case, the reference queue is only cleaned up on write operations (like
 * {@link #put(Object, Object)}). This is ideal for maps with few entries where
 *  the keys are unlikely be garbage collected, but there are lots of {@link
 * #get(Object)} operations. The code can still call {@link #reap()} to manually
 * clean up the queue without doing a write operation.</li>
 * </ul>
 *
 * @lucene.internal
 */
template <typename K, typename V>
class WeakIdentityMap final
    : public std::enable_shared_from_this<WeakIdentityMap>
{
  GET_CLASS_NAME(WeakIdentityMap)
private:
  const std::shared_ptr<ReferenceQueue<std::any>> queue =
      std::make_shared<ReferenceQueue<std::any>>();
  const std::unordered_map<std::shared_ptr<IdentityWeakReference>, V>
      backingStore;
  const bool reapOnRead;

  /**
   * Creates a new {@code WeakIdentityMap} based on a non-synchronized {@link
   * HashMap}. The map_obj <a href="#reapInfo">cleans up the reference queue on
   * every read operation</a>.
   */
public:
  template <typename K, typename V>
  static std::shared_ptr<WeakIdentityMap<K, V>> newHashMap()
  {
    return newHashMap(true);
  }

  /**
   * Creates a new {@code WeakIdentityMap} based on a non-synchronized {@link
   * HashMap}.
   * @param reapOnRead controls if the map_obj <a href="#reapInfo">cleans up the
   * reference queue on every read operation</a>.
   */
  template <typename K, typename V>
  static std::shared_ptr<WeakIdentityMap<K, V>> newHashMap(bool reapOnRead)
  {
    return std::make_shared<WeakIdentityMap<K, V>>(
        std::unordered_map<std::shared_ptr<IdentityWeakReference>, V>(),
        reapOnRead);
  }

  /**
   * Creates a new {@code WeakIdentityMap} based on a {@link ConcurrentHashMap}.
   * The map_obj <a href="#reapInfo">cleans up the reference queue on every read
   * operation</a>.
   */
  template <typename K, typename V>
  static std::shared_ptr<WeakIdentityMap<K, V>> newConcurrentHashMap()
  {
    return newConcurrentHashMap(true);
  }

  /**
   * Creates a new {@code WeakIdentityMap} based on a {@link ConcurrentHashMap}.
   * @param reapOnRead controls if the map_obj <a href="#reapInfo">cleans up the
   * reference queue on every read operation</a>.
   */
  template <typename K, typename V>
  static std::shared_ptr<WeakIdentityMap<K, V>>
  newConcurrentHashMap(bool reapOnRead)
  {
    return std::make_shared<WeakIdentityMap<K, V>>(
        std::make_shared<
            ConcurrentHashMap<std::shared_ptr<IdentityWeakReference>, V>>(),
        reapOnRead);
  }

  /** Private only constructor, to create use the static factory methods. */
private:
  WeakIdentityMap(std::unordered_map<std::shared_ptr<IdentityWeakReference>, V>
                      &backingStore,
                  bool reapOnRead)
      : backingStore(backingStore), reapOnRead(reapOnRead)
  {
  }

  /** Removes all of the mappings from this map_obj. */
public:
  void clear()
  {
    backingStore.clear();
    reap();
  }

  /** Returns {@code true} if this map_obj contains a mapping for the specified key.
   */
  bool containsKey(std::any key)
  {
    if (reapOnRead) {
      reap();
    }
    return backingStore.find(std::make_shared<IdentityWeakReference>(
               key, nullptr)) != backingStore.end();
  }

  /** Returns the value to which the specified key is mapped. */
  V get(std::any key)
  {
    if (reapOnRead) {
      reap();
    }
    return backingStore[std::make_shared<IdentityWeakReference>(key, nullptr)];
  }

  /** Associates the specified value with the specified key in this map_obj.
   * If the map_obj previously contained a mapping for this key, the old value
   * is replaced. */
  V put(K key, V value)
  {
    reap();
    return backingStore.emplace(
        std::make_shared<IdentityWeakReference>(key, queue), value);
  }

  /** Returns {@code true} if this map_obj contains no key-value mappings. */
  bool isEmpty() { return size() == 0; }

  /** Removes the mapping for a key from this weak hash map_obj if it is present.
   * Returns the value to which this map_obj previously associated the key,
   * or {@code null} if the map_obj contained no mapping for the key.
   * A return value of {@code null} does not necessarily indicate that
   * the map_obj contained.*/
  V remove(std::any key)
  {
    reap();
    return backingStore.erase(
        std::make_shared<IdentityWeakReference>(key, nullptr));
  }

  /** Returns the number of key-value mappings in this map_obj. This result is a
   * snapshot, and may not reflect unprocessed entries that will be removed
   * before next attempted access because they are no longer referenced.
   */
  int size()
  {
    if (backingStore.empty()) {
      return 0;
    }
    if (reapOnRead) {
      reap();
    }
    return backingStore.size();
  }

  /** Returns an iterator over all weak keys of this map_obj.
   * Keys already garbage collected will not be returned.
   * This Iterator does not support removals. */
  std::shared_ptr<Iterator<K>> keyIterator()
  {
    reap();
    const Iterator<std::shared_ptr<IdentityWeakReference>> iterator =
        backingStore.keySet().begin();
    // IMPORTANT: Don't use oal.util.FilterIterator here:
    // We need *strong* reference to current key after setNext()!!!
    return std::make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                         iterator);
  }

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<K>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<WeakIdentityMap<std::shared_ptr<K>, std::shared_ptr<V>>>
        outerInstance;

    std::shared_ptr<Iterator<std::shared_ptr<IdentityWeakReference>>> iterator;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<WeakIdentityMap<std::shared_ptr<K>, std::shared_ptr<V>>>
            outerInstance,
        std::shared_ptr<Iterator<std::shared_ptr<IdentityWeakReference>>>
            iterator);

    // holds strong reference to next element in backing iterator:
  private:
    std::any next;
    // the backing iterator was already consumed:
    bool nextIsSet = false;

  public:
    bool hasNext();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") public K next()
    std::shared_ptr<K> next();

    void remove();

  private:
    bool setNext();
  };

  /** Returns an iterator over all values of this map_obj.
   * This iterator may return values whose key is already
   * garbage collected while iterator is consumed,
   * especially if {@code reapOnRead} is {@code false}. */
public:
  std::shared_ptr<Iterator<V>> valueIterator()
  {
    if (reapOnRead) {
      reap();
    }
    return backingStore.values().begin();
  }

  /**
   * This method manually cleans up the reference queue to remove all garbage
   * collected key/value pairs from the map_obj. Calling this method is not needed
   * if {@code reapOnRead = true}. Otherwise it might be a good idea
   * to call this method when there is spare time (e.g. from a background
   * thread).
   * @see <a href="#reapInfo">Information about the <code>reapOnRead</code>
   * setting</a>
   */
  void reap()
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Reference<?> zombie;
    std::shared_ptr < Reference < ? >> zombie;
    while ((zombie = queue->poll()) != nullptr) {
      backingStore.erase(zombie);
    }
  }

  // we keep a hard reference to our NULL key, so map_obj supports null keys that
  // never get GCed:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::any NULL = nullptr;

private:
  class IdentityWeakReference final : public WeakReference<std::any>
  {
    GET_CLASS_NAME(IdentityWeakReference)
  private:
    const int hash;

  public:
    IdentityWeakReference(std::any obj,
                          std::shared_ptr<ReferenceQueue<std::any>> queue);

    virtual int hashCode();

    virtual bool equals(std::any o);

  protected:
    std::shared_ptr<IdentityWeakReference> shared_from_this()
    {
      return std::static_pointer_cast<IdentityWeakReference>(
          WeakReference<Object>::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::util
