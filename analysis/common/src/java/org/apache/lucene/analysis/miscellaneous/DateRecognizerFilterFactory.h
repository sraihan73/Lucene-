#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
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
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link DateRecognizerFilter}.
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_filter_none_date" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.DateRecognizerFilterFactory" datePattern="yyyy/mm/dd"
 * locale="en-US" /&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;
 * </pre>
 *
 * <p>
 * The {@code datePattern} is optional. If omitted, {@link DateRecognizerFilter}
 * will be created with the default date format of the system. The {@code
 * locale} is optional and if omitted the filter will be created with
 * {@link Locale#ENGLISH}.
 * @since 5.5.0
 */
class DateRecognizerFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(DateRecognizerFilterFactory)

public:
  static const std::wstring DATE_PATTERN;
  static const std::wstring LOCALE;

private:
  const std::shared_ptr<DateFormat> dateFormat;
  const std::shared_ptr<Locale> locale;

  /** Creates a new FingerprintFilterFactory */
public:
  DateRecognizerFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

private:
  std::shared_ptr<Locale> getLocale(const std::wstring &localeStr);

public:
  virtual std::shared_ptr<DateFormat>
  getDataFormat(const std::wstring &datePattern);

protected:
  std::shared_ptr<DateRecognizerFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<DateRecognizerFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
