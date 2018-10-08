#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}

namespace org::apache::lucene::search::spans
{
class Spans;
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
namespace org::apache::lucene::search::spans
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.*;

/** Some utility methods used for testing span queries */
class SpanTestUtil : public std::enable_shared_from_this<SpanTestUtil>
{
  GET_CLASS_NAME(SpanTestUtil)

  /**
   * Adds additional asserts to a spanquery. Highly recommended
   * if you want tests to actually be debuggable.
   */
public:
  static std::shared_ptr<SpanQuery> spanQuery(std::shared_ptr<SpanQuery> query);

  /**
   * Makes a new SpanTermQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery> spanTermQuery(const std::wstring &field,
                                                  const std::wstring &term);

  /**
   * Makes a new SpanOrQuery (with additional asserts) from the provided {@code
   * terms}.
   */
  static std::shared_ptr<SpanQuery>
  spanOrQuery(const std::wstring &field, std::deque<std::wstring> &terms);

  /**
   * Makes a new SpanOrQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanOrQuery(std::deque<SpanQuery> &subqueries);

  /**
   * Makes a new SpanNotQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanNotQuery(std::shared_ptr<SpanQuery> include,
               std::shared_ptr<SpanQuery> exclude);

  /**
   * Makes a new SpanNotQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanNotQuery(std::shared_ptr<SpanQuery> include,
               std::shared_ptr<SpanQuery> exclude, int pre, int post);

  /**
   * Makes a new SpanFirstQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanFirstQuery(std::shared_ptr<SpanQuery> query, int end);

  /**
   * Makes a new SpanPositionRangeQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanPositionRangeQuery(std::shared_ptr<SpanQuery> query, int start, int end);

  /**
   * Makes a new SpanContainingQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanContainingQuery(std::shared_ptr<SpanQuery> big,
                      std::shared_ptr<SpanQuery> little);

  /**
   * Makes a new SpanWithinQuery (with additional asserts).
   */
  static std::shared_ptr<SpanQuery>
  spanWithinQuery(std::shared_ptr<SpanQuery> big,
                  std::shared_ptr<SpanQuery> little);

  /**
   * Makes a new ordered SpanNearQuery (with additional asserts) from the
   * provided {@code terms}
   */
  static std::shared_ptr<SpanQuery>
  spanNearOrderedQuery(const std::wstring &field, int slop,
                       std::deque<std::wstring> &terms);

  /**
   * Makes a new ordered SpanNearQuery (with additional asserts)
   */
  static std::shared_ptr<SpanQuery>
  spanNearOrderedQuery(int slop, std::deque<SpanQuery> &subqueries);

  /**
   * Makes a new unordered SpanNearQuery (with additional asserts) from the
   * provided {@code terms}
   */
  static std::shared_ptr<SpanQuery>
  spanNearUnorderedQuery(const std::wstring &field, int slop,
                         std::deque<std::wstring> &terms);

  /**
   * Makes a new unordered SpanNearQuery (with additional asserts)
   */
  static std::shared_ptr<SpanQuery>
  spanNearUnorderedQuery(int slop, std::deque<SpanQuery> &subqueries);

  /**
   * Assert the next iteration from {@code spans} is a match
   * from {@code start} to {@code end} in {@code doc}.
   */
  static void assertNext(std::shared_ptr<Spans> spans, int doc, int start,
                         int end) ;

  /**
   * Assert that {@code spans} is exhausted.
   */
  static void
  assertFinished(std::shared_ptr<Spans> spans) ;
};

} // namespace org::apache::lucene::search::spans
