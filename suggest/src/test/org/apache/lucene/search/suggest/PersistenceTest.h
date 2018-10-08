#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

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
namespace org::apache::lucene::search::suggest
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class PersistenceTest : public LuceneTestCase
{
  GET_CLASS_NAME(PersistenceTest)
public:
  std::deque<std::wstring> const keys = std::deque<std::wstring>{
      L"one",     L"two",      L"three",      L"four",    L"oneness",
      L"onerous", L"onesimus", L"twofold",    L"twonk",   L"thrive",
      L"through", L"threat",   L"foundation", L"fourier", L"fourty"};

  virtual void testTSTPersistence() ;

  virtual void testJaspellPersistence() ;

  virtual void testFSTPersistence() ;

private:
  std::shared_ptr<Directory> getDirectory();

  void runTest(std::type_info lookupClass,
               bool supportsExactWeights) ;

protected:
  std::shared_ptr<PersistenceTest> shared_from_this()
  {
    return std::static_pointer_cast<PersistenceTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
