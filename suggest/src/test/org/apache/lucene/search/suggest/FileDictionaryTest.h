#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
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
namespace org::apache::lucene::search::suggest
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class FileDictionaryTest : public LuceneTestCase
{
  GET_CLASS_NAME(FileDictionaryTest)

private:
  std::unordered_map::Entry<std::deque<std::wstring>, std::wstring>
  generateFileEntry(const std::wstring &fieldDelimiter, bool hasWeight,
                    bool hasPayload);

  std::unordered_map::Entry<std::deque<std::deque<std::wstring>>,
                            std::wstring>
  generateFileInput(int count, const std::wstring &fieldDelimiter,
                    bool hasWeights, bool hasPayloads);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFileWithTerm() throws
  // java.io.IOException
  virtual void testFileWithTerm() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFileWithWeight() throws
  // java.io.IOException
  virtual void testFileWithWeight() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFileWithWeightAndPayload() throws
  // java.io.IOException
  virtual void testFileWithWeightAndPayload() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFileWithOneEntry() throws
  // java.io.IOException
  virtual void testFileWithOneEntry() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFileWithDifferentDelimiter() throws
  // java.io.IOException
  virtual void testFileWithDifferentDelimiter() ;

protected:
  std::shared_ptr<FileDictionaryTest> shared_from_this()
  {
    return std::static_pointer_cast<FileDictionaryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest
