#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionTokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
namespace org::apache::lucene::search::suggest::document
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * {@link SuggestField} which additionally takes in a set of
 * contexts. Example usage of adding a suggestion with contexts is as follows:
 *
 * <pre class="prettyprint">
 *  document.add(
 *   new ContextSuggestField(name, "suggestion", Arrays.asList("context1",
 * "context2"),  4));
 * </pre>
 *
 * Use {@link ContextQuery} to boost and/or filter suggestions
 * at query-time. Use {@link PrefixCompletionQuery}, {@link
 * RegexCompletionQuery} or {@link FuzzyCompletionQuery} if context
 * boost/filtering are not needed.
 *
 * @lucene.experimental
 */
class ContextSuggestField : public SuggestField
{
  GET_CLASS_NAME(ContextSuggestField)

  /**
   * Separator used between context value and the suggest field value
   */
public:
  static constexpr int CONTEXT_SEPARATOR = L'\u001D';
  static constexpr char TYPE = 1;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Set<std::shared_ptr<std::wstring>>> contexts_;

  /**
   * Creates a context-enabled suggest field
   *
   * @param name field name
   * @param value field value to get suggestion on
   * @param weight field weight
   * @param contexts associated contexts
   *
   * @throws IllegalArgumentException if either the name or value is null,
   * if value is an empty string, if the weight is negative, if value or
   * contexts contains any reserved characters
   */
public:
  ContextSuggestField(const std::wstring &name, const std::wstring &value,
                      int weight, std::deque<std::wstring> &contexts);

  /**
   * Expert: Sub-classes can inject contexts at
   * index-time
   */
protected:
  virtual std::deque<std::shared_ptr<std::wstring>> contexts();

  std::shared_ptr<CompletionTokenStream>
  wrapTokenStream(std::shared_ptr<TokenStream> stream) override;

  char type() override;

  /**
   * The {@link PrefixTokenFilter} wraps a {@link TokenStream} and adds a set
   * prefixes ahead. The position attribute will not be incremented for the
   * prefixes.
   */
private:
  class PrefixTokenFilter final : public TokenFilter
  {
    GET_CLASS_NAME(PrefixTokenFilter)

  private:
    const wchar_t separator;
    const std::shared_ptr<CharTermAttribute> termAttr =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posAttr =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::deque<std::shared_ptr<std::wstring>> prefixes;

    std::shared_ptr<Iterator<std::shared_ptr<std::wstring>>> currentPrefix;

    /**
     * Create a new {@link PrefixTokenFilter}
     *
     * @param input {@link TokenStream} to wrap
     * @param separator Character used separate prefixes from other tokens
     * @param prefixes {@link Iterable} of {@link std::wstring} which keeps all
     * prefixes
     */
  public:
    PrefixTokenFilter(std::shared_ptr<TokenStream> input, wchar_t separator,
                      std::deque<std::shared_ptr<std::wstring>> &prefixes);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PrefixTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<PrefixTokenFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

private:
  void validate(std::shared_ptr<std::wstring> value);

protected:
  std::shared_ptr<ContextSuggestField> shared_from_this()
  {
    return std::static_pointer_cast<ContextSuggestField>(
        SuggestField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
