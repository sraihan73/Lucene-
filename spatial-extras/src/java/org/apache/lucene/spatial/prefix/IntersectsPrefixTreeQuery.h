#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"

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
using DocIdSet = org::apache::lucene::search::DocIdSet;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A Query matching documents that have an {@link SpatialRelation#INTERSECTS}
 * (i.e. not DISTINCT) relationship with a provided query shape.
 *
 * @lucene.internal
 */
class IntersectsPrefixTreeQuery : public AbstractVisitingPrefixTreeQuery
{
  GET_CLASS_NAME(IntersectsPrefixTreeQuery)

public:
  IntersectsPrefixTreeQuery(std::shared_ptr<Shape> queryShape,
                            const std::wstring &fieldName,
                            std::shared_ptr<SpatialPrefixTree> grid,
                            int detailLevel, int prefixGridScanLevel);

protected:
  std::shared_ptr<DocIdSet> getDocIdSet(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class VisitorTemplateAnonymousInnerClass : public VisitorTemplate
  {
    GET_CLASS_NAME(VisitorTemplateAnonymousInnerClass)
  private:
    std::shared_ptr<IntersectsPrefixTreeQuery> outerInstance;

  public:
    VisitorTemplateAnonymousInnerClass(
        std::shared_ptr<IntersectsPrefixTreeQuery> outerInstance,
        std::shared_ptr<LeafReaderContext> context);

  private:
    std::shared_ptr<DocIdSetBuilder> results;

  protected:
    void start()  override;

    std::shared_ptr<DocIdSet> finish() override;

    bool visitPrefix(std::shared_ptr<Cell> cell)  override;

    void visitLeaf(std::shared_ptr<Cell> cell)  override;

  protected:
    std::shared_ptr<VisitorTemplateAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<VisitorTemplateAnonymousInnerClass>(
          VisitorTemplate::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<IntersectsPrefixTreeQuery> shared_from_this()
  {
    return std::static_pointer_cast<IntersectsPrefixTreeQuery>(
        AbstractVisitingPrefixTreeQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
