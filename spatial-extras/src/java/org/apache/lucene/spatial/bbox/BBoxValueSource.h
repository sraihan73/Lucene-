#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::bbox
{
class BBoxStrategy;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::spatial
{
class ShapeValues;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
namespace org::apache::lucene::spatial::bbox
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;

/**
 * A ShapeValuesSource returning a Rectangle from each document derived from
 * four numeric fields
 *
 * @lucene.internal
 */
class BBoxValueSource : public ShapeValuesSource
{
  GET_CLASS_NAME(BBoxValueSource)

private:
  const std::shared_ptr<BBoxStrategy> strategy;

public:
  BBoxValueSource(std::shared_ptr<BBoxStrategy> strategy);

  virtual std::wstring toString();

  std::shared_ptr<ShapeValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override;

private:
  class ShapeValuesAnonymousInnerClass : public ShapeValues
  {
    GET_CLASS_NAME(ShapeValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BBoxValueSource> outerInstance;

    std::shared_ptr<NumericDocValues> minX;
    std::shared_ptr<NumericDocValues> minY;
    std::shared_ptr<NumericDocValues> maxX;
    std::shared_ptr<NumericDocValues> maxY;
    std::shared_ptr<Rectangle> rect;

  public:
    ShapeValuesAnonymousInnerClass(
        std::shared_ptr<BBoxValueSource> outerInstance,
        std::shared_ptr<NumericDocValues> minX,
        std::shared_ptr<NumericDocValues> minY,
        std::shared_ptr<NumericDocValues> maxX,
        std::shared_ptr<NumericDocValues> maxY,
        std::shared_ptr<Rectangle> rect);

    bool advanceExact(int doc)  override;

    std::shared_ptr<Shape> value()  override;

  protected:
    std::shared_ptr<ShapeValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ShapeValuesAnonymousInnerClass>(
          org.apache.lucene.spatial.ShapeValues::shared_from_this());
    }
  };

public:
  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<BBoxValueSource> shared_from_this()
  {
    return std::static_pointer_cast<BBoxValueSource>(
        org.apache.lucene.spatial.ShapeValuesSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::bbox
