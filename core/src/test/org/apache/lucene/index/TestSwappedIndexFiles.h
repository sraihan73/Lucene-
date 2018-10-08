#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

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

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test that the same file name, but from a different index, is detected as
 * foreign.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressFileSystems("ExtrasFS") public class
// TestSwappedIndexFiles extends org.apache.lucene.util.LuceneTestCase
class TestSwappedIndexFiles : public LuceneTestCase
{
public:
  virtual void test() ;

private:
  void indexOneDoc(int64_t seed, std::shared_ptr<Directory> dir,
                   std::shared_ptr<Document> doc,
                   bool useCFS) ;

  void swapFiles(std::shared_ptr<Directory> dir1,
                 std::shared_ptr<Directory> dir2) ;

  void swapOneFile(std::shared_ptr<Directory> dir1,
                   std::shared_ptr<Directory> dir2,
                   const std::wstring &victim) ;

protected:
  std::shared_ptr<TestSwappedIndexFiles> shared_from_this()
  {
    return std::static_pointer_cast<TestSwappedIndexFiles>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
