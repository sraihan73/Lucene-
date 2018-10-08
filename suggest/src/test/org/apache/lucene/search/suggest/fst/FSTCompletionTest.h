#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class Input;
}

namespace org::apache::lucene::search::suggest::fst
{
class FSTCompletion;
}
namespace org::apache::lucene::search::suggest::fst
{
class Completion;
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
namespace org::apache::lucene::search::suggest::fst
{

using namespace org::apache::lucene::search::suggest;
using Completion =
    org::apache::lucene::search::suggest::fst::FSTCompletion::Completion;
using Directory = org::apache::lucene::store::Directory;
using namespace org::apache::lucene::util;

/**
 * Unit tests for {@link FSTCompletion}.
 */
class FSTCompletionTest : public LuceneTestCase
{
  GET_CLASS_NAME(FSTCompletionTest)

public:
  static std::shared_ptr<Input> tf(const std::wstring &t, int v);

private:
  std::shared_ptr<FSTCompletion> completion;
  std::shared_ptr<FSTCompletion> completionAlphabetical;

public:
  void setUp()  override;

private:
  std::deque<std::shared_ptr<Input>> evalKeys();

public:
  virtual void testExactMatchHighPriority() ;

  virtual void testExactMatchLowPriority() ;

  virtual void testExactMatchReordering() ;

  virtual void testRequestedCount() ;

  virtual void testMiss() ;

  virtual void testAlphabeticWithWeights() ;

  virtual void testFullMatchList() ;

  virtual void testThreeByte() ;

  virtual void testLargeInputConstantWeights() ;

  virtual void testMultilingualInput() ;

  virtual void testEmptyInput() ;

  virtual void testRandom() ;

private:
  std::shared_ptr<std::wstring>
  stringToCharSequence(const std::wstring &prefix);

  void assertMatchEquals(std::deque<std::shared_ptr<Completion>> &res,
                         std::deque<std::wstring> &expected);

  std::deque<std::wstring> stripScore(std::deque<std::wstring> &expected);

  int maxLen(std::deque<std::wstring> &result);

  std::shared_ptr<Directory> getDirectory();

protected:
  std::shared_ptr<FSTCompletionTest> shared_from_this()
  {
    return std::static_pointer_cast<FSTCompletionTest>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::fst
