#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search::highlight
{
class WeightedSpanTerm;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::search::uhighlight
{
class OffsetsEnum;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::search::uhighlight
{
class SpanCollectedOffsetsEnum;
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
namespace org::apache::lucene::search::uhighlight
{

using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Helps the {@link FieldOffsetStrategy} with position sensitive queries (e.g.
 * highlight phrases correctly). This is a stateful class holding information
 * about the query, but it can (and is) re-used across highlighting documents.
 * Despite this state, it's immutable after construction.
 *
 * @lucene.internal
 */
// TODO rename to SpanHighlighting ?
class PhraseHelper : public std::enable_shared_from_this<PhraseHelper>
{
  GET_CLASS_NAME(PhraseHelper)

public:
  static const std::shared_ptr<PhraseHelper> NONE;

private:
  const std::wstring fieldName;
  const std::shared_ptr<Set<std::shared_ptr<BytesRef>>>
      positionInsensitiveTerms; // (TermQuery terms)
  const std::shared_ptr<Set<std::shared_ptr<SpanQuery>>> spanQueries;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool willRewrite_;
  const std::function<bool(const std::wstring &)> fieldMatcher;

  /**
   * Constructor.
   * {@code rewriteQueryPred} is an extension hook to override the default
   * choice of
   * {@link WeightedSpanTermExtractor#mustRewriteQuery(SpanQuery)}. By default
   * unknown query types are rewritten, so use this to return {@link
   * Boolean#FALSE} if you know the query doesn't need to be rewritten.
   * Similarly, {@code preExtractRewriteFunction} is also an extension hook for
   * extract to allow different queries to be set before the {@link
   * WeightedSpanTermExtractor}'s extraction is invoked.
   * {@code ignoreQueriesNeedingRewrite} effectively ignores any query clause
   * that needs to be "rewritten", which is usually limited to just a {@link
   * SpanMultiTermQueryWrapper} but could be other custom ones.
   * {@code fieldMatcher} The field name predicate to use for extracting the
   * query part that must be highlighted.
   */
public:
  PhraseHelper(std::shared_ptr<Query> query, const std::wstring &field,
               std::function<bool(const std::wstring &)> &fieldMatcher,
               std::function<bool(SpanQuery *)> &rewriteQueryPred,
               std::function<std::deque<std::shared_ptr<Query>>>
                   &preExtractRewriteFunction,
               bool ignoreQueriesNeedingRewrite);

private:
  class TreeSetAnonymousInnerClass : public std::set<std::shared_ptr<Term>>
  {
    GET_CLASS_NAME(TreeSetAnonymousInnerClass)
  private:
    std::shared_ptr<PhraseHelper> outerInstance;

    std::function<bool(const std::wstring &)> fieldMatcher;

  public:
    TreeSetAnonymousInnerClass(
        std::shared_ptr<PhraseHelper> outerInstance,
        std::function<bool(const std::wstring &)> &fieldMatcher);

    bool add(std::shared_ptr<Term> term) override;

  protected:
    std::shared_ptr<TreeSetAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TreeSetAnonymousInnerClass>(
          java.util.TreeSet<org.apache.lucene.index.Term>::shared_from_this());
    }
  };

private:
  class WeightedSpanTermExtractorAnonymousInnerClass
      : public WeightedSpanTermExtractor
  {
    GET_CLASS_NAME(WeightedSpanTermExtractorAnonymousInnerClass)
  private:
    std::shared_ptr<PhraseHelper> outerInstance;

    std::shared_ptr<Query> query;
    std::function<bool(const std::wstring &)> fieldMatcher;
    std::function<bool(SpanQuery *)> rewriteQueryPred;
    std::function<std::deque<std::shared_ptr<Query>>> preExtractRewriteFunction;
    bool ignoreQueriesNeedingRewrite = false;
    std::deque<bool> mustRewriteHolder;
    std::shared_ptr<Set<std::shared_ptr<Term>>>
        extractPosInsensitiveTermsTarget;

  public:
    WeightedSpanTermExtractorAnonymousInnerClass(
        std::shared_ptr<PhraseHelper> outerInstance, const std::wstring &field,
        std::shared_ptr<Query> query,
        std::function<bool(const std::wstring &)> &fieldMatcher,
        std::function<bool(SpanQuery *)> &rewriteQueryPred,
        std::function<std::deque<std::shared_ptr<Query>>>
            &preExtractRewriteFunction,
        bool ignoreQueriesNeedingRewrite, std::deque<bool> &mustRewriteHolder,
        std::shared_ptr<Set<std::shared_ptr<Term>>>
            extractPosInsensitiveTermsTarget);

    // anonymous constructor

  protected:
    void
    extract(std::shared_ptr<Query> query, float boost,
            std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
                &terms)  override;

    bool isQueryUnsupported(std::type_info clazz) override;

    // called on Query types that are NOT position sensitive, e.g. TermQuery
    void extractWeightedTerms(
        std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
            &terms,
        std::shared_ptr<Query> query, float boost)  override;

    // called on SpanQueries. Some other position-sensitive queries like
    // PhraseQuery are converted beforehand
    void extractWeightedSpanTerms(
        std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
            &terms,
        std::shared_ptr<SpanQuery> spanQuery,
        float boost)  override;

    bool mustRewriteQuery(std::shared_ptr<SpanQuery> spanQuery) override;

  protected:
    std::shared_ptr<WeightedSpanTermExtractorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          WeightedSpanTermExtractorAnonymousInnerClass>(
          org.apache.lucene.search.highlight
              .WeightedSpanTermExtractor::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<Set<std::shared_ptr<SpanQuery>>> getSpanQueries();

  /**
   * If there is no position sensitivity then use of the instance of this class
   * can be ignored.
   */
  virtual bool hasPositionSensitivity();

  /**
   * Rewrite is needed for handling a {@link SpanMultiTermQueryWrapper} (MTQ /
   * wildcards) or some custom things.  When true, the resulting term deque will
   * probably be different than what it was known to be initially.
   */
  virtual bool willRewrite();

  /** Returns the terms that are position-insensitive (sorted). */
  virtual std::deque<std::shared_ptr<BytesRef>>
  getAllPositionInsensitiveTerms();

  /** Given the internal SpanQueries, produce a number of OffsetsEnum into the
   * {@code results} param. */
  virtual void createOffsetsEnumsForSpans(
      std::shared_ptr<LeafReader> leafReader, int docId,
      std::deque<std::shared_ptr<OffsetsEnum>> &results) ;

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<Spans>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  private:
    std::shared_ptr<PhraseHelper> outerInstance;

  public:
    PriorityQueueAnonymousInnerClass(
        std::shared_ptr<PhraseHelper> outerInstance,
        std::shared_ptr<UnknownType> size);

  protected:
    bool lessThan(std::shared_ptr<Spans> a, std::shared_ptr<Spans> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<
              org.apache.lucene.search.spans.Spans>::shared_from_this());
    }
  };

  /**
   * Needed to support the ability to highlight a query irrespective of the
   * field a query refers to (aka requireFieldMatch=false). This reader will
   * just delegate every call to a single field in the wrapped LeafReader. This
   * way we ensure that all queries going through this reader target the same
   * field.
   */
private:
  class SingleFieldWithOffsetsFilterLeafReader final : public FilterLeafReader
  {
    GET_CLASS_NAME(SingleFieldWithOffsetsFilterLeafReader)
  public:
    const std::wstring fieldName;

    SingleFieldWithOffsetsFilterLeafReader(std::shared_ptr<LeafReader> in_,
                                           const std::wstring &fieldName);

    std::shared_ptr<FieldInfos> getFieldInfos() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  private:
    class FilterTermsAnonymousInnerClass : public FilterTerms
    {
      GET_CLASS_NAME(FilterTermsAnonymousInnerClass)
    private:
      std::shared_ptr<SingleFieldWithOffsetsFilterLeafReader> outerInstance;

    public:
      FilterTermsAnonymousInnerClass(
          std::shared_ptr<SingleFieldWithOffsetsFilterLeafReader> outerInstance,
          std::shared_ptr<Terms> terms);

      std::shared_ptr<TermsEnum> iterator()  override;

    private:
      class FilterTermsEnumAnonymousInnerClass : public FilterTermsEnum
      {
        GET_CLASS_NAME(FilterTermsEnumAnonymousInnerClass)
      private:
        std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance;

      public:
        FilterTermsEnumAnonymousInnerClass(
            std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
            std::shared_ptr<UnknownType> iterator);

        std::shared_ptr<PostingsEnum>
        postings(std::shared_ptr<PostingsEnum> reuse,
                 int flags)  override;

      protected:
        std::shared_ptr<FilterTermsEnumAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterTermsEnumAnonymousInnerClass>(
              FilterTermsEnum::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<FilterTermsAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterTermsAnonymousInnerClass>(
            FilterTerms::shared_from_this());
      }
    };

  public:
    std::shared_ptr<NumericDocValues>
    getNormValues(const std::wstring &field)  override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<SingleFieldWithOffsetsFilterLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<SingleFieldWithOffsetsFilterLeafReader>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

private:
  class OffsetSpanCollector
      : public std::enable_shared_from_this<OffsetSpanCollector>,
        public SpanCollector
  {
    GET_CLASS_NAME(OffsetSpanCollector)
  private:
    std::shared_ptr<PhraseHelper> outerInstance;

  public:
    OffsetSpanCollector(std::shared_ptr<PhraseHelper> outerInstance);

    std::unordered_map<std::shared_ptr<BytesRef>,
                       std::shared_ptr<SpanCollectedOffsetsEnum>>
        termToOffsetsEnums =
            std::unordered_map<std::shared_ptr<BytesRef>,
                               std::shared_ptr<SpanCollectedOffsetsEnum>>();

    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    void reset() override;
  };

private:
  class SpanCollectedOffsetsEnum : public OffsetsEnum
  {
    GET_CLASS_NAME(SpanCollectedOffsetsEnum)
    // TODO perhaps optionally collect (and expose) payloads?
  private:
    const std::shared_ptr<BytesRef> term;
    std::deque<int> const startOffsets;
    std::deque<int> const endOffsets;
    int numPairs = 0;
    int enumIdx = -1;

    SpanCollectedOffsetsEnum(std::shared_ptr<BytesRef> term, int postingsFreq);

    // called from collector before it's navigated
  public:
    virtual void add(int startOffset, int endOffset);

    bool nextPosition()  override;

    int freq()  override;

    std::shared_ptr<BytesRef> getTerm()  override;

    int startOffset()  override;

    int endOffset()  override;

  protected:
    std::shared_ptr<SpanCollectedOffsetsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SpanCollectedOffsetsEnum>(
          OffsetsEnum::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::search::uhighlight
