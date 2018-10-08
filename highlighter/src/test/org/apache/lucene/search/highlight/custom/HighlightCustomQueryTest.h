#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/InvalidTokenOffsetsException.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTerm.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTermExtractor.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::search::highlight::custom
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using InvalidTokenOffsetsException =
    org::apache::lucene::search::highlight::InvalidTokenOffsetsException;
using QueryScorer = org::apache::lucene::search::highlight::QueryScorer;
using WeightedSpanTerm =
    org::apache::lucene::search::highlight::WeightedSpanTerm;
using WeightedSpanTermExtractor =
    org::apache::lucene::search::highlight::WeightedSpanTermExtractor;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests the extensibility of {@link WeightedSpanTermExtractor} and
 * {@link QueryScorer} in a user defined package
 */
class HighlightCustomQueryTest : public LuceneTestCase
{
  GET_CLASS_NAME(HighlightCustomQueryTest)

private:
  static const std::wstring FIELD_NAME;

public:
  virtual void testHighlightCustomQuery() throw(IOException,
                                                InvalidTokenOffsetsException);

  virtual void testHighlightKnownQuery() ;

private:
  class WeightedSpanTermExtractorAnonymousInnerClass
      : public WeightedSpanTermExtractor
  {
    GET_CLASS_NAME(WeightedSpanTermExtractorAnonymousInnerClass)
  private:
    std::shared_ptr<HighlightCustomQueryTest> outerInstance;

  public:
    WeightedSpanTermExtractorAnonymousInnerClass(
        std::shared_ptr<HighlightCustomQueryTest> outerInstance);

  protected:
    void extractUnknownQuery(
        std::shared_ptr<Query> query,
        std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
            &terms)  override;

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

  /**
   * This method intended for use with
   * <tt>testHighlightingWithDefaultField()</tt>
   */
private:
  std::wstring
  highlightField(std::shared_ptr<Query> query, const std::wstring &fieldName,
                 const std::wstring &text) throw(IOException,
                                                 InvalidTokenOffsetsException);

public:
  class MyWeightedSpanTermExtractor : public WeightedSpanTermExtractor
  {
    GET_CLASS_NAME(MyWeightedSpanTermExtractor)

  public:
    MyWeightedSpanTermExtractor();

    MyWeightedSpanTermExtractor(const std::wstring &defaultField);

  protected:
    void extractUnknownQuery(
        std::shared_ptr<Query> query,
        std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
            &terms)  override;

  protected:
    std::shared_ptr<MyWeightedSpanTermExtractor> shared_from_this()
    {
      return std::static_pointer_cast<MyWeightedSpanTermExtractor>(
          org.apache.lucene.search.highlight
              .WeightedSpanTermExtractor::shared_from_this());
    }
  };

public:
  class MyQueryScorer : public QueryScorer
  {
    GET_CLASS_NAME(MyQueryScorer)

  public:
    MyQueryScorer(std::shared_ptr<Query> query, const std::wstring &field,
                  const std::wstring &defaultField);

  protected:
    std::shared_ptr<WeightedSpanTermExtractor>
    newTermExtractor(const std::wstring &defaultField) override;

  protected:
    std::shared_ptr<MyQueryScorer> shared_from_this()
    {
      return std::static_pointer_cast<MyQueryScorer>(
          org.apache.lucene.search.highlight.QueryScorer::shared_from_this());
    }
  };

public:
  class CustomQuery : public Query
  {
    GET_CLASS_NAME(CustomQuery)
  private:
    const std::shared_ptr<Term> term;

  public:
    CustomQuery(std::shared_ptr<Term> term);

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    virtual int hashCode();

    bool equals(std::any other) override;

  protected:
    std::shared_ptr<CustomQuery> shared_from_this()
    {
      return std::static_pointer_cast<CustomQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<HighlightCustomQueryTest> shared_from_this()
  {
    return std::static_pointer_cast<HighlightCustomQueryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/custom/
