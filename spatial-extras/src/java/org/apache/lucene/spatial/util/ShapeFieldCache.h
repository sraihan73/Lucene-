#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>
#include <deque>

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
namespace org::apache::lucene::spatial::util
{

using org::locationtech::spatial4j::shape::Shape;

/**
 * Bounded Cache of Shapes associated with docIds.  Note, multiple Shapes can be
 * associated with a given docId.
 * <p>
 * WARNING: This class holds the data in an extremely inefficient manner as all
 * Points are in memory as objects and they are stored in many ArrayLists (one
 * per document).  So it works but doesn't scale.  It will be replaced in the
 * future.
 *
 * @lucene.internal
 */
template <typename T>
class ShapeFieldCache : public std::enable_shared_from_this<ShapeFieldCache>
{
  GET_CLASS_NAME(ShapeFieldCache)
  static_assert(
      std::is_base_of<org.locationtech.spatial4j.shape.Shape, T>::value,
      L"T must inherit from org.locationtech.spatial4j.shape.Shape");

private:
  std::deque<std::deque<T>> const cache;

public:
  const int defaultLength;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) public
  // ShapeFieldCache(int length, int defaultLength)
  ShapeFieldCache(int length, int defaultLength)
      : cache(std::deque<std::deque>(length))
  {
    this->defaultLength = defaultLength;
  }

  virtual void add(int docid, T s)
  {
    std::deque<T> deque = cache[docid];
    if (deque.empty()) {
      deque = cache[docid] = std::deque<>(defaultLength);
    }
    deque.push_back(s);
  }

  virtual std::deque<T> getShapes(int docid) { return cache[docid]; }
};

} // namespace org::apache::lucene::spatial::util
