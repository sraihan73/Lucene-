#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PrefixCodedTerms.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/index/TermIterator.h"
#include  "core/src/java/org/apache/lucene/util/BulkAdder.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

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

using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

/**
 * Abstract query class to find all documents whose single or multi-dimensional
point values, previously indexed with e.g. {@link IntPoint},
 * is contained in the specified set.
 *
 * <p>
 * This is for subclasses and works on the underlying binary encoding: to
 * create range queries for lucene's standard {@code Point} types, refer to
factory GET_CLASS_NAME(es)
 * methods on those classes, e.g. {@link IntPoint#newSetQuery
IntPoint.newSetQuery()} for
 * fields indexed with {@link IntPoint}.
GET_CLASS_NAME(es,)
 * @see PointValues
 * @lucene.experimental */

class PointInSetQuery : public Query
{
  GET_CLASS_NAME(PointInSetQuery)
  // A little bit overkill for us, since all of our "terms" are always in the
  // same field:
public:
  const std::shared_ptr<PrefixCodedTerms> sortedPackedPoints;
  const int sortedPackedPointsHashCode;
  const std::wstring field;
  const int numDims;
  const int bytesPerDim;

  /**
   * Iterator of encoded point values.
   */
  // TODO: if we want to stream, maybe we should use jdk stream class?
public:
  class Stream : public std::enable_shared_from_this<Stream>,
                 public BytesRefIterator
  {
    GET_CLASS_NAME(Stream)
  public:
    std::shared_ptr<BytesRef> next() = 0;
    override
  };

  /** The {@code packedPoints} iterator must be in sorted order. */
protected:
  PointInSetQuery(const std::wstring &field, int numDims, int bytesPerDim,
                  std::shared_ptr<Stream> packedPoints);

public:
  std::shared_ptr<Weight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override final;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<PointInSetQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<PointInSetQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

  /** Essentially does a merge sort, only collecting hits when the indexed point
   * and query point are the same.  This is an optimization, used in the 1D
   * case. */
private:
  class MergePointVisitor
      : public std::enable_shared_from_this<MergePointVisitor>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(MergePointVisitor)
  private:
    std::shared_ptr<PointInSetQuery> outerInstance;

    const std::shared_ptr<DocIdSetBuilder> result;
    std::shared_ptr<PrefixCodedTerms::TermIterator> iterator;
    std::shared_ptr<BytesRef> nextQueryPoint;
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
    const std::shared_ptr<PrefixCodedTerms> sortedPackedPoints;
    std::shared_ptr<DocIdSetBuilder::BulkAdder> adder;

  public:
    MergePointVisitor(
        std::shared_ptr<PointInSetQuery> outerInstance,
        std::shared_ptr<PrefixCodedTerms> sortedPackedPoints,
        std::shared_ptr<DocIdSetBuilder> result) ;

    void grow(int count) override;

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue) override;
  };

  /** IntersectVisitor that queries against a highly degenerate shape: a single
   * point.  This is used in the > 1D case. */
private:
  class SinglePointVisitor
      : public std::enable_shared_from_this<SinglePointVisitor>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(SinglePointVisitor)
  private:
    std::shared_ptr<PointInSetQuery> outerInstance;

    const std::shared_ptr<DocIdSetBuilder> result;
    std::deque<char> const pointBytes;
    std::shared_ptr<DocIdSetBuilder::BulkAdder> adder;

  public:
    SinglePointVisitor(std::shared_ptr<PointInSetQuery> outerInstance,
                       std::shared_ptr<DocIdSetBuilder> result);

    virtual void setPoint(std::shared_ptr<BytesRef> point);

    void grow(int count) override;

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue) override;
  };

public:
  virtual std::shared_ptr<std::deque<std::deque<char>>> getPackedPoints();

private:
  class AbstractCollectionAnonymousInnerClass
      : public AbstractCollection<std::deque<char>>
  {
    GET_CLASS_NAME(AbstractCollectionAnonymousInnerClass)
  private:
    std::shared_ptr<PointInSetQuery> outerInstance;

  public:
    AbstractCollectionAnonymousInnerClass(
        std::shared_ptr<PointInSetQuery> outerInstance);

    std::shared_ptr<Iterator<std::deque<char>>> iterator() override;

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::deque<char>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<AbstractCollectionAnonymousInnerClass> outerInstance;

      int size = 0;
      std::shared_ptr<PrefixCodedTerms::TermIterator> iterator;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<AbstractCollectionAnonymousInnerClass> outerInstance,
          int size, std::shared_ptr<PrefixCodedTerms::TermIterator> iterator);

      int upto = 0;

      bool hasNext();

      std::deque<char> next();
    };

  public:
    int size() override;

  protected:
    std::shared_ptr<AbstractCollectionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AbstractCollectionAnonymousInnerClass>(
          java.util.AbstractCollection<byte[]>::shared_from_this());
    }
  };

public:
  virtual std::wstring getField();

  virtual int getNumDims();

  virtual int getBytesPerDim();

  int hashCode() override final;

  bool equals(std::any other) override final;

private:
  bool equalsTo(std::shared_ptr<PointInSetQuery> other);

public:
  std::wstring toString(const std::wstring &field) override final;

  /**
   * Returns a string of a single value in a human-readable format for
   * debugging. This is used by {@link #toString()}.
   *
   * The default implementation encodes the individual byte values.
   *
   * @param value single value, never null
   * @return human readable value for debugging
   */
protected:
  virtual std::wstring toString(std::deque<char> &value) = 0;

protected:
  std::shared_ptr<PointInSetQuery> shared_from_this()
  {
    return std::static_pointer_cast<PointInSetQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
