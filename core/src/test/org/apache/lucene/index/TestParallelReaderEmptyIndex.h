#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Some tests for {@link ParallelLeafReader}s with empty indexes
 */
class TestParallelReaderEmptyIndex : public LuceneTestCase
{
  GET_CLASS_NAME(TestParallelReaderEmptyIndex)

  /**
   * Creates two empty indexes and wraps a ParallelReader around. Adding this
   * reader to a new index should not throw any exception.
   */
public:
  virtual void testEmptyIndex() ;

  /**
   * This method creates an empty index (numFields=0, numDocs=0) but is marked
   * to have TermVectors. Adding this index to another index should not throw
   * any exception.
   */
  virtual void testEmptyIndexWithVectors() ;

protected:
  std::shared_ptr<TestParallelReaderEmptyIndex> shared_from_this()
  {
    return std::static_pointer_cast<TestParallelReaderEmptyIndex>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
