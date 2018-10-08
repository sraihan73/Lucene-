#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanCollector.h"

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
namespace org::apache::lucene::search::spans
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * Expert:
 * Public for extension only.
 * This does not work correctly for terms that indexed at position
 * Integer.MAX_VALUE.
 */
class TermSpans : public Spans
{
  GET_CLASS_NAME(TermSpans)
protected:
  const std::shared_ptr<PostingsEnum> postings;
  const std::shared_ptr<Term> term;
  int doc = 0;
  int freq = 0;
  int count = 0;
  int position = 0;
  bool readPayload = false;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const float positionsCost_;

public:
  TermSpans(std::shared_ptr<Similarity::SimScorer> scorer,
            std::shared_ptr<PostingsEnum> postings, std::shared_ptr<Term> term,
            float positionsCost);

  int nextDoc()  override;

  int advance(int target)  override;

  int docID() override;

  int nextStartPosition()  override;

  int startPosition() override;

  int endPosition() override;

  int width() override;

  int64_t cost() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

  float positionsCost() override;

  virtual std::wstring toString();

  virtual std::shared_ptr<PostingsEnum> getPostings();

protected:
  std::shared_ptr<TermSpans> shared_from_this()
  {
    return std::static_pointer_cast<TermSpans>(Spans::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
