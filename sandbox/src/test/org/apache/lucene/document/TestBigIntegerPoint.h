#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
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

/** Simple tests for {@link BigIntegerPoint} */
class TestBigIntegerPoint : public LuceneTestCase
{
  GET_CLASS_NAME(TestBigIntegerPoint)

  /** Add a single 1D point and search for it */
public:
  virtual void testBasics() ;

  /** Add a negative 1D point and search for it */
  virtual void testNegative() ;

  /** Test if we add a too-large value */
  virtual void testTooLarge() ;

  virtual void testToString() ;

  virtual void testQueryEquals() ;

protected:
  std::shared_ptr<TestBigIntegerPoint> shared_from_this()
  {
    return std::static_pointer_cast<TestBigIntegerPoint>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
