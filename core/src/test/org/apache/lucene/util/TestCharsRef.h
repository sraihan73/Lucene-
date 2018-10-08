#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::util
{

class TestCharsRef : public LuceneTestCase
{
  GET_CLASS_NAME(TestCharsRef)
public:
  virtual void testUTF16InUTF8Order();

  virtual void testAppend();

  virtual void testCopy();

  // LUCENE-3590: fix charsequence to fully obey interface
  virtual void testCharSequenceCharAt();

  // LUCENE-3590: fix off-by-one in subsequence, and fully obey interface
  // LUCENE-4671: fix subSequence
  virtual void testCharSequenceSubSequence();

private:
  void doTestSequence(std::shared_ptr<std::wstring> c);

protected:
  std::shared_ptr<TestCharsRef> shared_from_this()
  {
    return std::static_pointer_cast<TestCharsRef>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
