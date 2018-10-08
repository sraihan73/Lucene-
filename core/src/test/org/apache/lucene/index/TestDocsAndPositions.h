#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDocsAndPositions : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocsAndPositions)
private:
  std::wstring fieldName;

public:
  void setUp()  override;

  /**
   * Simple testcase for {@link PostingsEnum}
   */
  virtual void testPositionsSimple() ;

  virtual std::shared_ptr<PostingsEnum>
  getDocsAndPositions(std::shared_ptr<LeafReader> reader,
                      std::shared_ptr<BytesRef> bytes) ;

  /**
   * this test indexes random numbers within a range into a field and checks
   * their occurrences by searching for a number from that range selected at
   * random. All positions for that number are saved up front and compared to
   * the enums positions.
   */
  virtual void testRandomPositions() ;

  virtual void testRandomDocs() ;

private:
  static int findNext(std::deque<int> &docs, int pos, int max);

  /**
   * tests retrieval of positions for terms that have a large number of
   * occurrences to force test of buffer refill during positions iteration.
   */
public:
  virtual void testLargeNumberOfPositions() ;

  virtual void testDocsEnumStart() ;

  virtual void testDocsAndPositionsEnumStart() ;

protected:
  std::shared_ptr<TestDocsAndPositions> shared_from_this()
  {
    return std::static_pointer_cast<TestDocsAndPositions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
