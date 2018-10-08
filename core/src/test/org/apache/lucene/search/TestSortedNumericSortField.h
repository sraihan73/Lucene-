#pragma once
#include "stringhelper.h"
#include <limits>
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

/** Simple tests for SortedNumericSortField */
class TestSortedNumericSortField : public LuceneTestCase
{
  GET_CLASS_NAME(TestSortedNumericSortField)

public:
  virtual void testEmptyIndex() ;

  virtual void testEquals() ;

  virtual void testForward() ;

  virtual void testReverse() ;

  virtual void testMissingFirst() ;

  virtual void testMissingLast() ;

  virtual void testSingleton() ;

  virtual void testFloat() ;

  virtual void testDouble() ;

protected:
  std::shared_ptr<TestSortedNumericSortField> shared_from_this()
  {
    return std::static_pointer_cast<TestSortedNumericSortField>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
