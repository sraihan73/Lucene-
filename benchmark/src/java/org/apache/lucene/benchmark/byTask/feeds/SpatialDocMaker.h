#pragma once
#include "DocMaker.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/ShapeConverter.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/RecursivePrefixTreeStrategy.h"
#include  "core/src/java/org/apache/lucene/spatial/serialized/SerializedDVStrategy.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/ContentSource.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;

/**
 * Indexes spatial data according to a configured {@link SpatialStrategy} with
optional
 * shape transformation via a configured {@link ShapeConverter}. The converter
can turn points into
 * circles and bounding boxes, in order to vary the type of indexing performance
tests.
 * Unless it's subclass-ed to do otherwise, this class configures a {@link
SpatialContext},
 * {@link SpatialPrefixTree}, and {@link RecursivePrefixTreeStrategy}. The
Strategy is made GET_CLASS_NAME(-ed)
 * available to a query maker via the static method {@link
#getSpatialStrategy(int)}.
 * See spatial.alg for a listing of spatial parameters, in particular those
starting with "spatial."
 * and "doc.spatial".
 */
class SpatialDocMaker : public DocMaker
{
  GET_CLASS_NAME(SpatialDocMaker)

public:
  static const std::wstring SPATIAL_FIELD;

  // cache spatialStrategy by round number
private:
  static std::unordered_map<int, std::shared_ptr<SpatialStrategy>>
      spatialStrategyCache;

  std::shared_ptr<SpatialStrategy> strategy;
  std::shared_ptr<ShapeConverter> shapeConverter;

  /**
   * Looks up the SpatialStrategy from the given round --
   * {@link org.apache.lucene.benchmark.byTask.utils.Config#getRoundNumber()}.
   * It's an error if it wasn't created already for this round -- when
   * SpatialDocMaker is initialized.
   */
public:
  static std::shared_ptr<SpatialStrategy> getSpatialStrategy(int roundNumber);

  /**
   * Builds a SpatialStrategy from configuration options.
   */
protected:
  virtual std::shared_ptr<SpatialStrategy>
  makeSpatialStrategy(std::shared_ptr<Config> config);

private:
  class AbstractMapAnonymousInnerClass
      : public AbstractMap<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(AbstractMapAnonymousInnerClass)
  private:
    std::shared_ptr<SpatialDocMaker> outerInstance;

    std::shared_ptr<Config> config;

  public:
    AbstractMapAnonymousInnerClass(
        std::shared_ptr<SpatialDocMaker> outerInstance,
        std::shared_ptr<Config> config);

    std::shared_ptr<Set<Entry<std::wstring, std::wstring>>> entrySet() override;

    std::wstring get(std::any key) override;

  protected:
    std::shared_ptr<AbstractMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AbstractMapAnonymousInnerClass>(
          java.util.AbstractMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<SpatialStrategy>
  makeSpatialStrategy(std::shared_ptr<Config> config,
                      std::unordered_map<std::wstring, std::wstring> &configMap,
                      std::shared_ptr<SpatialContext> ctx);

  virtual std::shared_ptr<RecursivePrefixTreeStrategy>
  makeRPTStrategy(const std::wstring &spatialField,
                  std::shared_ptr<Config> config,
                  std::unordered_map<std::wstring, std::wstring> &configMap,
                  std::shared_ptr<SpatialContext> ctx);

  virtual std::shared_ptr<SerializedDVStrategy> makeSerializedDVStrategy(
      const std::wstring &spatialField, std::shared_ptr<Config> config,
      std::unordered_map<std::wstring, std::wstring> &configMap,
      std::shared_ptr<SpatialContext> ctx);

  virtual std::shared_ptr<SpatialStrategy> makeCompositeStrategy(
      std::shared_ptr<Config> config,
      std::unordered_map<std::wstring, std::wstring> &configMap,
      std::shared_ptr<SpatialContext> ctx);

public:
  void setConfig(std::shared_ptr<Config> config,
                 std::shared_ptr<ContentSource> source) override;

  /**
   * Optionally converts points to circles, and optionally bbox'es result.
   */
  static std::shared_ptr<ShapeConverter>
  makeShapeConverter(std::shared_ptr<SpatialStrategy> spatialStrategy,
                     std::shared_ptr<Config> config,
                     const std::wstring &configKeyPrefix);

private:
  class ShapeConverterAnonymousInnerClass
      : public std::enable_shared_from_this<ShapeConverterAnonymousInnerClass>,
        public ShapeConverter
  {
    GET_CLASS_NAME(ShapeConverterAnonymousInnerClass)
  private:
    std::shared_ptr<SpatialStrategy> spatialStrategy;
    double radiusDegrees = 0;
    double plusMinus = 0;
    bool bbox = false;

  public:
    ShapeConverterAnonymousInnerClass(
        std::shared_ptr<SpatialStrategy> spatialStrategy, double radiusDegrees,
        double plusMinus, bool bbox);

    std::shared_ptr<Shape> convert(std::shared_ptr<Shape> shape) override;
  };

  /** Converts one shape to another. Created by
   * {@link #makeShapeConverter(org.apache.lucene.spatial.SpatialStrategy,
   * org.apache.lucene.benchmark.byTask.utils.Config, std::wstring)} */
public:
  class ShapeConverter
  {
    GET_CLASS_NAME(ShapeConverter)
  public:
    virtual std::shared_ptr<Shape> convert(std::shared_ptr<Shape> shape) = 0;
  };

public:
  std::shared_ptr<Document> makeDocument()  override;

  static std::shared_ptr<Shape>
  makeShapeFromString(std::shared_ptr<SpatialStrategy> strategy,
                      const std::wstring &name, const std::wstring &shapeStr);

  std::shared_ptr<Document>
  makeDocument(int size)  override;

protected:
  std::shared_ptr<SpatialDocMaker> shared_from_this()
  {
    return std::static_pointer_cast<SpatialDocMaker>(
        DocMaker::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
