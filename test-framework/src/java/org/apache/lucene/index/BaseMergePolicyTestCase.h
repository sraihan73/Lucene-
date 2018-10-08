#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <iostream>
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergePolicy;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::util
{
class InfoStream;
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

using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NullInfoStream = org::apache::lucene::util::NullInfoStream;

/**
 * Base test case for {@link MergePolicy}.
 */
class BaseMergePolicyTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseMergePolicyTestCase)

  /** Create a new {@link MergePolicy} instance. */
protected:
  virtual std::shared_ptr<MergePolicy> mergePolicy() = 0;

public:
  virtual void testForceMergeNotNeeded() ;

private:
  class SerialMergeSchedulerAnonymousInnerClass : public SerialMergeScheduler
  {
    GET_CLASS_NAME(SerialMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<BaseMergePolicyTestCase> outerInstance;

    std::shared_ptr<AtomicBoolean> mayMerge;

  public:
    SerialMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<BaseMergePolicyTestCase> outerInstance,
        std::shared_ptr<AtomicBoolean> mayMerge);

    // C++ WARNING: The following method was originally marked 'synchronized':
    void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
               bool newMergesFound)  override;

  protected:
    std::shared_ptr<SerialMergeSchedulerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SerialMergeSchedulerAnonymousInnerClass>(
          SerialMergeScheduler::shared_from_this());
    }
  };

public:
  virtual void testFindForcedDeletesMerges() ;

  /**
   * Simple mock merge context for tests
   */
public:
  class MockMergeContext final
      : public std::enable_shared_from_this<MockMergeContext>,
        public MergePolicy::MergeContext
  {
    GET_CLASS_NAME(MockMergeContext)
  private:
    const std::function<int(SegmentCommitInfo *)> numDeletesFunc;
    const std::shared_ptr<InfoStream> infoStream =
        std::make_shared<NullInfoStream>();

  public:
    MockMergeContext(std::function<int(SegmentCommitInfo *)> &numDeletesFunc);

    int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info) override;

    int numDeletedDocs(std::shared_ptr<SegmentCommitInfo> info) override;

    std::shared_ptr<InfoStream> getInfoStream() override;

    std::shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
    getMergingSegments() override;
  };

protected:
  std::shared_ptr<BaseMergePolicyTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseMergePolicyTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
