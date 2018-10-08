#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/HHMMSegmenter.h"

#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegTokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegToken.h"

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
namespace org::apache::lucene::analysis::cn::smart
{

using HHMMSegmenter =
    org::apache::lucene::analysis::cn::smart::hhmm::HHMMSegmenter;
using SegToken = org::apache::lucene::analysis::cn::smart::hhmm::SegToken;
using SegTokenFilter =
    org::apache::lucene::analysis::cn::smart::hhmm::SegTokenFilter;

/**
 * Segment a sentence of Chinese text into words.
 * @lucene.experimental
 */
class WordSegmenter : public std::enable_shared_from_this<WordSegmenter>
{
  GET_CLASS_NAME(WordSegmenter)

private:
  std::shared_ptr<HHMMSegmenter> hhmmSegmenter =
      std::make_shared<HHMMSegmenter>();

  std::shared_ptr<SegTokenFilter> tokenFilter =
      std::make_shared<SegTokenFilter>();

  /**
   * Segment a sentence into words with {@link HHMMSegmenter}
   *
   * @param sentence input sentence
   * @param startOffset start offset of sentence
   * @return {@link List} of {@link SegToken}
   */
public:
  virtual std::deque<std::shared_ptr<SegToken>>
  segmentSentence(const std::wstring &sentence, int startOffset);

  /**
   * Process a {@link SegToken} so that it is ready for indexing.
   *
   * This method calculates offsets and normalizes the token with {@link
   * SegTokenFilter}.
   *
   * @param st input {@link SegToken}
   * @param sentence associated Sentence
   * @param sentenceStartOffset offset into sentence
   * @return Lucene {@link SegToken}
   */
  virtual std::shared_ptr<SegToken>
  convertSegToken(std::shared_ptr<SegToken> st, const std::wstring &sentence,
                  int sentenceStartOffset);
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/
