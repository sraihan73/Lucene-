#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class RangeMergePolicy;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
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

using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPerSegmentDeletes : public LuceneTestCase
{
  GET_CLASS_NAME(TestPerSegmentDeletes)
public:
  virtual void testDeletes1() ;

  /**
  static bool hasPendingDeletes(SegmentInfos infos) {
    for (SegmentInfo info : infos) {
      if (info.deletes.any()) {
        return true;
      }
    }
    return false;
  }
  **/
  virtual void
  part2(std::shared_ptr<IndexWriter> writer,
        std::shared_ptr<RangeMergePolicy> fsmp) ;

  virtual bool segThere(std::shared_ptr<SegmentCommitInfo> info,
                        std::shared_ptr<SegmentInfos> infos);

  static void printDelDocs(std::shared_ptr<Bits> bits);

  virtual std::deque<int>
  toDocsArray(std::shared_ptr<Term> term, std::shared_ptr<Bits> bits,
              std::shared_ptr<IndexReader> reader) ;

  static std::deque<int>
  toArray(std::shared_ptr<PostingsEnum> postingsEnum) ;

public:
  class RangeMergePolicy : public MergePolicy
  {
    GET_CLASS_NAME(RangeMergePolicy)
  public:
    bool doMerge = false;
    int start = 0;
    int length = 0;

  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool useCompoundFile_;

    RangeMergePolicy(bool useCompoundFile);

  public:
    std::shared_ptr<MergeSpecification> findMerges(
        MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> segmentInfos,
        std::shared_ptr<MergeContext> mergeContext)  override;

    std::shared_ptr<MergeSpecification> findForcedMerges(
        std::shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
        std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
            &segmentsToMerge,
        std::shared_ptr<MergeContext> mergeContext)  override;

    std::shared_ptr<MergeSpecification> findForcedDeletesMerges(
        std::shared_ptr<SegmentInfos> segmentInfos,
        std::shared_ptr<MergeContext> mergeContext)  override;

    bool useCompoundFile(std::shared_ptr<SegmentInfos> segments,
                         std::shared_ptr<SegmentCommitInfo> newSegment,
                         std::shared_ptr<MergeContext> mergeContext) override;

  protected:
    std::shared_ptr<RangeMergePolicy> shared_from_this()
    {
      return std::static_pointer_cast<RangeMergePolicy>(
          MergePolicy::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPerSegmentDeletes> shared_from_this()
  {
    return std::static_pointer_cast<TestPerSegmentDeletes>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
