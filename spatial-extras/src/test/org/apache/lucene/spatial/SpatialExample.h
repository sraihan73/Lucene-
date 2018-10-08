#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"

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
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

/**
 * This class serves as example code to show how to use the Lucene spatial
 * module.
 */
class SpatialExample : public LuceneTestCase
{
  GET_CLASS_NAME(SpatialExample)

  // Note: Test invoked via TestTestFramework.spatialExample()

  static void main(std::deque<std::wstring> &args) ;

public:
  virtual void test() ;

  /**
   * The Spatial4j {@link SpatialContext} is a sort of global-ish singleton
   * needed by Lucene spatial.  It's a facade to the rest of Spatial4j, acting
   * as a factory for {@link Shape}s and provides access to reading and writing
   * them from Strings.
   */
private:
  std::shared_ptr<SpatialContext> ctx; //"ctx" is the conventional variable name

  /**
   * The Lucene spatial {@link SpatialStrategy} encapsulates an approach to
   * indexing and searching shapes, and providing distance values for them.
   * It's a simple API to unify different approaches. You might use more than
   * one strategy for a shape as each strategy has its strengths and weaknesses.
   * <p />
   * Note that these are initialized with a field name.
   */
  std::shared_ptr<SpatialStrategy> strategy;

  std::shared_ptr<Directory> directory;

protected:
  virtual void init();

private:
  void indexPoints() ;

  std::shared_ptr<Document> newSampleDocument(int id,
                                              std::deque<Shape> &shapes);

  void search() ;

  void assertDocMatchedIds(std::shared_ptr<IndexSearcher> indexSearcher,
                           std::shared_ptr<TopDocs> docs,
                           std::deque<int> &ids) ;

protected:
  std::shared_ptr<SpatialExample> shared_from_this()
  {
    return std::static_pointer_cast<SpatialExample>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/
