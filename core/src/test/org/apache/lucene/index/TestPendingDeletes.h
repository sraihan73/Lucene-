#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PendingDeletes;
}

namespace org::apache::lucene::index
{
class SegmentCommitInfo;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPendingDeletes : public LuceneTestCase
{
  GET_CLASS_NAME(TestPendingDeletes)

protected:
  virtual std::shared_ptr<PendingDeletes>
  newPendingDeletes(std::shared_ptr<SegmentCommitInfo> commitInfo);

public:
  virtual void testDeleteDoc() ;

  virtual void testWriteLiveDocs() ;

  virtual void testIsFullyDeleted() ;

protected:
  std::shared_ptr<TestPendingDeletes> shared_from_this()
  {
    return std::static_pointer_cast<TestPendingDeletes>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
