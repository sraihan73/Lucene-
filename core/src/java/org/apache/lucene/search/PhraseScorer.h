#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/PhraseMatcher.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
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

using Similarity = org::apache::lucene::search::similarities::Similarity;

class PhraseScorer : public Scorer
{
  GET_CLASS_NAME(PhraseScorer)

public:
  const std::shared_ptr<PhraseMatcher> matcher;
  const bool needsScores;

private:
  const std::shared_ptr<Similarity::SimScorer> simScorer;

public:
  const float matchCost;

private:
  float freq = 0;

public:
  PhraseScorer(std::shared_ptr<Weight> weight,
               std::shared_ptr<PhraseMatcher> matcher, bool needsScores,
               std::shared_ptr<Similarity::SimScorer> simScorer);

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<PhraseScorer> outerInstance;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<PhraseScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            approximation);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          TwoPhaseIterator::shared_from_this());
    }
  };

public:
  int docID() override;

  float score()  override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<PhraseScorer> shared_from_this()
  {
    return std::static_pointer_cast<PhraseScorer>(Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
