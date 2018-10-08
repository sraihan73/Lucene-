#pragma once
#include "../util/SegmentingTokenizerBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/CharArrayIterator.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::th
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharArrayIterator =
    org::apache::lucene::analysis::util::CharArrayIterator;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Tokenizer that use {@link BreakIterator} to tokenize Thai text.
 * <p>WARNING: this tokenizer may not be supported by all JREs.
 *    It is known to work with Sun/Oracle and Harmony JREs.
 *    If your application needs to be fully portable, consider using
 * ICUTokenizer instead, which uses an ICU Thai BreakIterator that will always
 * be available.
 */
class ThaiTokenizer : public SegmentingTokenizerBase
{
  GET_CLASS_NAME(ThaiTokenizer)
  /**
   * True if the JRE supports a working dictionary-based breakiterator for Thai.
   * If this is false, this tokenizer will not work at all!
   */
public:
  static const bool DBBI_AVAILABLE = false;

private:
  static const std::shared_ptr<BreakIterator> proto;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static ThaiTokenizer::StaticConstructor staticConstructor;

  /** used for breaking the text into sentences */
  static const std::shared_ptr<BreakIterator> sentenceProto;

  const std::shared_ptr<BreakIterator> wordBreaker;
  const std::shared_ptr<CharArrayIterator> wrapper =
      CharArrayIterator::newWordInstance();

public:
  int sentenceStart = 0;
  int sentenceEnd = 0;

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  /** Creates a new ThaiTokenizer */
public:
  ThaiTokenizer();

  /** Creates a new ThaiTokenizer, supplying the AttributeFactory */
  ThaiTokenizer(std::shared_ptr<AttributeFactory> factory);

protected:
  void setNextSentence(int sentenceStart, int sentenceEnd) override;

  bool incrementWord() override;

protected:
  std::shared_ptr<ThaiTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<ThaiTokenizer>(
        org.apache.lucene.analysis.util
            .SegmentingTokenizerBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/th/
