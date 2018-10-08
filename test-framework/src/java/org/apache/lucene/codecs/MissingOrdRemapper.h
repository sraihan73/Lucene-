#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * a utility class to write missing values for SORTED as if they were the empty
 * string (to simulate pre-Lucene4.5 dv behavior for testing old codecs)
 */
class MissingOrdRemapper
    : public std::enable_shared_from_this<MissingOrdRemapper>
{
  GET_CLASS_NAME(MissingOrdRemapper)

  /** insert an empty byte[] to the front of this iterable */
public:
  static std::deque<std::shared_ptr<BytesRef>>
  insertEmptyValue(std::deque<std::shared_ptr<BytesRef>> &iterable);

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<BytesRef>> iterable;

  public:
    IterableAnonymousInnerClass(
        std::deque<std::shared_ptr<BytesRef>> &iterable);

    std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<BytesRef>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance);

      bool seenEmpty = false;
      std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> in_;

      bool hasNext();

      std::shared_ptr<BytesRef> next();

      void remove();
    };
  };

  /** remaps ord -1 to ord 0 on this iterable. */
public:
  static std::deque<std::shared_ptr<Number>>
  mapMissingToOrd0(std::deque<std::shared_ptr<Number>> &iterable);

private:
  class IterableAnonymousInnerClass2
      : public std::enable_shared_from_this<IterableAnonymousInnerClass2>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass2)
  private:
    std::deque<std::shared_ptr<Number>> iterable;

  public:
    IterableAnonymousInnerClass2(
        std::deque<std::shared_ptr<Number>> &iterable);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass2
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass2>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass2)
    private:
      std::shared_ptr<IterableAnonymousInnerClass2> outerInstance;

    public:
      IteratorAnonymousInnerClass2(
          std::shared_ptr<IterableAnonymousInnerClass2> outerInstance);

      std::shared_ptr<Iterator<std::shared_ptr<Number>>> in_;

      bool hasNext();

      std::shared_ptr<Number> next();

      void remove();
    };
  };

  /** remaps every ord+1 on this iterable */
public:
  static std::deque<std::shared_ptr<Number>>
  mapAllOrds(std::deque<std::shared_ptr<Number>> &iterable);

private:
  class IterableAnonymousInnerClass3
      : public std::enable_shared_from_this<IterableAnonymousInnerClass3>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass3)
  private:
    std::deque<std::shared_ptr<Number>> iterable;

  public:
    IterableAnonymousInnerClass3(
        std::deque<std::shared_ptr<Number>> &iterable);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass3
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass3>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass3)
    private:
      std::shared_ptr<IterableAnonymousInnerClass3> outerInstance;

    public:
      IteratorAnonymousInnerClass3(
          std::shared_ptr<IterableAnonymousInnerClass3> outerInstance);

      std::shared_ptr<Iterator<std::shared_ptr<Number>>> in_;

      bool hasNext();

      std::shared_ptr<Number> next();

      void remove();
    };
  };
};

} // namespace org::apache::lucene::codecs
