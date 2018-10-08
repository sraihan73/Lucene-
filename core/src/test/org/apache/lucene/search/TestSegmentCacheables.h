#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReaderContext;
}

namespace org::apache::lucene::search
{
class SegmentCacheable;
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

namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSegmentCacheables : public LuceneTestCase
{
  GET_CLASS_NAME(TestSegmentCacheables)

private:
  static bool isCacheable(std::shared_ptr<LeafReaderContext> ctx,
                          std::deque<SegmentCacheable> &ss);

public:
  virtual void testMultipleDocValuesDelegates() ;

protected:
  std::shared_ptr<TestSegmentCacheables> shared_from_this()
  {
    return std::static_pointer_cast<TestSegmentCacheables>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
