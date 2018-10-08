#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BitSet.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"

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
namespace org::apache::lucene::spatial::prefix
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using BitSet = org::apache::lucene::util::BitSet;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Base class for Lucene Queries on SpatialPrefixTree fields.
 * @lucene.internal
 */
class AbstractPrefixTreeQuery : public Query
{
  GET_CLASS_NAME(AbstractPrefixTreeQuery)

protected:
  const std::shared_ptr<Shape> queryShape;
  const std::wstring fieldName;
  const std::shared_ptr<SpatialPrefixTree>
      grid; // not in equals/hashCode since it's implied for a specific field
  const int detailLevel;

public:
  AbstractPrefixTreeQuery(std::shared_ptr<Shape> queryShape,
                          const std::wstring &fieldName,
                          std::shared_ptr<SpatialPrefixTree> grid,
                          int detailLevel);

  virtual bool equals(std::any o);

private:
  bool equalsTo(std::shared_ptr<AbstractPrefixTreeQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<AbstractPrefixTreeQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<AbstractPrefixTreeQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<DocIdSet>
  getDocIdSet(std::shared_ptr<LeafReaderContext> context) = 0;

  /** Holds transient state and docid collecting utility methods as part of
   * traversing a {@link TermsEnum} for a {@link
   * org.apache.lucene.index.LeafReaderContext}. */
public:
  class BaseTermsEnumTraverser
      : public std::enable_shared_from_this<BaseTermsEnumTraverser>
  { // TODO rename to LeafTermsEnumTraverser ?
    GET_CLASS_NAME(BaseTermsEnumTraverser)
  private:
    std::shared_ptr<AbstractPrefixTreeQuery> outerInstance;

    // note: only 'fieldName' (accessed in constructor) keeps this from being a
    // static inner class

  protected:
    const std::shared_ptr<LeafReaderContext> context;
    const int maxDoc;

    const std::shared_ptr<Terms> terms;         // maybe null
    const std::shared_ptr<TermsEnum> termsEnum; // remember to check for null!
    std::shared_ptr<PostingsEnum> postingsEnum;

  public:
    BaseTermsEnumTraverser(
        std::shared_ptr<AbstractPrefixTreeQuery> outerInstance,
        std::shared_ptr<LeafReaderContext> context) ;

  protected:
    virtual void collectDocs(std::shared_ptr<BitSet> bitSet) ;

    virtual void collectDocs(
        std::shared_ptr<DocIdSetBuilder> docSetBuilder) ;
  };

protected:
  std::shared_ptr<AbstractPrefixTreeQuery> shared_from_this()
  {
    return std::static_pointer_cast<AbstractPrefixTreeQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
