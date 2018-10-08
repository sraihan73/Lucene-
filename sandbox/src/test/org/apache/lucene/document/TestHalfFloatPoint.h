#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
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

class TestHalfFloatPoint : public LuceneTestCase
{
  GET_CLASS_NAME(TestHalfFloatPoint)

private:
  void testHalfFloat(const std::wstring &sbits, float value);

public:
  virtual void testHalfFloatConversion();

  virtual void testRoundShift();

  virtual void testRounding();

  virtual void testSortableBits();

  virtual void testSortableBytes();

  /** Add a single value and search for it */
  virtual void testBasics() ;

  /** Add a single multi-dimensional value and search for it */
  virtual void testBasicsMultiDims() ;

  virtual void testNextUp();

  virtual void testNextDown();

protected:
  std::shared_ptr<TestHalfFloatPoint> shared_from_this()
  {
    return std::static_pointer_cast<TestHalfFloatPoint>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
