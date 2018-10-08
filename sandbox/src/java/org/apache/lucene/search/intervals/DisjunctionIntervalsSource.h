#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::intervals
{
class IntervalsSource;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class DisiPriorityQueue;
}
namespace org::apache::lucene::search::intervals
{
class DisiPriorityQueue;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

class DisjunctionIntervalsSource : public IntervalsSource
{
  GET_CLASS_NAME(DisjunctionIntervalsSource)

public:
  const std::deque<std::shared_ptr<IntervalsSource>> subSources;

  DisjunctionIntervalsSource(
      std::deque<std::shared_ptr<IntervalsSource>> &subSources);

  std::shared_ptr<IntervalIterator>
  intervals(const std::wstring &field,
            std::shared_ptr<LeafReaderContext> ctx)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

  void extractTerms(const std::wstring &field,
                    std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

private:
  class DisjunctionIntervalIterator : public IntervalIterator
  {
    GET_CLASS_NAME(DisjunctionIntervalIterator)

  public:
    const std::shared_ptr<DocIdSetIterator> approximation;
    const std::shared_ptr<PriorityQueue<std::shared_ptr<IntervalIterator>>>
        intervalQueue;
    const std::shared_ptr<DisiPriorityQueue> disiQueue;
    const std::deque<std::shared_ptr<IntervalIterator>> iterators;
    // C++ NOTE: Fields cannot have the same name as methods:
    const float matchCost_;

    std::shared_ptr<IntervalIterator> current = EMPTY;

    DisjunctionIntervalIterator(
        std::deque<std::shared_ptr<IntervalIterator>> &iterators);

  public:
    float matchCost() override;

    int start() override;

    int end() override;

  private:
    void reset() ;

  public:
    int nextInterval()  override;

  private:
    bool contains(std::shared_ptr<IntervalIterator> it, int start, int end);

  public:
    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DisjunctionIntervalIterator> shared_from_this()
    {
      return std::static_pointer_cast<DisjunctionIntervalIterator>(
          IntervalIterator::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<IntervalIterator> EMPTY;

private:
  class IntervalIteratorAnonymousInnerClass : public IntervalIterator
  {
    GET_CLASS_NAME(IntervalIteratorAnonymousInnerClass)
  public:
    IntervalIteratorAnonymousInnerClass();

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

    int start() override;

    int end() override;

    int nextInterval() override;

    float matchCost() override;

  protected:
    std::shared_ptr<IntervalIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntervalIteratorAnonymousInnerClass>(
          IntervalIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DisjunctionIntervalsSource> shared_from_this()
  {
    return std::static_pointer_cast<DisjunctionIntervalsSource>(
        IntervalsSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
