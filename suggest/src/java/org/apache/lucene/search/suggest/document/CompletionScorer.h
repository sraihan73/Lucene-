#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/NRTSuggester.h"

#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"

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
namespace org::apache::lucene::search::suggest::document
{

using LeafReader = org::apache::lucene::index::LeafReader;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Bits = org::apache::lucene::util::Bits;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * Expert: Responsible for executing the query against an
 * appropriate suggester and collecting the results
 * via a collector.
 *
 * {@link #score(LeafCollector, Bits, int, int)} is called
 * for each leaf reader.
 *
 * {@link #accept(int,Bits)} and {@link #score(float, float)}
 * is called for every matched completion (i.e. document)
 *
 * @lucene.experimental
 */
class CompletionScorer : public BulkScorer
{
  GET_CLASS_NAME(CompletionScorer)
private:
  const std::shared_ptr<NRTSuggester> suggester;
  const std::shared_ptr<Bits> filterDocs;

  // values accessed by suggester
  /** weight that created this scorer */
protected:
  const std::shared_ptr<CompletionWeight> weight;

public:
  const std::shared_ptr<LeafReader> reader;
  const bool filtered;
  const std::shared_ptr<Automaton> automaton;

  /**
   * Creates a scorer for a field-specific <code>suggester</code> scoped by
   * <code>acceptDocs</code>
   */
protected:
  CompletionScorer(std::shared_ptr<CompletionWeight> weight,
                   std::shared_ptr<NRTSuggester> suggester,
                   std::shared_ptr<LeafReader> reader,
                   std::shared_ptr<Bits> filterDocs, bool const filtered,
                   std::shared_ptr<Automaton> automaton) ;

public:
  int score(std::shared_ptr<LeafCollector> collector,
            std::shared_ptr<Bits> acceptDocs, int min,
            int max)  override;

  int64_t cost() override;

  /**
   * Returns true if a document with <code>docID</code> is accepted,
   * false if the docID maps to a deleted
   * document or has been filtered out
   * @param liveDocs the {@link Bits} representing live docs, or possibly
   *                 {@code null} if all docs are live
   */
  bool accept(int docID, std::shared_ptr<Bits> liveDocs);

  /**
   * Returns the score for a matched completion
   * based on the query time boost and the
   * index time weight.
   */
  virtual float score(float weight, float boost);

protected:
  std::shared_ptr<CompletionScorer> shared_from_this()
  {
    return std::static_pointer_cast<CompletionScorer>(
        org.apache.lucene.search.BulkScorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
