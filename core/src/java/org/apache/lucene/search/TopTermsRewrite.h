#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <unordered_map>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class ScoreTerm;
}

namespace org::apache::lucene::search
{
class MaxNonCompetitiveBoostAttribute;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::search
{
class BoostAttribute;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class TermContext;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using TermContext = org::apache::lucene::index::TermContext;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Base rewrite method for collecting only the top terms
 * via a priority queue.
 * @lucene.internal Only public to be accessible by spans package.
 */
template <typename B>
class TopTermsRewrite : public TermCollectingRewrite<B>
{
  GET_CLASS_NAME(TopTermsRewrite)

private:
  const int size;

  /**
   * Create a TopTermsBooleanQueryRewrite for
   * at most <code>size</code> terms.
   * <p>
   * NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than
   * <code>size</code>, then it will be used instead.
   */
public:
  TopTermsRewrite(int size) : size(size) {}

  /** return the maximum priority queue size */
  virtual int getSize() { return size; }

  /** return the maximum size of the priority queue (for bool rewrites this
   * is BooleanQuery#getMaxClauseCount). */
protected:
  virtual int getMaxSize() = 0;

public:
  std::shared_ptr<Query> rewrite(
      std::shared_ptr<IndexReader> reader,
      std::shared_ptr<MultiTermQuery> query)  override final
  {
    constexpr int maxSize = std::min(size, getMaxSize());
    std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> *const stQueue =
        std::make_shared<PriorityQueue<std::shared_ptr<ScoreTerm>>>();
    collectTerms(reader, query,
                 std::make_shared<TermCollectorAnonymousInnerClass>(
                     shared_from_this(), maxSize, stQueue));

    constexpr B b = getTopLevelBuilder();
    std::deque<std::shared_ptr<ScoreTerm>> scoreTerms = stQueue->toArray(
        std::deque<std::shared_ptr<ScoreTerm>>(stQueue->size()));
    ArrayUtil::timSort(scoreTerms, scoreTermSortByTermComp);

    for (auto st : scoreTerms) {
      std::shared_ptr<Term> *const term =
          std::make_shared<Term>(query->field, st->bytes->toBytesRef());
      // We allow negative term scores (fuzzy query does this, for example)
      // while collecting the terms, but truncate such boosts to 0.0f when
      // building the query:
      addClause(b, term, st->termState->docFreq(), std::max(0.0f, st->boost),
                st->termState); // add to query
    }
    return build(b);
  }

private:
  class TermCollectorAnonymousInnerClass : public TermCollector
  {
    GET_CLASS_NAME(TermCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TopTermsRewrite<std::shared_ptr<B>>> outerInstance;

    int maxSize = 0;
    std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> stQueue;

  public:
    TermCollectorAnonymousInnerClass(
        std::shared_ptr<TopTermsRewrite<std::shared_ptr<B>>> outerInstance,
        int maxSize,
        std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> stQueue);

  private:
    const std::shared_ptr<MaxNonCompetitiveBoostAttribute> maxBoostAtt;

    const std::unordered_map<std::shared_ptr<BytesRef>,
                             std::shared_ptr<ScoreTerm>>
        visitedTerms;

    std::shared_ptr<TermsEnum> termsEnum;
    std::shared_ptr<BoostAttribute> boostAtt;
    std::shared_ptr<ScoreTerm> st;

  public:
    void setNextEnum(std::shared_ptr<TermsEnum> termsEnum) override;

    // for assert:
  private:
    std::shared_ptr<BytesRefBuilder> lastTerm;
    bool compareToLastTerm(std::shared_ptr<BytesRef> t);

  public:
    bool collect(std::shared_ptr<BytesRef> bytes)  override;

  protected:
    std::shared_ptr<TermCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TermCollectorAnonymousInnerClass>(
          TermCollector::shared_from_this());
    }
  };

public:
  int hashCode() override { return 31 * size; }

  bool equals(std::any obj) override
  {
    if (shared_from_this() == obj) {
      return true;
    }
    if (obj == nullptr) {
      return false;
    }
    if (getClass() != obj.type()) {
      return false;
    }
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final TopTermsRewrite<?> other = (TopTermsRewrite<?>) obj;
    std::shared_ptr < TopTermsRewrite <
        ? >> *const other = std::any_cast < TopTermsRewrite < ? >> (obj);
    if (size != other->size) {
      return false;
    }
    return true;
  }

private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<Comparator<std::shared_ptr<ScoreTerm>>>
      scoreTermSortByTermComp =
          std::make_shared<ComparatorAnonymousInnerClass>();

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<ScoreTerm>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<ScoreTerm> st1, std::shared_ptr<ScoreTerm> st2);
  };

public:
  class ScoreTerm final : public std::enable_shared_from_this<ScoreTerm>,
                          public Comparable<std::shared_ptr<ScoreTerm>>
  {
    GET_CLASS_NAME(ScoreTerm)
  public:
    const std::shared_ptr<BytesRefBuilder> bytes =
        std::make_shared<BytesRefBuilder>();
    float boost = 0;
    const std::shared_ptr<TermContext> termState;
    ScoreTerm(std::shared_ptr<TermContext> termState);

    int compareTo(std::shared_ptr<ScoreTerm> other) override;
  };

protected:
  std::shared_ptr<TopTermsRewrite> shared_from_this()
  {
    return std::static_pointer_cast<TopTermsRewrite>(
        TermCollectingRewrite<B>::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
