#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <unordered_map>
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
namespace org::apache::lucene::store
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Test huge RAMFile with more than Integer.MAX_VALUE bytes. */
class TestHugeRamFile : public LuceneTestCase
{
  GET_CLASS_NAME(TestHugeRamFile)

private:
  static const int64_t MAX_VALUE =
      static_cast<int64_t>(2) *
      static_cast<int64_t>(std::numeric_limits<int>::max());

  /** Fake a huge ram file by using the same byte buffer for all
   * buffers under maxint. */
private:
  class DenseRAMFile : public RAMFile
  {
    GET_CLASS_NAME(DenseRAMFile)
  private:
    int64_t capacity = 0;
    std::unordered_map<int, std::deque<char>> singleBuffers =
        std::unordered_map<int, std::deque<char>>();

  protected:
    std::deque<char> newBuffer(int size) override;

  protected:
    std::shared_ptr<DenseRAMFile> shared_from_this()
    {
      return std::static_pointer_cast<DenseRAMFile>(
          RAMFile::shared_from_this());
    }
  };

  /** Test huge RAMFile with more than Integer.MAX_VALUE bytes. (LUCENE-957) */
public:
  virtual void testHugeFile() ;

protected:
  std::shared_ptr<TestHugeRamFile> shared_from_this()
  {
    return std::static_pointer_cast<TestHugeRamFile>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
