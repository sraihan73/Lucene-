#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/intervals/IntervalIterator.h"

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

/**
 * A function that takes two interval iterators and combines them to produce a
 * third, generally by computing a difference interval between them
 */
class DifferenceIntervalFunction
    : public std::enable_shared_from_this<DifferenceIntervalFunction>
{
  GET_CLASS_NAME(DifferenceIntervalFunction)

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
       * Combine two interval iterators into a third
       */
      virtual std::shared_ptr<IntervalIterator>
      apply(std::shared_ptr<IntervalIterator> minuend,
            std::shared_ptr<IntervalIterator> subtrahend) = 0;

  /**
   * Filters the minuend iterator so that only intervals that do not overlap
   * intervals from the subtrahend iterator are returned
   */
  static const std::shared_ptr<DifferenceIntervalFunction> NON_OVERLAPPING;

private:
  class SingletonFunctionAnonymousInnerClass : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass)
  public:
    SingletonFunctionAnonymousInnerClass();

    std::shared_ptr<IntervalIterator>
    apply(std::shared_ptr<IntervalIterator> minuend,
          std::shared_ptr<IntervalIterator> subtrahend) override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass>(
          SingletonFunction::shared_from_this());
    }
  };

  /**
   * Filters the minuend iterator so that only intervals that do not contain
   * intervals from the subtrahend iterator are returned
   */
public:
  static const std::shared_ptr<DifferenceIntervalFunction> NOT_CONTAINING;

private:
  class SingletonFunctionAnonymousInnerClass2 : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass2)
  public:
    SingletonFunctionAnonymousInnerClass2();

    std::shared_ptr<IntervalIterator>
    apply(std::shared_ptr<IntervalIterator> minuend,
          std::shared_ptr<IntervalIterator> subtrahend) override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass2>(
          SingletonFunction::shared_from_this());
    }
  };

  /**
   * Filters the minuend iterator so that only intervals that are not contained
   * by intervals from the subtrahend iterator are returned
   */
public:
  static const std::shared_ptr<DifferenceIntervalFunction> NOT_CONTAINED_BY;

private:
  class SingletonFunctionAnonymousInnerClass3 : public SingletonFunction
  {
    GET_CLASS_NAME(SingletonFunctionAnonymousInnerClass3)
  public:
    SingletonFunctionAnonymousInnerClass3();

    std::shared_ptr<IntervalIterator>
    apply(std::shared_ptr<IntervalIterator> minuend,
          std::shared_ptr<IntervalIterator> subtrahend) override;

  protected:
    std::shared_ptr<SingletonFunctionAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SingletonFunctionAnonymousInnerClass3>(
          SingletonFunction::shared_from_this());
    }
  };

private:
  class RelativeIterator : public IntervalIterator
  {
    GET_CLASS_NAME(RelativeIterator)

  public:
    const std::shared_ptr<IntervalIterator> a;
    const std::shared_ptr<IntervalIterator> b;

    bool bpos = false;

    RelativeIterator(std::shared_ptr<IntervalIterator> a,
                     std::shared_ptr<IntervalIterator> b);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    virtual void reset() ;

  public:
    int start() override;

    int end() override;

    float matchCost() override;

  protected:
    std::shared_ptr<RelativeIterator> shared_from_this()
    {
      return std::static_pointer_cast<RelativeIterator>(
          IntervalIterator::shared_from_this());
    }
  };

private:
  class NonOverlappingIterator : public RelativeIterator
  {
    GET_CLASS_NAME(NonOverlappingIterator)

  private:
    NonOverlappingIterator(std::shared_ptr<IntervalIterator> minuend,
                           std::shared_ptr<IntervalIterator> subtrahend);

  public:
    int nextInterval()  override;

  protected:
    std::shared_ptr<NonOverlappingIterator> shared_from_this()
    {
      return std::static_pointer_cast<NonOverlappingIterator>(
          RelativeIterator::shared_from_this());
    }
  };

  /**
   * Filters the minuend iterator so that only intervals that do not occur
   * within a set number of positions of intervals from the subtrahend iterator
   * are returned
   */
public:
  class NotWithinFunction;

private:
  class NotContainingIterator : public RelativeIterator
  {
    GET_CLASS_NAME(NotContainingIterator)

  private:
    NotContainingIterator(std::shared_ptr<IntervalIterator> minuend,
                          std::shared_ptr<IntervalIterator> subtrahend);

  public:
    int nextInterval()  override;

  protected:
    std::shared_ptr<NotContainingIterator> shared_from_this()
    {
      return std::static_pointer_cast<NotContainingIterator>(
          RelativeIterator::shared_from_this());
    }
  };

private:
  class NotContainedByIterator : public RelativeIterator
  {
    GET_CLASS_NAME(NotContainedByIterator)

  public:
    NotContainedByIterator(std::shared_ptr<IntervalIterator> a,
                           std::shared_ptr<IntervalIterator> b);

    int nextInterval()  override;

  protected:
    std::shared_ptr<NotContainedByIterator> shared_from_this()
    {
      return std::static_pointer_cast<NotContainedByIterator>(
          RelativeIterator::shared_from_this());
    }
  };

private:
  class SingletonFunction;
};

} // namespace org::apache::lucene::search::intervals
class DifferenceIntervalFunction::NotWithinFunction
    : public DifferenceIntervalFunction
{
  GET_CLASS_NAME(DifferenceIntervalFunction::NotWithinFunction)

private:
  const int positions;

public:
  NotWithinFunction(int positions);

  virtual bool equals(std::any o);

  virtual std::wstring toString();

  virtual int hashCode();

  std::shared_ptr<IntervalIterator>
  apply(std::shared_ptr<IntervalIterator> minuend,
        std::shared_ptr<IntervalIterator> subtrahend) override;

private:
  class IntervalIteratorAnonymousInnerClass : public IntervalIterator
  {
    GET_CLASS_NAME(IntervalIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<NotWithinFunction> outerInstance;

    std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
        subtrahend;

  public:
    IntervalIteratorAnonymousInnerClass(
        std::shared_ptr<NotWithinFunction> outerInstance,
        std::shared_ptr<
            org::apache::lucene::search::intervals::IntervalIterator>
            subtrahend);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

    bool positioned = false;

    int start() override;

    int end() override;

    int nextInterval()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<IntervalIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntervalIteratorAnonymousInnerClass>(
          IntervalIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NotWithinFunction> shared_from_this()
  {
    return std::static_pointer_cast<NotWithinFunction>(
        DifferenceIntervalFunction::shared_from_this());
  }
};
class DifferenceIntervalFunction::SingletonFunction
    : public DifferenceIntervalFunction
{
  GET_CLASS_NAME(DifferenceIntervalFunction::SingletonFunction)

private:
  const std::wstring name;

public:
  SingletonFunction(const std::wstring &name);

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<SingletonFunction> shared_from_this()
  {
    return std::static_pointer_cast<SingletonFunction>(
        DifferenceIntervalFunction::shared_from_this());
  }
};
