#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::highlight
{
class TokenGroup;
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
namespace org::apache::lucene::search::highlight
{

/**
 * Simple {@link Formatter} implementation to highlight terms with a pre and
 * post tag.
 */
class SimpleHTMLFormatter
    : public std::enable_shared_from_this<SimpleHTMLFormatter>,
      public Formatter
{
  GET_CLASS_NAME(SimpleHTMLFormatter)

private:
  static const std::wstring DEFAULT_PRE_TAG;
  static const std::wstring DEFAULT_POST_TAG;

  std::wstring preTag;
  std::wstring postTag;

public:
  SimpleHTMLFormatter(const std::wstring &preTag, const std::wstring &postTag);

  /** Default constructor uses HTML: &lt;B&gt; tags to markup terms. */
  SimpleHTMLFormatter();

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.search.highlight.Formatter#highlightTerm(java.lang.std::wstring,
   * org.apache.lucene.search.highlight.TokenGroup)
   */
  std::wstring highlightTerm(const std::wstring &originalText,
                             std::shared_ptr<TokenGroup> tokenGroup) override;
};

} // namespace org::apache::lucene::search::highlight
