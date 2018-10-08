#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
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
namespace org::apache::lucene::search
{

/**
 * A {@link Multiset} is a set that allows for duplicate elements. Two
 * {@link Multiset}s are equal if they contain the same unique elements and if
 * each unique element has as many occurrences in both multisets.
 * Iteration order is not specified.
 * @lucene.internal
 */
template <typename T>
class Multiset final : public AbstractCollection<T>
{
  GET_CLASS_NAME(Multiset)

private:
  const std::unordered_map<T, int> map_obj = std::unordered_map<T, int>();
  // C++ NOTE: Fields cannot have the same name as methods:
  int size_ = 0;

  /** Create an empty {@link Multiset}. */
public:
  Multiset() : AbstractCollection<T>() {}

  std::shared_ptr<Iterator<T>> iterator() override
  {
    constexpr std::unordered_map<T, int>::const_iterator mapIterator =
        map_obj.begin();
    return std::make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                         mapIterator);
  }

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<T>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<Multiset<std::shared_ptr<T>>> outerInstance;

    std::unordered_map<std::shared_ptr<T>, int>::const_iterator mapIterator;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<Multiset<std::shared_ptr<T>>> outerInstance,
        std::unordered_map<T, int>::const_iterator mapIterator);

    std::shared_ptr<T> current;
    int remaining = 0;

    bool hasNext();

    std::shared_ptr<T> next();
  };

public:
  int size() override { return size_; }

  void clear() override
  {
    map_obj.clear();
    size_ = 0;
  }

  bool add(T e) override
  {
    map_obj.emplace(e, map_obj.getOrDefault(e, 0) + 1);
    size_ += 1;
    return true;
  }

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public bool
  // remove(Object o)
  bool remove(std::any o) override
  {
    const std::optional<int> count = map_obj[o];
    if (!count) {
      return false;
    } else if (1 == count.value()) {
      map_obj.erase(o);
    } else {
      map_obj.emplace(std::any_cast<T>(o), count - 1);
    }
    size_ -= 1;
    return true;
  }

  bool contains(std::any o) override { return map_obj.find(o) != map_obj.end(); }

  bool equals(std::any obj) override
  {
    if (obj == nullptr || obj.type() != getClass()) {
      return false;
    }
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Multiset<?> that = (Multiset<?>) obj;
    std::shared_ptr < Multiset < ? >> that =
                                       std::any_cast < Multiset < ? >> (obj);
    return size_ == that->size_ && map_obj.equals(that->map_obj);
  }

  int hashCode() override
  {
    return 31 * getClass().hashCode() + map_obj.hashCode();
  }

protected:
  std::shared_ptr<Multiset> shared_from_this()
  {
    return std::static_pointer_cast<Multiset>(
        java.util.AbstractCollection<T>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
