#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
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

namespace org::apache::lucene::search::intervals
{

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Combine a deque of {@link IntervalIterator}s into another
 */
class IntervalFunction : public std::enable_shared_from_this<IntervalFunction>
{
  GET_CLASS_NAME(IntervalFunction)

public:
  int hashCode() = 0;
  override

      bool
      equals(std::any obj) = 0;
  override

      std::wstring
      toString() = 0;
  override

      /**
       * Combine the iterators into another iterator
       */
      virtual std::shared_ptr<IntervalIterator>
      apply(std::deque<std::shared_ptr<IntervalIterator>> &iterators) = 0;

  static const std::shared_ptr<IntervalFunction> BLOCK;

private:
  class SingletonFunctionAnonymousInnerClass : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass)
  public:
    SingletonFunctionAnonymousInnerClass();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &iterators) override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass>(
          SingletonFunction::shared_from_this());
    }
  };

private:
  class BlockIntervalIterator : public ConjunctionIntervalIterator
  {
    GET_CLASS_NAME(BlockIntervalIterator)

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    int start_ = -1, end_ = -1;

    BlockIntervalIterator(
        std::deque<std::shared_ptr<IntervalIterator>> &subIterators);

    int start() override;

    int end() override;

    int nextInterval()  override;

  protected:
    void reset() override;

  protected:
    std::shared_ptr<BlockIntervalIterator> shared_from_this()
    {
      return std::static_pointer_cast<BlockIntervalIterator>(
          ConjunctionIntervalIterator::shared_from_this());
    }
  };

  /**
   * Return an iterator over intervals where the subiterators appear in a given
   * order
   */
public:
  static const std::shared_ptr<IntervalFunction> ORDERED;

private:
  class SingletonFunctionAnonymousInnerClass : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass)
  public:
    SingletonFunctionAnonymousInnerClass();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &intervalIterators)
        override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass>(
          SingletonFunction::shared_from_this());
    }
  };

private:
  class OrderedIntervalIterator : public ConjunctionIntervalIterator
  {
    GET_CLASS_NAME(OrderedIntervalIterator)

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    int start_ = -1, end_ = -1, i = 0;

  private:
    OrderedIntervalIterator(
        std::deque<std::shared_ptr<IntervalIterator>> &subIntervals);

  public:
    int start() override;

    int end() override;

    int nextInterval()  override;

  protected:
    void reset()  override;

  protected:
    std::shared_ptr<OrderedIntervalIterator> shared_from_this()
    {
      return std::static_pointer_cast<OrderedIntervalIterator>(
          ConjunctionIntervalIterator::shared_from_this());
    }
  };

  /**
   * Return an iterator over intervals where the subiterators appear in any
   * order
   */
public:
  static const std::shared_ptr<IntervalFunction> UNORDERED;

private:
  class SingletonFunctionAnonymousInnerClass : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass)
  public:
    SingletonFunctionAnonymousInnerClass();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &intervalIterators)
        override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass>(
          SingletonFunction::shared_from_this());
    }
  };

  /**
   * Return an iterator over intervals where the subiterators appear in any
   * order, and do not overlap
   */
public:
  static const std::shared_ptr<IntervalFunction> UNORDERED_NO_OVERLAP;

private:
  class SingletonFunctionAnonymousInnerClass2 : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass2)
  public:
    SingletonFunctionAnonymousInnerClass2();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &iterators) override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass2>(
          SingletonFunction::shared_from_this());
    }
  };

private:
  class UnorderedIntervalIterator : public ConjunctionIntervalIterator
  {
    GET_CLASS_NAME(UnorderedIntervalIterator)

  private:
    const std::shared_ptr<PriorityQueue<std::shared_ptr<IntervalIterator>>>
        queue;
    std::deque<std::shared_ptr<IntervalIterator>> const subIterators;
    const bool allowOverlaps;

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    int start_ = -1, end_ = -1, queueEnd = 0;

    UnorderedIntervalIterator(
        std::deque<std::shared_ptr<IntervalIterator>> &subIterators,
        bool allowOverlaps);

  public:
    int start() override;

    int end() override;

    virtual void updateRightExtreme(std::shared_ptr<IntervalIterator> it);

    int nextInterval()  override;

  protected:
    void reset()  override;

  private:
    bool hasOverlaps(std::shared_ptr<IntervalIterator> candidate);

  protected:
    std::shared_ptr<UnorderedIntervalIterator> shared_from_this()
    {
      return std::static_pointer_cast<UnorderedIntervalIterator>(
          ConjunctionIntervalIterator::shared_from_this());
    }
  };

  /**
   * Returns an interval over iterators where the first iterator contains
   * intervals from the second
   */
