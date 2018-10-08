#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/join/GenericTermsCollector.h"
#include  "core/src/java/org/apache/lucene/index/OrdinalMap.h"

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
namespace org::apache::lucene::search::join
{

using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/**
 * Utility for query time joining.
 *
 * @lucene.experimental
 */
class JoinUtil final : public std::enable_shared_from_this<JoinUtil>
{
  GET_CLASS_NAME(JoinUtil)

  // No instances allowed
private:
  JoinUtil();

  /**
   * Method for query time joining.
   * <p>
   * Execute the returned query with a {@link IndexSearcher} to retrieve all
   * documents that have the same terms in the to field that match with
   * documents matching the specified fromQuery and have the same terms in the
   * from field. <p> In the case a single document relates to more than one
   * document the <code>multipleValuesPerDocument</code> option should be set to
   * true. When the <code>multipleValuesPerDocument</code> is set to
   * <code>true</code> only the the score from the first encountered join value
   * originating from the 'from' side is mapped into the 'to' side. Even in the
   * case when a second join value related to a specific document yields a
   * higher score. Obviously this doesn't apply in the case that {@link
   * ScoreMode#None} is used, since no scores are computed at all. <p> Memory
   * considerations: During joining all unique join values are kept in memory.
   * On top of that when the scoreMode isn't set to {@link ScoreMode#None} a
   * float value per unique join value is kept in memory for computing scores.
   * When scoreMode is set to {@link ScoreMode#Avg} also an additional integer
   * value is kept in memory per unique join value.
   *
   * @param fromField                 The from field to join from
   * @param multipleValuesPerDocument Whether the from field has multiple terms
   * per document
   * @param toField                   The to field to join to
   * @param fromQuery                 The query to match documents on the from
   * side
   * @param fromSearcher              The searcher that executed the specified
   * fromQuery
   * @param scoreMode                 Instructs how scores from the fromQuery
   * are mapped to the returned query
   * @return a {@link Query} instance that can be used to join documents based
   * on the terms in the from and to field
   * @throws IOException If I/O related errors occur
   */
public:
  static std::shared_ptr<Query>
  createJoinQuery(const std::wstring &fromField, bool multipleValuesPerDocument,
                  const std::wstring &toField, std::shared_ptr<Query> fromQuery,
                  std::shared_ptr<IndexSearcher> fromSearcher,
                  ScoreMode scoreMode) ;

  /**
   * Method for query time joining for numeric fields. It supports multi- and
   * single- values longs, ints, floats and longs. All considerations from
   * {@link JoinUtil#createJoinQuery(std::wstring, bool, std::wstring, Query,
   * IndexSearcher, ScoreMode)} are applicable here too, though memory
   * consumption might be higher. <p>
   *
   * @param fromField                 The from field to join from
   * @param multipleValuesPerDocument Whether the from field has multiple terms
   * per document when true fromField might be {@link
   * DocValuesType#SORTED_NUMERIC}, otherwise fromField should be {@link
   * DocValuesType#NUMERIC}
   * @param toField                   The to field to join to, should be {@link
   * IntPoint}, {@link LongPoint}, {@link FloatPoint} or {@link DoublePoint}.
   * @param numericType               either {@link java.lang.Integer}, {@link
   * java.lang.Long}, {@link java.lang.Float} or {@link java.lang.Double} it
   * should correspond to toField types
   * @param fromQuery                 The query to match documents on the from
   * side
   * @param fromSearcher              The searcher that executed the specified
   * fromQuery
   * @param scoreMode                 Instructs how scores from the fromQuery
   * are mapped to the returned query
   * @return a {@link Query} instance that can be used to join documents based
   * on the terms in the from and to field
   * @throws IOException If I/O related errors occur
   */
  static std::shared_ptr<Query>
  createJoinQuery(const std::wstring &fromField, bool multipleValuesPerDocument,
                  const std::wstring &toField, std::type_info numericType,
                  std::shared_ptr<Query> fromQuery,
                  std::shared_ptr<IndexSearcher> fromSearcher,
                  ScoreMode scoreMode) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::wstring fromField;
    std::set<int64_t> joinValues;
    bool needsScore = false;
    std::function<void(int64_t, float)> scoreAggregator;

  public:
    internal std::shared_ptr<public> SimpleCollectorAnonymousInnerClass(
        const std::wstring &fromField, std::set<int64_t> &joinValues,
        bool needsScore,
        std::function<void(int64_t, float)> &scoreAggregator);

    std::shared_ptr<SortedNumericDocValues> sortedNumericDocValues;
    std::shared_ptr<Scorer> scorer;

    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::wstring fromField;
    std::set<int64_t> joinValues;
    bool needsScore = false;
    std::function<void(int64_t, float)> scoreAggregator;

  public:
    internal std::shared_ptr<public> SimpleCollectorAnonymousInnerClass2(
        const std::wstring &fromField, std::set<int64_t> &joinValues,
        bool needsScore,
        std::function<void(int64_t, float)> &scoreAggregator);

    std::shared_ptr<NumericDocValues> numericDocValues;
    std::shared_ptr<Scorer> scorer;

  private:
    int lastDocID = -1;

    bool docsInOrder(int docID);

  public:
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

private:
  class StreamAnonymousInnerClass : public PointInSetIncludingScoreQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    bool needsScore = false;
    std::function<float(int64_t)> joinScorer;
    std::set<int64_t>::const_iterator iterator;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(bool needsScore,
                              std::function<float(int64_t)> &joinScorer,
                              std::set<int64_t>::const_iterator iterator,
                              std::shared_ptr<BytesRef> encoded);

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass>(
          PointInSetIncludingScoreQuery.Stream::shared_from_this());
    }
  };

private:
  class StreamAnonymousInnerClass2
      : public PointInSetIncludingScoreQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass2)
  private:
    bool needsScore = false;
    std::function<float(int64_t)> joinScorer;
    std::set<int64_t>::const_iterator iterator;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass2(bool needsScore,
                               std::function<float(int64_t)> &joinScorer,
                               std::set<int64_t>::const_iterator iterator,
                               std::shared_ptr<BytesRef> encoded);

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass2>(
          PointInSetIncludingScoreQuery.Stream::shared_from_this());
    }
  };

private:
  class StreamAnonymousInnerClass3
      : public PointInSetIncludingScoreQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass3)
  private:
    bool needsScore = false;
    std::function<float(int64_t)> joinScorer;
    std::set<int64_t>::const_iterator iterator;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass3(bool needsScore,
                               std::function<float(int64_t)> &joinScorer,
                               std::set<int64_t>::const_iterator iterator,
                               std::shared_ptr<BytesRef> encoded);

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass3>(
          PointInSetIncludingScoreQuery.Stream::shared_from_this());
    }
  };

private:
  class StreamAnonymousInnerClass4
      : public PointInSetIncludingScoreQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass4)
  private:
    bool needsScore = false;
    std::function<float(int64_t)> joinScorer;
    std::set<int64_t>::const_iterator iterator;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass4(bool needsScore,
                               std::function<float(int64_t)> &joinScorer,
                               std::set<int64_t>::const_iterator iterator,
                               std::shared_ptr<BytesRef> encoded);

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass4>(
          PointInSetIncludingScoreQuery.Stream::shared_from_this());
    }
  };

private:
  class PointInSetIncludingScoreQueryAnonymousInnerClass
      : public PointInSetIncludingScoreQuery
  {
    GET_CLASS_NAME(PointInSetIncludingScoreQueryAnonymousInnerClass)
  private:
    std::type_info numericType;

  public:
    PointInSetIncludingScoreQueryAnonymousInnerClass(
        org::apache::lucene::search::join::ScoreMode scoreMode,
        std::shared_ptr<Query> fromQuery, bool multipleValuesPerDocument,
        const std::wstring &toField, std::type_info numericType);

  protected:
    std::wstring toString(std::deque<char> &value) override;

  protected:
    std::shared_ptr<PointInSetIncludingScoreQueryAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PointInSetIncludingScoreQueryAnonymousInnerClass>(
          PointInSetIncludingScoreQuery::shared_from_this());
    }
  };

