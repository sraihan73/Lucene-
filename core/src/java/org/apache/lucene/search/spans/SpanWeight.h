#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
}

namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::search::spans
{
class SpanScorer;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search
{
class Explanation;
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
namespace org::apache::lucene::search::spans
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

/**
 * Expert-only.  Public for use by other weight implementations
 */
class SpanWeight : public Weight
{
  GET_CLASS_NAME(SpanWeight)

  /**
   * Enumeration defining what postings information should be retrieved from the
   * index for a given Spans
   */
public:
  class Postings final
  {
    GET_CLASS_NAME(Postings)
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        POSITIONS
    //        {
    //            public int getRequiredPostings()
    //            {
    //                return org.apache.lucene.index.PostingsEnum.POSITIONS;
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        PAYLOADS
    //        {
    //            public int getRequiredPostings()
    //            {
    //                return org.apache.lucene.index.PostingsEnum.PAYLOADS;
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        OFFSETS
    //        {
    //            public int getRequiredPostings()
    //            {
    //                return org.apache.lucene.index.PostingsEnum.PAYLOADS |
    //                org.apache.lucene.index.PostingsEnum.OFFSETS;
    //            }
    //        };

  private:
    static std::deque<Postings> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    Postings(const std::wstring &name, InnerEnum innerEnum);

  public:
    virtual int getRequiredPostings() = 0;

    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        public Postings atLeast(Postings postings)
    //        {
    //            if(postings.compareTo(this) > 0)
    //                return postings;
    //            return this;
    //        }

  public:
    bool operator==(const Postings &other);

    bool operator!=(const Postings &other);

    static std::deque<Postings> values();

    int ordinal();

    std::wstring toString();

    static Postings valueOf(const std::wstring &name);
  };

protected:
  const std::shared_ptr<Similarity> similarity;
  const std::shared_ptr<Similarity::SimWeight> simWeight;
  const std::wstring field;

  /**
   * Create a new SpanWeight
   * @param query the parent query
   * @param searcher the IndexSearcher to query against
   * @param termContexts a map_obj of terms to termcontexts for use in building the
   * similarity.  May be null if scores are not required
   * @throws IOException on error
   */
public:
  SpanWeight(std::shared_ptr<SpanQuery> query,
             std::shared_ptr<IndexSearcher> searcher,
             std::unordered_map<std::shared_ptr<Term>,
                                std::shared_ptr<TermContext>> &termContexts,
             float boost) ;

private:
  std::shared_ptr<Similarity::SimWeight> buildSimWeight(
      std::shared_ptr<SpanQuery> query, std::shared_ptr<IndexSearcher> searcher,
      std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
          &termContexts,
      float boost) ;

  /**
   * Collect all TermContexts used by this Weight
   * @param contexts a map_obj to add the TermContexts to
   */
public:
  virtual void extractTermContexts(
      std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
          &contexts) = 0;

  /**
   * Expert: Return a Spans object iterating over matches from this Weight
   * @param ctx a LeafReaderContext for this Spans
   * @return a Spans
   * @throws IOException on error
   */
  virtual std::shared_ptr<Spans>
  getSpans(std::shared_ptr<LeafReaderContext> ctx,
           Postings requiredPostings) = 0;

  std::shared_ptr<SpanScorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  /**
   * Return a SimScorer for this context
   * @param context the LeafReaderContext
   * @return a SimWeight
   * @throws IOException on error
   */
  virtual std::shared_ptr<Similarity::SimScorer>
  getSimScorer(std::shared_ptr<LeafReaderContext> context) ;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

protected:
  std::shared_ptr<SpanWeight> shared_from_this()
  {
    return std::static_pointer_cast<SpanWeight>(
        org.apache.lucene.search.Weight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
