#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial
{
class ShapeValuesSource;
}

namespace org::apache::lucene::spatial::query
{
class SpatialOperation;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::spatial
{
class ShapeValues;
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
namespace org::apache::lucene::spatial::util
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SegmentCacheable = org::apache::lucene::search::SegmentCacheable;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Compares a shape from a provided {@link ShapeValuesSource} with a given Shape
 * and sees if it matches a given {@link SpatialOperation} (the predicate).
 *
 * Consumers should call {@link #iterator(LeafReaderContext, DocIdSetIterator)}
 * to obtain a
 * {@link TwoPhaseIterator} over a particular {@link DocIdSetIterator}.  The
 * initial DocIdSetIterator will be used as the approximation, and the {@link
 * SpatialOperation} comparison will only be performed in {@link
 * TwoPhaseIterator#matches()}
 *
 * @lucene.experimental
 */
class ShapeValuesPredicate
    : public std::enable_shared_from_this<ShapeValuesPredicate>,
      public SegmentCacheable
{
  GET_CLASS_NAME(ShapeValuesPredicate)
private:
  const std::shared_ptr<ShapeValuesSource>
      shapeValuesource; // the left hand side
  const std::shared_ptr<SpatialOperation> op;
  const std::shared_ptr<Shape> queryShape; // the right hand side (constant)

  /**
   *
   * @param shapeValuesource Must yield {@link Shape} instances from its
   * objectVal(doc). If null then the result is false. This is the left-hand
   * (indexed) side.
   * @param op the predicate
   * @param queryShape The shape on the right-hand (query) side.
   */
public:
  ShapeValuesPredicate(std::shared_ptr<ShapeValuesSource> shapeValuesource,
                       std::shared_ptr<SpatialOperation> op,
                       std::shared_ptr<Shape> queryShape);

  virtual std::wstring toString();

  virtual std::shared_ptr<TwoPhaseIterator>
  iterator(std::shared_ptr<LeafReaderContext> ctx,
           std::shared_ptr<DocIdSetIterator> approximation) ;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<ShapeValuesPredicate> outerInstance;

    std::shared_ptr<DocIdSetIterator> approximation;
    std::shared_ptr<ShapeValues> shapeValues;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<ShapeValuesPredicate> outerInstance,
        std::shared_ptr<DocIdSetIterator> approximation,
        std::shared_ptr<ShapeValues> shapeValues);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;
};

} // namespace org::apache::lucene::spatial::util