private:
  class PointInSetQueryAnonymousInnerClass : public PointInSetQuery
  {
    GET_CLASS_NAME(PointInSetQueryAnonymousInnerClass)
  private:
    std::type_info numericType;

  public:
    PointInSetQueryAnonymousInnerClass(const std::wstring &toField,
                                       std::type_info numericType);

  protected:
    std::wstring toString(std::deque<char> &value) override;

  protected:
    std::shared_ptr<PointInSetQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointInSetQueryAnonymousInnerClass>(
          org.apache.lucene.search.PointInSetQuery::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Query> createJoinQuery(
      bool multipleValuesPerDocument, const std::wstring &toField,
      std::shared_ptr<Query> fromQuery, const std::wstring &fromField,
      std::shared_ptr<IndexSearcher> fromSearcher, ScoreMode scoreMode,
      std::shared_ptr<GenericTermsCollector> collector) ;

  /**
   * Delegates to {@link #createJoinQuery(std::wstring, Query, Query, IndexSearcher,
   * ScoreMode, OrdinalMap, int, int)}, but disables the min and max filtering.
   *
   * @param joinField   The {@link SortedDocValues} field containing the join
   * values
   * @param fromQuery   The query containing the actual user query. Also the
   * fromQuery can only match "from" documents.
   * @param toQuery     The query identifying all documents on the "to" side.
   * @param searcher    The index searcher used to execute the from query
   * @param scoreMode   Instructs how scores from the fromQuery are mapped to
   * the returned query
   * @param ordinalMap  The ordinal map_obj constructed over the joinField. In case
   * of a single segment index, no ordinal map_obj needs to be provided.
   * @return a {@link Query} instance that can be used to join documents based
   * on the join field
   * @throws IOException If I/O related errors occur
   */
public:
  static std::shared_ptr<Query>
  createJoinQuery(const std::wstring &joinField,
                  std::shared_ptr<Query> fromQuery,
                  std::shared_ptr<Query> toQuery,
                  std::shared_ptr<IndexSearcher> searcher, ScoreMode scoreMode,
                  std::shared_ptr<OrdinalMap> ordinalMap) ;

  /**
   * A query time join using global ordinals over a dedicated join field.
   *
   * This join has certain restrictions and requirements:
   * 1) A document can only refer to one other document. (but can be referred by
   * one or more documents) 2) Documents on each side of the join must be
   * distinguishable. Typically this can be done by adding an extra field that
   * identifies the "from" and "to" side and then the fromQuery and toQuery must
   * take the this into account. 3) There must be a single sorted doc values
   * join field used by both the "from" and "to" documents. This join field
   *    should store the join values as UTF-8 strings.
   * 4) An ordinal map_obj must be provided that is created on top of the join
   * field.
   *
   * Note: min and max filtering and the avg score mode will require this join
   * to keep track of the number of times a document matches per join value.
   * This will increase the per join cost in terms of execution time and memory.
   *
   * @param joinField   The {@link SortedDocValues} field containing the join
   * values
   * @param fromQuery   The query containing the actual user query. Also the
   * fromQuery can only match "from" documents.
   * @param toQuery     The query identifying all documents on the "to" side.
   * @param searcher    The index searcher used to execute the from query
   * @param scoreMode   Instructs how scores from the fromQuery are mapped to
   * the returned query
   * @param ordinalMap  The ordinal map_obj constructed over the joinField. In case
   * of a single segment index, no ordinal map_obj needs to be provided.
   * @param min         Optionally the minimum number of "from" documents that
   * are required to match for a "to" document to be a match. The min is
   * inclusive. Setting min to 0 and max to <code>Interger.MAX_VALUE</code>
   *                    disables the min and max "from" documents filtering
   * @param max         Optionally the maximum number of "from" documents that
   * are allowed to match for a "to" document to be a match. The max is
   * inclusive. Setting min to 0 and max to <code>Interger.MAX_VALUE</code>
   *                    disables the min and max "from" documents filtering
   * @return a {@link Query} instance that can be used to join documents based
   * on the join field
   * @throws IOException If I/O related errors occur
   */
  static std::shared_ptr<Query> createJoinQuery(
      const std::wstring &joinField, std::shared_ptr<Query> fromQuery,
      std::shared_ptr<Query> toQuery, std::shared_ptr<IndexSearcher> searcher,
      ScoreMode scoreMode, std::shared_ptr<OrdinalMap> ordinalMap, int min,
      int max) ;
};

} // #include  "core/src/java/org/apache/lucene/search/join/
