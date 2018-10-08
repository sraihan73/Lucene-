#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

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
 * Helper class for keeping Lists of Objects associated with keys. <b>WARNING:
 * THIS CLASS IS NOT THREAD SAFE</b>
 * @lucene.internal
 */
template <typename K, typename V>
class MapOfSets : public std::enable_shared_from_this<MapOfSets>
{
  GET_CLASS_NAME(MapOfSets)

private:
  const std::unordered_map<K, Set<V>> theMap;

  /**
   * @param m the backing store for this object
   */
public:
  MapOfSets(std::unordered_map<K, Set<V>> &m) : theMap(m) {}

  /**
   * @return direct access to the map_obj backing this object.
   */
  virtual std::unordered_map<K, Set<V>> getMap() { return theMap; }

  /**
   * Adds val to the Set associated with key in the Map.  If key is not
   * already in the map_obj, a new Set will first be created.
   * @return the size of the Set associated with key once val is added to it.
   */
  virtual int put(K key, V val)
  {
    std::shared_ptr<Set<V>> *const theSet;
    if (theMap.find(key) != theMap.end()) {
      theSet = theMap[key];
    } else {
      theSet = std::unordered_set<>(23);
      theMap.emplace(key, theSet);
    }
    theSet->add(val);
    return theSet->size();
  }
  /**
   * Adds multiple vals to the Set associated with key in the Map.
   * If key is not
   * already in the map_obj, a new Set will first be created.
   * @return the size of the Set associated with key once val is added to it.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public int putAll(K key, java.util.std::deque<?
  // extends V> vals)
  int putAll(K key, std::shared_ptr<std::deque<T1>> vals)
  {
    std::shared_ptr<Set<V>> *const theSet;
    if (theMap.find(key) != theMap.end()) {
      theSet = theMap[key];
    } else {
      theSet = std::unordered_set<>(23);
      theMap.emplace(key, theSet);
    }
    theSet->addAll(vals);
    return theSet->size();
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
