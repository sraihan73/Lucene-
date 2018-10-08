#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DirectoryReader;
}

namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::spatial
{
class SearchResults;
}
namespace org::apache::lucene::spatial
{
class SearchResult;
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
namespace org::apache::lucene::spatial
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomGaussian;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

/** A base test class for spatial lucene. It's mostly Lucene generic. */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressSysoutChecks(bugUrl = "These tests use JUL
// extensively.") public abstract class SpatialTestCase extends
// org.apache.lucene.util.LuceneTestCase
class SpatialTestCase : public LuceneTestCase
{

protected:
  std::shared_ptr<Logger> log = Logger::getLogger(getClassName());

private:
  std::shared_ptr<DirectoryReader> indexReader;

protected:
  std::shared_ptr<RandomIndexWriter> indexWriter;

private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<Analyzer> analyzer;

protected:
  std::shared_ptr<IndexSearcher> indexSearcher;

  std::shared_ptr<SpatialContext> ctx; // subclass must initialize

public:
  void setUp()  override;

  void tearDown()  override;

  // ================================================= Helper Methods
  // ================================================

protected:
  virtual void addDocument(std::shared_ptr<Document> doc) ;

  virtual void addDocumentsAndCommit(
      std::deque<std::shared_ptr<Document>> &documents) ;

  virtual void deleteAll() ;

  virtual void commit() ;

  virtual void verifyDocumentsIndexed(int numDocs);

  virtual std::shared_ptr<SearchResults>
  executeQuery(std::shared_ptr<Query> query, int numDocs);

  virtual std::shared_ptr<Point> randomPoint();

  virtual std::shared_ptr<Rectangle> randomRectangle();

  virtual std::shared_ptr<Rectangle>
  randomRectangle(std::shared_ptr<Rectangle> bounds);

  /** Returns new minStart and new length that is inside the range specified by
   * the arguments. */
  virtual std::deque<double> randomSubRange(double boundStart,
                                             double boundLen);

private:
  double randomGaussianMinMeanMax(double min, double mean, double max);

  /**
   * Within one standard deviation (68% of the time) the result is "close" to
   * mean. By "close": when greater than mean, it's the lesser of 2*mean or half
   * way to max, when lesser than mean, it's the greater of max-2*mean or half
   * way to 0. The other 32% of the time it's in the rest of the range, touching
   * either 0 or max but never exceeding.
   */
  double randomGaussianMeanMax(double mean, double max);

  // ================================================= Inner Classes
  // =================================================

protected:
  class SearchResults : public std::enable_shared_from_this<SearchResults>
  {
    GET_CLASS_NAME(SearchResults)

  public:
    int64_t numFound = 0;
    std::deque<std::shared_ptr<SearchResult>> results;

    SearchResults(int64_t numFound,
                  std::deque<std::shared_ptr<SearchResult>> &results);

    virtual std::shared_ptr<StringBuilder> toDebugString();

    virtual std::wstring toString();
  };

protected:
  class SearchResult : public std::enable_shared_from_this<SearchResult>
  {
    GET_CLASS_NAME(SearchResult)

  public:
    float score = 0;
    std::shared_ptr<Document> document;

    SearchResult(float score, std::shared_ptr<Document> document);

    virtual std::wstring getId();

    virtual std::wstring toString();
  };

protected:
  std::shared_ptr<SpatialTestCase> shared_from_this()
  {
    return std::static_pointer_cast<SpatialTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial
