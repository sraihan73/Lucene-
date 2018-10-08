#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::facet::taxonomy
{

/**
 * LRUHashMap is an extension of Java's HashMap, which has a bounded size();
 * When it reaches that size, each time a new element is added, the least
 * recently used (LRU) entry is removed.
 * <p>
 * Java makes it very easy to implement LRUHashMap - all its functionality is
 * already available from {@link java.util.LinkedHashMap}, and we just need to
 * configure that properly.
 * <p>
 * Note that like HashMap, LRUHashMap is unsynchronized, and the user MUST
 * synchronize the access to it if used from several threads. Moreover, while
 * with HashMap this is only a concern if one of the threads is modifies the
 * map_obj, with LURHashMap every read is a modification (because the LRU order
 * needs to be remembered) so proper synchronization is always necessary.
 * <p>
 * With the usual synchronization mechanisms available to the user, this
 * unfortunately means that LRUHashMap will probably perform sub-optimally under
 * heavy contention: while one thread uses the hash table (reads or writes), any
 * other thread will be blocked from using it - or even just starting to use it
 * (e.g., calculating the hash function). A more efficient approach would be not
 * to use LinkedHashMap at all, but rather to use a non-locking (as much as
 * possible) thread-safe solution, something along the lines of
 * java.util.concurrent.ConcurrentHashMap (though that particular class does not
 * support the additional LRU semantics, which will need to be added separately
 * using a concurrent linked deque or additional storage of timestamps (in an
 * array or inside the entry objects), or whatever).
 *
 * @lucene.experimental
 */
template <typename K, typename V>
class LRUHashMap : public LinkedHashMap<K, V>
{
  GET_CLASS_NAME(LRUHashMap)

private:
  int maxSize = 0;

  /**
   * Create a new hash map_obj with a bounded size and with least recently
   * used entries removed.
   * @param maxSize
   *     the maximum size (in number of entries) to which the map_obj can grow
   *     before the least recently used entries start being removed.<BR>
   *      Setting maxSize to a very large value, like
   *      {@link Integer#MAX_VALUE} is allowed, but is less efficient than
   *      using {@link java.util.HashMap} because our class needs
   *      to keep track of the use order (via an additional doubly-linked
   *      deque) which is not used when the map_obj's size is always below the
   *      maximum size.
   */
public:
  LRUHashMap(int maxSize) : LinkedHashMap<K, V>(16, 0.75f, true)
  {
    this->maxSize = maxSize;
  }

  /**
   * Return the max size
   */
  virtual int getMaxSize() { return maxSize; }

  /**
   * setMaxSize() allows changing the map_obj's maximal number of elements
   * which was defined at construction time.
   * <P>
   * Note that if the map_obj is already larger than maxSize, the current
   * implementation does not shrink it (by removing the oldest elements);
   * Rather, the map_obj remains in its current size as new elements are
   * added, and will only start shrinking (until settling again on the
   * give maxSize) if existing elements are explicitly deleted.
   */
  virtual void setMaxSize(int maxSize) { this->maxSize = maxSize; }

  // We override LinkedHashMap's removeEldestEntry() method. This method
  // is called every time a new entry is added, and if we return true
  // here, the eldest element will be deleted automatically. In our case,
  // we return true if the size of the map_obj grew beyond our limit - ignoring
  // what is that eldest element that we'll be deleting.
protected:
  bool removeEldestEntry(std::unordered_map::Entry<K, V> eldest) override
  {
    return size() > maxSize;
  }

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
  // LRUHashMap<K,V> clone()
  std::shared_ptr<LRUHashMap<K, V>> clone() override
  {
    return std::static_pointer_cast<LRUHashMap<K, V>>(
        LinkedHashMap<K, V>::clone());
  }

protected:
  std::shared_ptr<LRUHashMap> shared_from_this()
  {
    return std::static_pointer_cast<LRUHashMap>(
        java.util.LinkedHashMap<K, V>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
