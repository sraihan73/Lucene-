#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::search
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Tests for SortedSetSortField selectors other than MIN,
 *  these require optional codec support (random access to ordinals) */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"Memory", "SimpleText"}) public class
// TestSortedSetSelector extends org.apache.lucene.util.LuceneTestCase
class TestSortedSetSelector : public LuceneTestCase
{

public:
  virtual void testMax() ;

  virtual void testMaxReverse() ;

  virtual void testMaxMissingFirst() ;

  virtual void testMaxMissingLast() ;

  virtual void testMaxSingleton() ;

  virtual void testMiddleMin() ;

  virtual void testMiddleMinReverse() ;

  virtual void testMiddleMinMissingFirst() ;

  virtual void testMiddleMinMissingLast() ;

  virtual void testMiddleMinSingleton() ;

  virtual void testMiddleMax() ;

  virtual void testMiddleMaxReverse() ;

  virtual void testMiddleMaxMissingFirst() ;

  virtual void testMiddleMaxMissingLast() ;

  virtual void testMiddleMaxSingleton() ;

protected:
  std::shared_ptr<TestSortedSetSelector> shared_from_this()
  {
    return std::static_pointer_cast<TestSortedSetSelector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
