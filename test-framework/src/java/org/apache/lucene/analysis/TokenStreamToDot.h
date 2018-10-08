#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
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
namespace org::apache::lucene::analysis
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;

/** Consumes a TokenStream and outputs the dot (graphviz) string (graph). */
class TokenStreamToDot : public std::enable_shared_from_this<TokenStreamToDot>
{
  GET_CLASS_NAME(TokenStreamToDot)

private:
  const std::shared_ptr<TokenStream> in_;
  const std::shared_ptr<CharTermAttribute> termAtt;
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt;
  const std::shared_ptr<PositionLengthAttribute> posLengthAtt;
  const std::shared_ptr<OffsetAttribute> offsetAtt;
  const std::wstring inputText;

protected:
  const std::shared_ptr<PrintWriter> out;

  /** If inputText is non-null, and the TokenStream has
   *  offsets, we include the surface form in each arc's
   *  label. */
public:
  TokenStreamToDot(const std::wstring &inputText,
                   std::shared_ptr<TokenStream> in_,
                   std::shared_ptr<PrintWriter> out);

  virtual void toDot() ;

protected:
  virtual void writeArc(int fromNode, int toNode, const std::wstring &label,
                        const std::wstring &style);

  virtual void writeNode(int name, const std::wstring &label);

private:
  static const std::wstring FONT_NAME;

  /** Override to customize. */
protected:
  virtual void writeHeader();

  /** Override to customize. */
  virtual void writeTrailer();
};

} // namespace org::apache::lucene::analysis
