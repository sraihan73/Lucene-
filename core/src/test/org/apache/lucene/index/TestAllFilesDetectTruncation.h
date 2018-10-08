#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
 * Test that a plain default detects index file truncation early (on opening a
 * reader).
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressFileSystems("ExtrasFS") public class
// TestAllFilesDetectTruncation extends org.apache.lucene.util.LuceneTestCase
class TestAllFilesDetectTruncation : public LuceneTestCase
{
public:
  virtual void test() ;

private:
  void checkTruncation(std::shared_ptr<Directory> dir) ;

  void truncateOneFile(std::shared_ptr<Directory> dir,
                       const std::wstring &victim) ;

protected:
  std::shared_ptr<TestAllFilesDetectTruncation> shared_from_this()
  {
    return std::static_pointer_cast<TestAllFilesDetectTruncation>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
