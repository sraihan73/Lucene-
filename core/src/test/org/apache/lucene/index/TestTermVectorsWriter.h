#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
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
namespace org::apache::lucene::index
{

using FieldType = org::apache::lucene::document::FieldType;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** tests for writing term vectors */
class TestTermVectorsWriter : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermVectorsWriter)
  // LUCENE-1442
public:
  virtual void testDoubleOffsetCounting() ;

  // LUCENE-1442
  virtual void testDoubleOffsetCounting2() ;

  // LUCENE-1448
  virtual void testEndOffsetPositionCharAnalyzer() ;

  // LUCENE-1448
  virtual void
  testEndOffsetPositionWithCachingTokenFilter() ;

  // LUCENE-1448
  virtual void testEndOffsetPositionStopFilter() ;

  // LUCENE-1448
  virtual void testEndOffsetPositionStandard() ;

  // LUCENE-1448
  virtual void
  testEndOffsetPositionStandardEmptyField() ;

  // LUCENE-1448
  virtual void
  testEndOffsetPositionStandardEmptyField2() ;

  // LUCENE-1168
  virtual void testTermVectorCorruption() ;

  // LUCENE-1168
  virtual void testTermVectorCorruption2() ;

  // LUCENE-1168
  virtual void testTermVectorCorruption3() ;

  // LUCENE-1008
  virtual void testNoTermVectorAfterTermVector() ;

  // LUCENE-1010
  virtual void testNoTermVectorAfterTermVectorMerge() ;

  /**
   * In a single doc, for the same field, mix the term vectors up
   */
  virtual void testInconsistentTermVectorOptions() ;

private:
  void doTestMixup(std::shared_ptr<FieldType> ft1,
                   std::shared_ptr<FieldType> ft2) ;

  // LUCENE-5611: don't abort segment when term deque settings are wrong
public:
  virtual void testNoAbortOnBadTVSettings() ;

protected:
  std::shared_ptr<TestTermVectorsWriter> shared_from_this()
  {
    return std::static_pointer_cast<TestTermVectorsWriter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
