#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MultiPostingsEnum;
}

namespace org::apache::lucene::index
{
class MappingPostingsSub;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Exposes flex API, merged from flex API of sub-segments,
 * remapping docIDs (this is used for segment merging).
 *
 * @lucene.experimental
 */

class MappingMultiPostingsEnum final : public PostingsEnum
{
  GET_CLASS_NAME(MappingMultiPostingsEnum)
public:
  std::shared_ptr<MultiPostingsEnum> multiDocsAndPositionsEnum;
  const std::wstring field;
  const std::shared_ptr<DocIDMerger<std::shared_ptr<MappingPostingsSub>>>
      docIDMerger;

private:
  std::shared_ptr<MappingPostingsSub> current;
  std::deque<std::shared_ptr<MappingPostingsSub>> const allSubs;
  const std::deque<std::shared_ptr<MappingPostingsSub>> subs =
      std::deque<std::shared_ptr<MappingPostingsSub>>();

private:
  class MappingPostingsSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(MappingPostingsSub)
  public:
    std::shared_ptr<PostingsEnum> postings;

    MappingPostingsSub(std::shared_ptr<MergeState::DocMap> docMap);

    int nextDoc() override;

  protected:
    std::shared_ptr<MappingPostingsSub> shared_from_this()
    {
      return std::static_pointer_cast<MappingPostingsSub>(
          DocIDMerger.Sub::shared_from_this());
    }
  };

  /** Sole constructor. */
public:
  MappingMultiPostingsEnum(
      const std::wstring &field,
      std::shared_ptr<MergeState> mergeState) ;

  std::shared_ptr<MappingMultiPostingsEnum>
  reset(std::shared_ptr<MultiPostingsEnum> postingsEnum) ;

  int freq()  override;

  int docID() override;

  int advance(int target) override;

  int nextDoc()  override;

  int nextPosition()  override;

  int startOffset()  override;

  int endOffset()  override;

  std::shared_ptr<BytesRef> getPayload()  override;

  int64_t cost() override;

protected:
  std::shared_ptr<MappingMultiPostingsEnum> shared_from_this()
  {
    return std::static_pointer_cast<MappingMultiPostingsEnum>(
        PostingsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
