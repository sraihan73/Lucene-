#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::uhighlight
{
class FieldOffsetStrategy;
}

namespace org::apache::lucene::search::uhighlight
{
class PassageScorer;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageFormatter;
}
namespace org::apache::lucene::search::uhighlight
{
class UnifiedHighlighter;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::uhighlight
{
class Passage;
}
namespace org::apache::lucene::search::uhighlight
{
class OffsetsEnum;
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
namespace org::apache::lucene::search::uhighlight
{

using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Internal highlighter abstraction that operates on a per field basis.
 *
 * @lucene.internal
 */
class FieldHighlighter : public std::enable_shared_from_this<FieldHighlighter>
{
  GET_CLASS_NAME(FieldHighlighter)

protected:
  const std::wstring field;
  const std::shared_ptr<FieldOffsetStrategy> fieldOffsetStrategy;
  const std::shared_ptr<BreakIterator> breakIterator; // note: stateful!
  const std::shared_ptr<PassageScorer> passageScorer;
  const int maxPassages;
  const int maxNoHighlightPassages;
  const std::shared_ptr<PassageFormatter> passageFormatter;

public:
  FieldHighlighter(const std::wstring &field,
                   std::shared_ptr<FieldOffsetStrategy> fieldOffsetStrategy,
                   std::shared_ptr<BreakIterator> breakIterator,
                   std::shared_ptr<PassageScorer> passageScorer,
                   int maxPassages, int maxNoHighlightPassages,
                   std::shared_ptr<PassageFormatter> passageFormatter);

  virtual std::wstring getField();

  virtual UnifiedHighlighter::OffsetSource getOffsetSource();

  /**
   * The primary method -- highlight this doc, assuming a specific field and
   * given this content.
   */
  virtual std::any
  highlightFieldForDoc(std::shared_ptr<IndexReader> reader, int docId,
                       const std::wstring &content) ;

  /**
   * Called to summarize a document when no highlights were found.
   * By default this just returns the first
   * {@link #maxPassages} sentences; subclasses can override to customize.
   * The state of {@link #breakIterator} should be at the beginning.
GET_CLASS_NAME(es)
   */
protected:
  virtual std::deque<std::shared_ptr<Passage>>
  getSummaryPassagesNoHighlight(int maxPassages);

  // algorithm: treat sentence snippets as miniature documents
  // we can intersect these with the postings lists via
  // BreakIterator.preceding(offset),s score each sentence as
  // norm(sentenceStartOffset) * sum(weight * tf(freq))
  virtual std::deque<std::shared_ptr<Passage>>
  highlightOffsetsEnums(std::shared_ptr<OffsetsEnum> off) ;

private:
  std::shared_ptr<Passage> maybeAddPassage(
      std::shared_ptr<PriorityQueue<std::shared_ptr<Passage>>> passageQueue,
      std::shared_ptr<PassageScorer> scorer, std::shared_ptr<Passage> passage,
      int contentLength);
};

} // namespace org::apache::lucene::search::uhighlight
