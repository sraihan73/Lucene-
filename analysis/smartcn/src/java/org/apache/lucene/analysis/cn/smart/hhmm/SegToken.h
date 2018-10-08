#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * SmartChineseAnalyzer internal token
 * @lucene.experimental
 */
class SegToken : public std::enable_shared_from_this<SegToken>
{
  GET_CLASS_NAME(SegToken)
  /**
   * Character array containing token text
   */
public:
  std::deque<wchar_t> charArray;

  /**
   * start offset into original sentence
   */
  int startOffset = 0;

  /**
   * end offset into original sentence
   */
  int endOffset = 0;

  /**
   * {@link WordType} of the text
   */
  int wordType = 0;

  /**
   * word frequency
   */
  int weight = 0;

  /**
   * during segmentation, this is used to store the index of the token in the
   * token deque table
   */
  int index = 0;

  /**
   * Create a new SegToken from a character array.
   *
   * @param idArray character array containing text
   * @param start start offset of SegToken in original sentence
   * @param end end offset of SegToken in original sentence
   * @param wordType {@link WordType} of the text
   * @param weight word frequency
   */
  SegToken(std::deque<wchar_t> &idArray, int start, int end, int wordType,
           int weight);

  /**
   * @see java.lang.Object#hashCode()
   */
  virtual int hashCode();

  /**
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::analysis::cn::smart::hhmm
