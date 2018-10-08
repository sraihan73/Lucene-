#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PostingsEnum;
}

namespace org::apache::lucene::search
{
class PhraseQuery;
}
namespace org::apache::lucene::search
{
class PostingsAndFreq;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
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

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;

class ExactPhraseMatcher final : public PhraseMatcher
{
  GET_CLASS_NAME(ExactPhraseMatcher)

private:
  class PostingsAndPosition
      : public std::enable_shared_from_this<PostingsAndPosition>
  {
    GET_CLASS_NAME(PostingsAndPosition)
  private:
    const std::shared_ptr<PostingsEnum> postings;
    const int offset;
    int freq = 0, upTo = 0, pos = 0;

  public:
    PostingsAndPosition(std::shared_ptr<PostingsEnum> postings, int offset);
  };

private:
  std::deque<std::shared_ptr<PostingsAndPosition>> const postings;

public:
  ExactPhraseMatcher(
      std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings,
      float matchCost);

private:
  static std::shared_ptr<DocIdSetIterator> approximation(
      std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings);

public:
  float maxFreq() override;

  /** Advance the given pos enum to the first doc on or after {@code target}.
   *  Return {@code false} if the enum was exhausted before reaching
   *  {@code target} and {@code true} otherwise. */
private:
  static bool advancePosition(std::shared_ptr<PostingsAndPosition> posting,
                              int target) ;

public:
  void reset()  override;

  bool nextMatch()  override;

  float sloppyWeight(std::shared_ptr<Similarity::SimScorer> simScorer) override;

  int startPosition() override;

  int endPosition() override;

  int startOffset()  override;

  int endOffset()  override;

protected:
  std::shared_ptr<ExactPhraseMatcher> shared_from_this()
  {
    return std::static_pointer_cast<ExactPhraseMatcher>(
        PhraseMatcher::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
