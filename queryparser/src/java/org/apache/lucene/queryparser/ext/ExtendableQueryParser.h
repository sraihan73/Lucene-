#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::ext
{
class Extensions;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::classic
{
class ParseException;
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
namespace org::apache::lucene::queryparser::ext
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;

/**
 * The {@link ExtendableQueryParser} enables arbitrary query parser extension
 * based on a customizable field naming scheme. The lucene query syntax allows
 * implicit and explicit field definitions as query prefix followed by a colon
 * (':') character. The {@link ExtendableQueryParser} allows to encode extension
 * keys into the field symbol associated with a registered instance of
 * {@link ParserExtension}. A customizable separation character separates the
 * extension key from the actual field symbol. The {@link ExtendableQueryParser}
 * splits (@see {@link Extensions#splitExtensionField(std::wstring, std::wstring)}) the
 * extension key from the field symbol and tries to resolve the associated
 * {@link ParserExtension}. If the parser can't resolve the key or the field
 * token does not contain a separation character, {@link ExtendableQueryParser}
 * yields the same behavior as its super class {@link QueryParser}. Otherwise,
 * if the key is associated with a {@link ParserExtension} instance, the parser
GET_CLASS_NAME({@link)
 * builds an instance of {@link ExtensionQuery} to be processed by
 * {@link ParserExtension#parse(ExtensionQuery)}.If a extension field does not
 * contain a field part the default field for the query will be used.
 * <p>
 * To guarantee that an extension field is processed with its associated
 * extension, the extension query part must escape any special characters like
 * '*' or '['. If the extension query contains any whitespace characters, the
 * extension query part must be enclosed in quotes.
 * Example ('_' used as separation character):
 * <pre>
 *   title_customExt:"Apache Lucene\?" OR content_customExt:prefix\*
 * </pre>
 *
 * Search on the default field:
 * <pre>
 *   _customExt:"Apache Lucene\?" OR _customExt:prefix\*
 * </pre>
 * <p>
 * The {@link ExtendableQueryParser} itself does not implement the logic how
 * field and extension key are separated or ordered. All logic regarding the
 * extension key and field symbol parsing is located in {@link Extensions}.
 * Customized extension schemes should be implemented by sub-classing
 * {@link Extensions}.
GET_CLASS_NAME(ing)
 * </p>
 * <p>
 * For details about the default encoding scheme see {@link Extensions}.
 * </p>
 *
 * @see Extensions
 * @see ParserExtension
 * @see ExtensionQuery
 */
class ExtendableQueryParser : public QueryParser
{
  GET_CLASS_NAME(ExtendableQueryParser)

private:
  const std::wstring defaultField;
  const std::shared_ptr<Extensions> extensions;

  /**
   * Default empty extensions instance
   */
  static const std::shared_ptr<Extensions> DEFAULT_EXTENSION;

  /**
   * Creates a new {@link ExtendableQueryParser} instance
   *
   * @param f
   *          the default query field
   * @param a
   *          the analyzer used to find terms in a query string
   */
public:
  ExtendableQueryParser(const std::wstring &f, std::shared_ptr<Analyzer> a);

  /**
   * Creates a new {@link ExtendableQueryParser} instance
   *
   * @param f
   *          the default query field
   * @param a
   *          the analyzer used to find terms in a query string
   * @param ext
   *          the query parser extensions
   */
  ExtendableQueryParser(const std::wstring &f, std::shared_ptr<Analyzer> a,
                        std::shared_ptr<Extensions> ext);

  /**
   * Returns the extension field delimiter character.
   *
   * @return the extension field delimiter character.
   */
  virtual wchar_t getExtensionFieldDelimiter();

protected:
  std::shared_ptr<Query>
  getFieldQuery(const std::wstring &field, const std::wstring &queryText,
                bool quoted)  override;

protected:
  std::shared_ptr<ExtendableQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<ExtendableQueryParser>(
        org.apache.lucene.queryparser.classic.QueryParser::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::ext
