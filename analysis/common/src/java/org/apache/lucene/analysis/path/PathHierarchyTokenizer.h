#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeFactory;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
 * Tokenizer for path-like hierarchies.
 * <p>
 * Take something like:
 *
 * <pre>
 *  /something/something/else
 * </pre>
 *
 * and make:
 *
 * <pre>
 *  /something
 *  /something/something
 *  /something/something/else
 * </pre>
 */
class PathHierarchyTokenizer : public Tokenizer
{
  GET_CLASS_NAME(PathHierarchyTokenizer)

public:
  PathHierarchyTokenizer();

  PathHierarchyTokenizer(int skip);

  PathHierarchyTokenizer(int bufferSize, wchar_t delimiter);

  PathHierarchyTokenizer(wchar_t delimiter, wchar_t replacement);

  PathHierarchyTokenizer(wchar_t delimiter, wchar_t replacement, int skip);

  PathHierarchyTokenizer(std::shared_ptr<AttributeFactory> factory,
                         wchar_t delimiter, wchar_t replacement, int skip);

  PathHierarchyTokenizer(int bufferSize, wchar_t delimiter, wchar_t replacement,
                         int skip);

  PathHierarchyTokenizer(std::shared_ptr<AttributeFactory> factory,
                         int bufferSize, wchar_t delimiter, wchar_t replacement,
                         int skip);

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
  int startPosition = 0;
  int skipped = 0;
  bool endDelimiter = false;
  std::shared_ptr<StringBuilder> resultToken;

  int charsRead = 0;

public:
  bool incrementToken()  override final;

  void end()  override final;

  void reset()  override;

protected:
  std::shared_ptr<PathHierarchyTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<PathHierarchyTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::path
