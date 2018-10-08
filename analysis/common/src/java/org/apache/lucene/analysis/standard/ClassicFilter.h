#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

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
namespace org::apache::lucene::analysis::standard
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/** Normalizes tokens extracted with {@link ClassicTokenizer}. */

class ClassicFilter : public TokenFilter
{
  GET_CLASS_NAME(ClassicFilter)

  /** Construct filtering <i>in</i>. */
public:
  ClassicFilter(std::shared_ptr<TokenStream> in_);

private:
  static const std::wstring APOSTROPHE_TYPE;
  static const std::wstring ACRONYM_TYPE;

  // this filters uses attribute type
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /** Returns the next token in the stream, or null at EOS.
   * <p>Removes <tt>'s</tt> from the end of words.
   * <p>Removes dots from acronyms.
   */
public:
  bool incrementToken()  override final;

protected:
  std::shared_ptr<ClassicFilter> shared_from_this()
  {
    return std::static_pointer_cast<ClassicFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
