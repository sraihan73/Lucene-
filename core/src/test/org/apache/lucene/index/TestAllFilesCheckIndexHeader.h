#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
 * Test that a plain default detects broken index headers early (on opening a
 * reader).
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressFileSystems("ExtrasFS") public class
// TestAllFilesCheckIndexHeader extends org.apache.lucene.util.LuceneTestCase
class TestAllFilesCheckIndexHeader : public LuceneTestCase
{
public:
  virtual void test() ;

private:
  void checkIndexHeader(std::shared_ptr<Directory> dir) ;

  void checkOneFile(std::shared_ptr<Directory> dir,
                    const std::wstring &victim) ;

protected:
  std::shared_ptr<TestAllFilesCheckIndexHeader> shared_from_this()
  {
    return std::static_pointer_cast<TestAllFilesCheckIndexHeader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
