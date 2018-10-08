#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::document
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

/** Simple tests for {@link LatLonDocValuesField#newDistanceSort} */
class TestLatLonPointDistanceSort : public LuceneTestCase
{
  GET_CLASS_NAME(TestLatLonPointDistanceSort)

  /** Add three points and sort by distance */
public:
  virtual void testDistanceSort() ;

  /** Add two points (one doc missing) and sort by distance */
  virtual void testMissingLast() ;

  /** Run a few iterations with just 10 docs, hopefully easy to debug */
  virtual void testRandom() ;

  /** Runs with thousands of docs */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomHuge() throws Exception
  virtual void testRandomHuge() ;

  // result class used for testing. holds an id+distance.
  // we sort these with Arrays.sort and compare with lucene's results
public:
  class Result : public std::enable_shared_from_this<Result>,
                 public Comparable<std::shared_ptr<Result>>
  {
    GET_CLASS_NAME(Result)
  public:
    int id = 0;
    double distance = 0;

    Result(int id, double distance);

    int compareTo(std::shared_ptr<Result> o) override;

    virtual int hashCode();

    bool equals(std::any obj) override;

    virtual std::wstring toString();
  };

private:
  void doRandomTest(int numDocs, int numQueries) ;

protected:
  std::shared_ptr<TestLatLonPointDistanceSort> shared_from_this()
  {
    return std::static_pointer_cast<TestLatLonPointDistanceSort>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
