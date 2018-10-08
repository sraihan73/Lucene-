#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::util
{
class AttributeSource;
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

namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * A TokenStream that takes an array of input TokenStreams as sources, and
 * concatenates them together.
 *
 * Offsets from the second and subsequent sources are incremented to behave
 * as if all the inputs were from a single source.
 *
 * All of the input TokenStreams must have the same attribute implementations
 */
class ConcatenatingTokenStream final : public TokenStream
{
  GET_CLASS_NAME(ConcatenatingTokenStream)

private:
  std::deque<std::shared_ptr<TokenStream>> const sources;
  std::deque<std::shared_ptr<OffsetAttribute>> const sourceOffsets;
  const std::shared_ptr<OffsetAttribute> offsetAtt;

  int currentSource = 0;
  int offsetIncrement = 0;

  /**
   * Create a new ConcatenatingTokenStream from a set of inputs
   * @param sources an array of TokenStream inputs to concatenate
   */
public:
  ConcatenatingTokenStream(std::deque<TokenStream> &sources);

private:
  static std::shared_ptr<AttributeSource>
  combineSources(std::deque<TokenStream> &sources);

public:
  bool incrementToken()  override;

  void end()  override;

  void reset()  override;

  virtual ~ConcatenatingTokenStream();

protected:
  std::shared_ptr<ConcatenatingTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<ConcatenatingTokenStream>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
