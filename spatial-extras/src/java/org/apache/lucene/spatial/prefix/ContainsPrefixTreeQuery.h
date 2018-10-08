#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DocIdSet;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::spatial::prefix
{
class SmallDocSet;
}
namespace org::apache::lucene::util
{
class SentinelIntSet;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::spatial::prefix
{

using org::locationtech::spatial4j::shape::Shape;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using SentinelIntSet = org::apache::lucene::util::SentinelIntSet;

/**
 * Finds docs where its indexed shape {@link
 * org.apache.lucene.spatial.query.SpatialOperation#Contains CONTAINS} the query
 * shape. For use on {@link RecursivePrefixTreeStrategy}.
 *
 * @lucene.experimental
 */
class ContainsPrefixTreeQuery : public AbstractPrefixTreeQuery
{
  GET_CLASS_NAME(ContainsPrefixTreeQuery)

  /**
   * If the spatial data for a document is comprised of multiple overlapping or
   * adjacent parts, it might fail to match a query shape when doing the
   * CONTAINS predicate when the sum of those shapes contain the query shape but
   * none do individually.  Set this to false to increase performance if you
   * don't care about that circumstance (such as if your indexed data doesn't
   * even have such conditions).  See LUCENE-5062.
   */
protected:
  const bool multiOverlappingIndexedShapes;

public:
  ContainsPrefixTreeQuery(std::shared_ptr<Shape> queryShape,
                          const std::wstring &fieldName,
                          std::shared_ptr<SpatialPrefixTree> grid,
                          int detailLevel, bool multiOverlappingIndexedShapes);

  virtual bool equals(std::any o);

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<DocIdSet> getDocIdSet(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class ContainsVisitor : public BaseTermsEnumTraverser
  {
    GET_CLASS_NAME(ContainsVisitor)
  private:
    std::shared_ptr<ContainsPrefixTreeQuery> outerInstance;

  public:
    ContainsVisitor(
        std::shared_ptr<ContainsPrefixTreeQuery> outerInstance,
        std::shared_ptr<LeafReaderContext> context) ;

    std::shared_ptr<BytesRef> seekTerm =
        std::make_shared<BytesRef>();   // temp; see seek()
    std::shared_ptr<BytesRef> thisTerm; // current term in termsEnum
    std::shared_ptr<Cell> indexedCell;  // the cell wrapper around thisTerm

    /** This is the primary algorithm; recursive.  Returns null if finds none.
     */
  private:
    std::shared_ptr<SmallDocSet>
    visit(std::shared_ptr<Cell> cell,
          std::shared_ptr<Bits> acceptContains) ;

    bool seek(std::shared_ptr<Cell> cell) ;

    /** Get prefix & leaf docs at this cell. */
    std::shared_ptr<SmallDocSet>
    getDocs(std::shared_ptr<Cell> cell,
            std::shared_ptr<Bits> acceptContains) ;

    /** Gets docs on the leaf of the given cell, _if_ there is a leaf cell,
     * otherwise null. */
    std::shared_ptr<SmallDocSet>
    getLeafDocs(std::shared_ptr<Cell> cell,
                std::shared_ptr<Bits> acceptContains) ;

    bool nextTerm() ;

    std::shared_ptr<SmallDocSet> union_(std::shared_ptr<SmallDocSet> aSet,
                                        std::shared_ptr<SmallDocSet> bSet);

    std::shared_ptr<SmallDocSet>
    collectDocs(std::shared_ptr<Bits> acceptContains) ;

  protected:
    std::shared_ptr<ContainsVisitor> shared_from_this()
    {
      return std::static_pointer_cast<ContainsVisitor>(
          BaseTermsEnumTraverser::shared_from_this());
    }
  }; // class ContainsVisitor

  /** A hash based mutable set of docIds. If this were Solr code then we might
   * use a combination of HashDocSet and SortedIntDocSet instead. */
  // TODO use DocIdSetBuilder?
private:
  class SmallDocSet : public DocIdSet, public Bits
  {
    GET_CLASS_NAME(SmallDocSet)

  private:
    const std::shared_ptr<SentinelIntSet> intSet;
    int maxInt = 0;

  public:
    SmallDocSet(int size);

    bool get(int index) override;

    virtual void set(int index);

    /** Largest docid. */
    int length() override;

    /** Number of docids. */
    virtual int size();

    /** NOTE: modifies and returns either "this" or "other" */
    virtual std::shared_ptr<SmallDocSet>
    union_(std::shared_ptr<SmallDocSet> other);

    std::shared_ptr<Bits> bits()  override;

    std::shared_ptr<DocIdSetIterator> iterator()  override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<SmallDocSet> outerInstance;

      std::deque<int> docs;
      int size = 0;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<SmallDocSet> outerInstance, std::deque<int> &docs,
          int size);

      int idx = 0;
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            org.apache.lucene.search.DocIdSetIterator::shared_from_this());
      }
    };

  public:
    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<SmallDocSet> shared_from_this()
    {
      return std::static_pointer_cast<SmallDocSet>(
          org.apache.lucene.search.DocIdSet::shared_from_this());
    }
  }; // class SmallDocSet

protected:
  std::shared_ptr<ContainsPrefixTreeQuery> shared_from_this()
  {
    return std::static_pointer_cast<ContainsPrefixTreeQuery>(
        AbstractPrefixTreeQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
