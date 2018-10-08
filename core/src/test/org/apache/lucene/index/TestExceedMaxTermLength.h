#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
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

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests that a useful exception is thrown when attempting to index a term that
 * is too large
 *
 * @see IndexWriter#MAX_TERM_LENGTH
 */
class TestExceedMaxTermLength : public LuceneTestCase
{
  GET_CLASS_NAME(TestExceedMaxTermLength)

private:
  static const int minTestTermLength = IndexWriter::MAX_TERM_LENGTH + 1;
  static const int maxTestTermLegnth = IndexWriter::MAX_TERM_LENGTH * 2;

public:
  std::shared_ptr<Directory> dir = nullptr;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void createDir()
  virtual void createDir();
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void destroyDir() throws java.io.IOException
  virtual void destroyDir() ;

  virtual void test() ;

protected:
  std::shared_ptr<TestExceedMaxTermLength> shared_from_this()
  {
    return std::static_pointer_cast<TestExceedMaxTermLength>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