public:
  static const std::shared_ptr<IntervalFunction> CONTAINING;

private:
  class SingletonFunctionAnonymousInnerClass : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass)
  public:
    SingletonFunctionAnonymousInnerClass();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &iterators) override;

  private:
    class ConjunctionIntervalIteratorAnonymousInnerClass
        : public ConjunctionIntervalIterator
    {
      GET_CLASS_NAME(ConjunctionIntervalIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<SingletonFunctionAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
          a;
      std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
          b;

    public:
      ConjunctionIntervalIteratorAnonymousInnerClass(
          std::shared_ptr<SingletonFunctionAnonymousInnerClass> outerInstance,
          std::deque<std::shared_ptr<IntervalIterator>> &iterators,
          std::shared_ptr<
              org::apache::lucene::search::intervals::IntervalIterator>
              a,
          std::shared_ptr<
              org::apache::lucene::search::intervals::IntervalIterator>
              b);

      bool bpos = false;

      int start() override;

      int end() override;

      int nextInterval()  override;

    protected:
      void reset()  override;

    protected:
      std::shared_ptr<ConjunctionIntervalIteratorAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            ConjunctionIntervalIteratorAnonymousInnerClass>(
            ConjunctionIntervalIterator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass>(
          SingletonFunction::shared_from_this());
    }
  };

  /**
   * Return an iterator over intervals where the first iterator is contained by
   * intervals from the second
   */
public:
  static const std::shared_ptr<IntervalFunction> CONTAINED_BY;

private:
  class SingletonFunctionAnonymousInnerClass2 : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass2)
  public:
    SingletonFunctionAnonymousInnerClass2();

    std::shared_ptr<IntervalIterator>
    apply(std::deque<std::shared_ptr<IntervalIterator>> &iterators) override;

  private:
    class ConjunctionIntervalIteratorAnonymousInnerClass2
        : public ConjunctionIntervalIterator
    {
      GET_CLASS_NAME(ConjunctionIntervalIteratorAnonymousInnerClass2)
    private:
      std::shared_ptr<SingletonFunctionAnonymousInnerClass2> outerInstance;

      std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
          a;
      std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
          b;

    public:
      ConjunctionIntervalIteratorAnonymousInnerClass2(
          std::shared_ptr<SingletonFunctionAnonymousInnerClass2> outerInstance,
          std::deque<std::shared_ptr<IntervalIterator>> &iterators,
          std::shared_ptr<
              org::apache::lucene::search::intervals::IntervalIterator>
              a,
          std::shared_ptr<
              org::apache::lucene::search::intervals::IntervalIterator>
              b);

      bool bpos = false;

      int start() override;

      int end() override;

      int nextInterval()  override;

    protected:
      void reset()  override;

    protected:
      std::shared_ptr<ConjunctionIntervalIteratorAnonymousInnerClass2>
      shared_from_this()
      {
        return std::static_pointer_cast<
            ConjunctionIntervalIteratorAnonymousInnerClass2>(
            ConjunctionIntervalIterator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass2>(
          SingletonFunction::shared_from_this());
    }
  };

private:
  class SingletonFunction;
};

} // namespace org::apache::lucene::search::intervals
class IntervalFunction::SingletonFunction : public IntervalFunction
{
  GET_CLASS_NAME(IntervalFunction::SingletonFunction)

private:
  const std::wstring name;

protected:
  SingletonFunction(const std::wstring &name);

public:
  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<SingletonFunction> shared_from_this()
  {
    return std::static_pointer_cast<SingletonFunction>(
        IntervalFunction::shared_from_this());
  }
};
