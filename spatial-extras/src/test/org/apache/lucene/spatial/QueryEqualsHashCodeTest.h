#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"

#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"
#include  "core/src/java/org/apache/lucene/spatial/ObjGenerator.h"

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

using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::locationtech::spatial4j::context::SpatialContext;

class QueryEqualsHashCodeTest : public LuceneTestCase
{
  GET_CLASS_NAME(QueryEqualsHashCodeTest)

private:
  const std::shared_ptr<SpatialContext> ctx = SpatialContext::GEO;

  std::shared_ptr<SpatialOperation> predicate;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEqualsHashCode()
  virtual void testEqualsHashCode();

private:
  void testEqualsHashcode(std::shared_ptr<SpatialStrategy> strategy);

private:
  class ObjGeneratorAnonymousInnerClass
      : public std::enable_shared_from_this<ObjGeneratorAnonymousInnerClass>,
        public ObjGenerator
  {
    GET_CLASS_NAME(ObjGeneratorAnonymousInnerClass)
  private:
    std::shared_ptr<QueryEqualsHashCodeTest> outerInstance;

    std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy> strategy;

  public:
    ObjGeneratorAnonymousInnerClass(
        std::shared_ptr<QueryEqualsHashCodeTest> outerInstance,
        std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy>
            strategy);

    std::any gen(std::shared_ptr<SpatialArgs> args) override;
  };

private:
  class ObjGeneratorAnonymousInnerClass2
      : public std::enable_shared_from_this<ObjGeneratorAnonymousInnerClass2>,
        public ObjGenerator
  {
    GET_CLASS_NAME(ObjGeneratorAnonymousInnerClass2)
  private:
    std::shared_ptr<QueryEqualsHashCodeTest> outerInstance;

    std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy> strategy;

  public:
    ObjGeneratorAnonymousInnerClass2(
        std::shared_ptr<QueryEqualsHashCodeTest> outerInstance,
        std::shared_ptr<org::apache::lucene::spatial::SpatialStrategy>
            strategy);

    std::any gen(std::shared_ptr<SpatialArgs> args) override;
  };

private:
  void testEqualsHashcode(std::shared_ptr<SpatialArgs> args1,
                          std::shared_ptr<SpatialArgs> args2,
                          std::shared_ptr<ObjGenerator> generator);

  std::shared_ptr<SpatialArgs> makeArgs1();

  std::shared_ptr<SpatialArgs> makeArgs2();

public:
  class ObjGenerator
  {
    GET_CLASS_NAME(ObjGenerator)
  public:
    virtual std::any gen(std::shared_ptr<SpatialArgs> args) = 0;
  };

protected:
  std::shared_ptr<QueryEqualsHashCodeTest> shared_from_this()
  {
    return std::static_pointer_cast<QueryEqualsHashCodeTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/
