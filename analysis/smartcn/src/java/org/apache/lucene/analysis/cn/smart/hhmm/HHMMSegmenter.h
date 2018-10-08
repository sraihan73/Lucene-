#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/WordDictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegGraph.h"
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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * Finds the optimal segmentation of a sentence into Chinese words
 * @lucene.experimental
 */
class HHMMSegmenter : public std::enable_shared_from_this<HHMMSegmenter>
{
  GET_CLASS_NAME(HHMMSegmenter)

private:
  static std::shared_ptr<WordDictionary> wordDict;

  /**
   * Create the {@link SegGraph} for a sentence.
   *
   * @param sentence input sentence, without start and end markers
   * @return {@link SegGraph} corresponding to the input sentence.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private SegGraph
  // createSegGraph(std::wstring sentence)
  std::shared_ptr<SegGraph> createSegGraph(const std::wstring &sentence);

  /**
   * Get the character types for every character in a sentence.
   *
   * @see Utility#getCharType(char)
   * @param sentence input sentence
   * @return array of character types corresponding to character positions in
   * the sentence
   */
  static std::deque<int> getCharTypes(const std::wstring &sentence);

  /**
   * Return a deque of {@link SegToken} representing the best segmentation of a
   * sentence
   * @param sentence input sentence
   * @return best segmentation as a {@link List}
   */
public:
  virtual std::deque<std::shared_ptr<SegToken>>
  process(const std::wstring &sentence);
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
