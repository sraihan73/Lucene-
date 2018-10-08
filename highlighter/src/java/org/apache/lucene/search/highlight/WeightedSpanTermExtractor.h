#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTerm.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheHelper.h"

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
namespace org::apache::lucene::search::highlight
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Terms = org::apache::lucene::index::Terms;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * Class used to extract {@link WeightedSpanTerm}s from a {@link Query} based on
 * whether
 * {@link Term}s from the {@link Query} are contained in a supplied {@link
 * TokenStream}.
 */
class WeightedSpanTermExtractor
    : public std::enable_shared_from_this<WeightedSpanTermExtractor>
{
  GET_CLASS_NAME(WeightedSpanTermExtractor)

private:
  std::wstring fieldName;
  std::shared_ptr<TokenStream>
      tokenStream; // set subsequent to getWeightedSpanTerms* methods
  std::wstring defaultField;
  bool expandMultiTermQuery = false;
  bool cachedTokenStream = false;
  bool wrapToCaching = true;
  int maxDocCharsToAnalyze = 0;
  bool usePayloads = false;
  std::shared_ptr<LeafReader> internalReader = nullptr;

public:
  WeightedSpanTermExtractor();

  WeightedSpanTermExtractor(const std::wstring &defaultField);

  /**
   * Fills a <code>Map</code> with {@link WeightedSpanTerm}s using the terms
   * from the supplied <code>Query</code>.
   *
   * @param query
   *          Query to extract Terms from
   * @param terms
   *          Map to place created WeightedSpanTerms in
   * @throws IOException If there is a low-level I/O error
   */
protected:
  virtual void
  extract(std::shared_ptr<Query> query, float boost,
          std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
              &terms) ;

  virtual bool isQueryUnsupported(std::type_info clazz);

  virtual void extractUnknownQuery(
      std::shared_ptr<Query> query,
      std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
          &terms) ;

  /**
   * Fills a <code>Map</code> with {@link WeightedSpanTerm}s using the terms
   * from the supplied <code>SpanQuery</code>.
   *
   * @param terms
   *          Map to place created WeightedSpanTerms in
   * @param spanQuery
   *          SpanQuery to extract Terms from
   * @throws IOException If there is a low-level I/O error
   */
  virtual void extractWeightedSpanTerms(
      std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
          &terms,
      std::shared_ptr<SpanQuery> spanQuery, float boost) ;

  /**
   * Fills a <code>Map</code> with {@link WeightedSpanTerm}s using the terms
   * from the supplied <code>Query</code>.
   *
   * @param terms
   *          Map to place created WeightedSpanTerms in
   * @param query
   *          Query to extract Terms from
   * @throws IOException If there is a low-level I/O error
   */
  virtual void extractWeightedTerms(
      std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
          &terms,
      std::shared_ptr<Query> query, float boost) ;

  /**
   * Necessary to implement matches for queries against
   * <code>defaultField</code>
   */
  virtual bool fieldNameComparator(const std::wstring &fieldNameToCheck);

  virtual std::shared_ptr<LeafReaderContext>
  getLeafContext() ;

  /*
   * This reader will just delegate every call to a single field in the wrapped
   * LeafReader. This way we only need to build this field once rather than
   * N-Times
   */
public:
  class DelegatingLeafReader final : public FilterLeafReader
  {
    GET_CLASS_NAME(DelegatingLeafReader)
  private:
    static const std::wstring FIELD_NAME;

  public:
    DelegatingLeafReader(std::shared_ptr<LeafReader> in_);

    std::shared_ptr<FieldInfos> getFieldInfos() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    std::shared_ptr<NumericDocValues>
    getNumericDocValues(const std::wstring &field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinaryDocValues(const std::wstring &field)  override;

    std::shared_ptr<SortedDocValues>
    getSortedDocValues(const std::wstring &field)  override;

    std::shared_ptr<NumericDocValues>
    getNormValues(const std::wstring &field)  override;

    std::shared_ptr<IndexReader::CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DelegatingLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<DelegatingLeafReader>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

  /**
   * Creates a Map of <code>WeightedSpanTerms</code> from the given
   * <code>Query</code> and <code>TokenStream</code>.
   *
   * <p>
   *
   * @param query
   *          that caused hit
   * @param tokenStream
   *          of text to be highlighted
   * @return Map containing WeightedSpanTerms
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
  getWeightedSpanTerms(
      std::shared_ptr<Query> query, float boost,
      std::shared_ptr<TokenStream> tokenStream) ;

  /**
   * Creates a Map of <code>WeightedSpanTerms</code> from the given
   * <code>Query</code> and <code>TokenStream</code>.
   *
   * <p>
   *
   * @param query
   *          that caused hit
   * @param tokenStream
   *          of text to be highlighted
   * @param fieldName
   *          restricts Term's used based on field name
   * @return Map containing WeightedSpanTerms
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
  getWeightedSpanTerms(std::shared_ptr<Query> query, float boost,
                       std::shared_ptr<TokenStream> tokenStream,
                       const std::wstring &fieldName) ;

  /**
   * Creates a Map of <code>WeightedSpanTerms</code> from the given
   * <code>Query</code> and <code>TokenStream</code>. Uses a supplied
   * <code>IndexReader</code> to properly weight terms (for gradient
   * highlighting).
   *
   * <p>
   *
   * @param query
   *          that caused hit
   * @param tokenStream
   *          of text to be highlighted
   * @param fieldName
   *          restricts Term's used based on field name
   * @param reader
   *          to use for scoring
   * @return Map of WeightedSpanTerms with quasi tf/idf scores
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
  getWeightedSpanTermsWithScores(
      std::shared_ptr<Query> query, float boost,
      std::shared_ptr<TokenStream> tokenStream, const std::wstring &fieldName,
      std::shared_ptr<IndexReader> reader) ;

protected:
  virtual void
  collectSpanQueryFields(std::shared_ptr<SpanQuery> spanQuery,
                         std::shared_ptr<Set<std::wstring>> fieldNames);

  virtual bool mustRewriteQuery(std::shared_ptr<SpanQuery> spanQuery);

  /**
   * This class makes sure that if both position sensitive and insensitive
   * versions of the same term are added, the position insensitive one wins.
   */
protected:
  template <typename K>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") protected static class
  // PositionCheckingMap<K> extends java.util.HashMap<K,WeightedSpanTerm>
  class PositionCheckingMap
      : public std::unordered_map<K, std::shared_ptr<WeightedSpanTerm>>
  {
  public:
    template <typename T1, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void putAll(java.util.Map<?
    // extends K,? extends WeightedSpanTerm> m)
    void putAll(std::unordered_map<T1> m)
    {
      for (auto entry : m) {
        this->put(entry.first, entry.second);
      }
    }

    std::shared_ptr<WeightedSpanTerm>
    put(K key, std::shared_ptr<WeightedSpanTerm> value) override
    {
      std::shared_ptr<WeightedSpanTerm> prev =
          HashMap<K, WeightedSpanTerm>::emplace(key, value);
      if (prev == nullptr) {
        return prev;
      }
      std::shared_ptr<WeightedSpanTerm> prevTerm = prev;
      std::shared_ptr<WeightedSpanTerm> newTerm = value;
      if (!prevTerm->positionSensitive) {
        newTerm->positionSensitive = false;
      }
      return prev;
    }

  protected:
    std::shared_ptr<PositionCheckingMap> shared_from_this()
    {
      return std::static_pointer_cast<PositionCheckingMap>(
          java.util.HashMap<K, WeightedSpanTerm>::shared_from_this());
    }
  };

public:
  virtual bool getExpandMultiTermQuery();

  virtual void setExpandMultiTermQuery(bool expandMultiTermQuery);

  virtual bool isUsePayloads();

  virtual void setUsePayloads(bool usePayloads);

  virtual bool isCachedTokenStream();

  /** Returns the tokenStream which may have been wrapped in a
   * CachingTokenFilter. getWeightedSpanTerms* sets the tokenStream, so don't
   * call this before. */
  virtual std::shared_ptr<TokenStream> getTokenStream();

  /**
   * By default, {@link TokenStream}s that are not of the type
   * {@link CachingTokenFilter} are wrapped in a {@link CachingTokenFilter} to
   * ensure an efficient reset - if you are already using a different caching
   * {@link TokenStream} impl and you don't want it to be wrapped, set this to
   * false. This setting is ignored when a term deque based TokenStream is
   * supplied, since it can be reset efficiently.
   */
  virtual void setWrapIfNotCachingTokenFilter(bool wrap);

  /** A threshold of number of characters to analyze. When a TokenStream based
   * on term vectors with offsets and positions are supplied, this setting
   * does not apply. */
protected:
  void setMaxDocCharsToAnalyze(int maxDocCharsToAnalyze);
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
