#pragma once
#include "AbstractQueryMaker.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"

#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/SpatialDocMaker.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/ShapeConverter.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Query = org::apache::lucene::search::Query;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Reads spatial data from the body field docs from an internally created {@link
 * LineDocSource}. It's parsed by {@link
 * org.locationtech.spatial4j.context.SpatialContext#readShapeFromWkt(std::wstring)}
 * (std::wstring)} and then further manipulated via a configurable {@link
 * SpatialDocMaker.ShapeConverter}. When using point data, it's likely you'll
 * want to configure the shape converter so that the query shapes actually cover
 * a region. The queries are all created and cached in advance. This query maker
 * works in conjunction with {@link SpatialDocMaker}.  See spatial.alg for a
 * listing of options, in particular the options starting with "query.".
 */
class SpatialFileQueryMaker : public AbstractQueryMaker
{
  GET_CLASS_NAME(SpatialFileQueryMaker)
protected:
  std::shared_ptr<SpatialStrategy> strategy;
  double distErrPct = 0; // NaN if not set
  std::shared_ptr<SpatialOperation> operation;
  bool score = false;

  std::shared_ptr<SpatialDocMaker::ShapeConverter> shapeConverter;

public:
  void
  setConfig(std::shared_ptr<Config> config)  override;

protected:
  std::deque<std::shared_ptr<Query>>
  prepareQueries()  override;

  virtual std::shared_ptr<Query>
  makeQueryFromShape(std::shared_ptr<Shape> shape);

protected:
  std::shared_ptr<SpatialFileQueryMaker> shared_from_this()
  {
    return std::static_pointer_cast<SpatialFileQueryMaker>(
        AbstractQueryMaker::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
