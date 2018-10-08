#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <type_traits>

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
 * An {@link Iterator} implementation that filters elements with a bool
 * predicate.
 *
 * @param <T> generic parameter for this iterator instance: this iterator
 * implements {@link Iterator Iterator&lt;T&gt;}
 * @param <InnerT> generic parameter of the wrapped iterator, must be <tt>T</tt>
 * or extend <tt>T</tt>
 * @see #predicateFunction
 * @lucene.internal
 */
template <typename T, typename InnerT>
class FilterIterator : public std::enable_shared_from_this<FilterIterator>,
                       public Iterator<T>
{
  GET_CLASS_NAME(FilterIterator)
  static_assert(std::is_base_of<T, InnerT>::value,
                L"InnerT must inherit from T");

private:
  const std::shared_ptr<Iterator<InnerT>> iterator;
  // C++ NOTE: Fields cannot have the same name as methods:
  T next_ = nullptr;
  bool nextIsSet = false;

  /** returns true, if this element should be returned by {@link #next()}. */
protected:
  virtual bool predicateFunction(InnerT object) = 0;

public:
  FilterIterator(std::shared_ptr<Iterator<InnerT>> baseIterator)
      : iterator(baseIterator)
  {
  }

  bool hasNext() override { return nextIsSet || setNext(); }

  T next() override
  {
    if (!hasNext()) {
      // C++ TODO: The following line could not be converted:
      throw java.util.NoSuchElementException();
    }
    assert(nextIsSet);
    try {
      return next_;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nextIsSet = false;
      next_ = nullptr;
    }
  }

  void remove() override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

private:
  bool setNext()
  {
    while (iterator->hasNext()) {
      constexpr InnerT object = iterator->next();
      if (predicateFunction(object)) {
        next_ = object;
        nextIsSet = true;
        return true;
      }
      iterator++;
    }
    return false;
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
