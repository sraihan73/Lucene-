#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgsParser.h"

#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"
#include  "core/src/java/org/apache/lucene/spatial/SpatialMatchConcern.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/spatial/SpatialTestData.h"
#include  "core/src/java/org/apache/lucene/spatial/SpatialTestQuery.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"

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
namespace org::apache::lucene::spatial
{

using Document = org::apache::lucene::document::Document;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

class StrategyTestCase : public SpatialTestCase
{
  GET_CLASS_NAME(StrategyTestCase)

public:
  static const std::wstring DATA_SIMPLE_BBOX;
  static const std::wstring DATA_STATES_POLY;
  static const std::wstring DATA_STATES_BBOX;
  static const std::wstring DATA_COUNTRIES_POLY;
  static const std::wstring DATA_COUNTRIES_BBOX;
  static const std::wstring DATA_WORLD_CITIES_POINTS;

  static const std::wstring QTEST_States_IsWithin_BBox;
  static const std::wstring QTEST_States_Intersects_BBox;
  static const std::wstring QTEST_Cities_Intersects_BBox;
  static const std::wstring QTEST_Simple_Queries_BBox;

protected:
  std::shared_ptr<Logger> log = Logger::getLogger(getClassName());

  const std::shared_ptr<SpatialArgsParser> argsParser =
      std::make_shared<SpatialArgsParser>();

  std::shared_ptr<SpatialStrategy> strategy;
  bool storeShape = true;

  virtual void
  executeQueries(std::shared_ptr<SpatialMatchConcern> concern,
                 std::deque<std::wstring> &testQueryFile) ;

  virtual void getAddAndVerifyIndexedDocuments(
      const std::wstring &testDataFile) ;

  virtual std::deque<std::shared_ptr<Document>>
  getDocuments(const std::wstring &testDataFile) ;

  virtual std::deque<std::shared_ptr<Document>> getDocuments(
      std::shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>> sampleData);

  /** Subclasses may override to transform or remove a shape for indexing */
  virtual std::shared_ptr<Shape>
  convertShapeFromGetDocuments(std::shared_ptr<Shape> shape);

  virtual std::shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>>
  getSampleData(const std::wstring &testDataFile) ;

  virtual std::shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>>
  getTestQueries(const std::wstring &testQueryFile,
                 std::shared_ptr<SpatialContext> ctx) ;

public:
  virtual void runTestQueries(
      std::shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>> queries,
      std::shared_ptr<SpatialMatchConcern> concern);

  virtual void runTestQuery(std::shared_ptr<SpatialMatchConcern> concern,
                            std::shared_ptr<SpatialTestQuery> q);

protected:
  virtual std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialTestQuery> q);

  virtual void adoc(const std::wstring &id,
                    const std::wstring &shapeStr) throw(IOException,
                                                        ParseException);
  virtual void adoc(const std::wstring &id,
                    std::shared_ptr<Shape> shape) ;

  virtual std::shared_ptr<Document> newDoc(const std::wstring &id,
                                           std::shared_ptr<Shape> shape);

  virtual void deleteDoc(const std::wstring &id) ;

  /** scores[] are in docId order */
  virtual void checkValueSource(std::shared_ptr<DoubleValuesSource> vs,
                                std::deque<float> &scores,
                                float delta) ;

  virtual void testOperation(std::shared_ptr<Shape> indexedShape,
                             std::shared_ptr<SpatialOperation> operation,
                             std::shared_ptr<Shape> queryShape,
                             bool match) ;

protected:
  std::shared_ptr<StrategyTestCase> shared_from_this()
  {
    return std::static_pointer_cast<StrategyTestCase>(
        SpatialTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/
