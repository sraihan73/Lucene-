#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/ShapeValuesSource.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/spatial/ShapeValues.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
namespace org::apache::lucene::spatial::bbox
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::shape::Rectangle;

/**
 * A base class for calculating a spatial relevance rank per document from a
provided
 * {@link ShapeValuesSource} returning a {@link
GET_CLASS_NAME(for)
 * org.locationtech.spatial4j.shape.Rectangle} per-document.
 * <p>
 * Implementers: remember to implement equals and hashCode if you have
 * fields!
 *
 * @lucene.experimental
 */
class BBoxSimilarityValueSource : public DoubleValuesSource
{
  GET_CLASS_NAME(BBoxSimilarityValueSource)

private:
  const std::shared_ptr<ShapeValuesSource> bboxValueSource;

public:
  BBoxSimilarityValueSource(std::shared_ptr<ShapeValuesSource> bboxValueSource);

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual std::wstring toString();

  /** A comma-separated deque of configurable items of the subclass to put into
   * {@link #toString()}. */
protected:
  virtual std::wstring similarityDescription() = 0;

public:
  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BBoxSimilarityValueSource> outerInstance;

    std::shared_ptr<ShapeValues> shapeValues;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<BBoxSimilarityValueSource> outerInstance,
        std::shared_ptr<ShapeValues> shapeValues);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

  /**
   * Return a relevancy score. If {@code exp} is provided then diagnostic
   * information is added.
   * @param rect The indexed rectangle; not null.
   * @param exp Optional diagnostic holder.
   * @return a score.
   */
protected:
  virtual double
  score(std::shared_ptr<Rectangle> rect,
        std::shared_ptr<AtomicReference<std::shared_ptr<Explanation>>> exp) = 0;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  bool needsScores() override;

protected:
  std::shared_ptr<BBoxSimilarityValueSource> shared_from_this()
  {
    return std::static_pointer_cast<BBoxSimilarityValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/bbox/
