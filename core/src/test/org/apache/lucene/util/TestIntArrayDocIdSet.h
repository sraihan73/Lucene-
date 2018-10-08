#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IntArrayDocIdSet;
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
namespace org::apache::lucene::util
{

class TestIntArrayDocIdSet
    : public BaseDocIdSetTestCase<std::shared_ptr<IntArrayDocIdSet>>
{
  GET_CLASS_NAME(TestIntArrayDocIdSet)

public:
  std::shared_ptr<IntArrayDocIdSet>
  copyOf(std::shared_ptr<BitSet> bs, int length)  override;

protected:
  std::shared_ptr<TestIntArrayDocIdSet> shared_from_this()
  {
    return std::static_pointer_cast<TestIntArrayDocIdSet>(
        BaseDocIdSetTestCase<IntArrayDocIdSet>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
