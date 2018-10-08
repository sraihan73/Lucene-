#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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

using namespace org::apache::lucene::index;

/**
 * Position of a term in a document that takes into account the term offset
 * within the phrase.
 */
class PhrasePositions final
    : public std::enable_shared_from_this<PhrasePositions>
{
  GET_CLASS_NAME(PhrasePositions)
public:
  int position = 0; // position in doc
  int count = 0;    // remaining pos in this doc
  int offset = 0;   // position in phrase
  const int ord;    // unique across all PhrasePositions instances
  const std::shared_ptr<PostingsEnum> postings; // stream of docs & positions
  std::shared_ptr<PhrasePositions> next;        // used to make lists
  int rptGroup = -1; // >=0 indicates that this is a repeating PP
  int rptInd = 0;    // index in the rptGroup
  std::deque<std::shared_ptr<Term>> const
      terms; // for repetitions initialization

  PhrasePositions(std::shared_ptr<PostingsEnum> postings, int o, int ord,
                  std::deque<std::shared_ptr<Term>> &terms);

  void firstPosition() ;

  /**
   * Go to next location of this term current document, and set
   * <code>position</code> as <code>location - offset</code>, so that a
   * matching exact phrase is easily identified when all PhrasePositions
   * have exactly the same <code>position</code>.
   */
  bool nextPosition() ;

  /** for debug purposes */
  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/search/
