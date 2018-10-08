#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

/** Expert: A <code>Scorer</code> for documents matching a <code>Term</code>.
 */
class TermScorer final : public Scorer
{
  GET_CLASS_NAME(TermScorer)
private:
  const std::shared_ptr<PostingsEnum> postingsEnum;
  const std::shared_ptr<Similarity::SimScorer> docScorer;

  /**
   * Construct a <code>TermScorer</code>.
   *
   * @param weight
   *          The weight of the <code>Term</code> in the query.
   * @param td
   *          An iterator over the documents matching the <code>Term</code>.
   * @param docScorer
   *          The <code>Similarity.SimScorer</code> implementation
   *          to be used for score computations.
   */
public:
  TermScorer(std::shared_ptr<Weight> weight, std::shared_ptr<PostingsEnum> td,
             std::shared_ptr<Similarity::SimScorer> docScorer);

  int docID() override;

  int freq() ;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  float score()  override;

  /** Returns a string representation of this <code>TermScorer</code>. */
  virtual std::wstring toString();

protected:
  std::shared_ptr<TermScorer> shared_from_this()
  {
    return std::static_pointer_cast<TermScorer>(Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
