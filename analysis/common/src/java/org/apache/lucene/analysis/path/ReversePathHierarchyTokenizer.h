#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

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
namespace org::apache::lucene::analysis::path
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Tokenizer for domain-like hierarchies.
 * <p>
 * Take something like:
 *
 * <pre>
 * www.site.co.uk
 * </pre>
 *
 * and make:
 *
 * <pre>
 * www.site.co.uk
 * site.co.uk
 * co.uk
 * uk
 * </pre>
 *
 */
class ReversePathHierarchyTokenizer : public Tokenizer
{
  GET_CLASS_NAME(ReversePathHierarchyTokenizer)

public:
  ReversePathHierarchyTokenizer();

  ReversePathHierarchyTokenizer(int skip);

  ReversePathHierarchyTokenizer(int bufferSize, wchar_t delimiter);

  ReversePathHierarchyTokenizer(wchar_t delimiter, wchar_t replacement);

  ReversePathHierarchyTokenizer(int bufferSize, wchar_t delimiter,
                                wchar_t replacement);

  ReversePathHierarchyTokenizer(wchar_t delimiter, int skip);

  ReversePathHierarchyTokenizer(wchar_t delimiter, wchar_t replacement,
                                int skip);

  ReversePathHierarchyTokenizer(std::shared_ptr<AttributeFactory> factory,
                                wchar_t delimiter, wchar_t replacement,
                                int skip);

  ReversePathHierarchyTokenizer(int bufferSize, wchar_t delimiter,
                                wchar_t replacement, int skip);
  ReversePathHierarchyTokenizer(std::shared_ptr<AttributeFactory> factory,
                                int bufferSize, wchar_t delimiter,
                                wchar_t replacement, int skip);

private:
  static constexpr int DEFAULT_BUFFER_SIZE = 1024;

public:
  static constexpr wchar_t DEFAULT_DELIMITER = L'/';
  static constexpr int DEFAULT_SKIP = 0;

private:
  const wchar_t delimiter;
  const wchar_t replacement;
  const int skip;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  int endPosition = 0;
  int finalOffset = 0;
  int skipped = 0;
  std::shared_ptr<StringBuilder> resultToken;

  std::deque<int> delimiterPositions;
  int delimitersCount = -1;
  std::deque<wchar_t> resultTokenBuffer;

public:
  bool incrementToken()  override final;

  void end()  override final;

  void reset()  override;

protected:
  std::shared_ptr<ReversePathHierarchyTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<ReversePathHierarchyTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/path/
