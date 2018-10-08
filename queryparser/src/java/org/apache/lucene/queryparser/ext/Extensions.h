#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/ext/ParserExtension.h"

namespace org::apache::lucene::queryparser::ext
{
template <typename Curtypename Cud>
class Pair;
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

/**
 * The {@link Extensions} class represents an extension mapping to associate
 * {@link ParserExtension} instances with extension keys. An extension key is a
GET_CLASS_NAME(represents)
 * string encoded into a Lucene standard query parser field symbol recognized by
 * {@link ExtendableQueryParser}. The query parser passes each extension field
 * token to {@link #splitExtensionField(std::wstring, std::wstring)} to separate the
 * extension key from the field identifier.
 * <p>
 * In addition to the key to extension mapping this class also defines the field
 * name overloading scheme. {@link ExtendableQueryParser} uses the given
GET_CLASS_NAME(also)
 * extension to split the actual field name and extension key by calling
 * {@link #splitExtensionField(std::wstring, std::wstring)}. To change the order or the key
 * / field name encoding scheme users can subclass {@link Extensions} to
 * implement their own.
 *
 * @see ExtendableQueryParser
 * @see ParserExtension
 */
class Extensions : public std::enable_shared_from_this<Extensions>
{
  GET_CLASS_NAME(Extensions)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<ParserExtension>>
      extensions =
          std::unordered_map<std::wstring, std::shared_ptr<ParserExtension>>();
  const wchar_t extensionFieldDelimiter;
  /**
   * The default extension field delimiter character. This constant is set to
   * ':'
   */
public:
  static constexpr wchar_t DEFAULT_EXTENSION_FIELD_DELIMITER = L':';

  /**
   * Creates a new {@link Extensions} instance with the
   * {@link #DEFAULT_EXTENSION_FIELD_DELIMITER} as a delimiter character.
   */
  Extensions();

  /**
   * Creates a new {@link Extensions} instance
   *
   * @param extensionFieldDelimiter
   *          the extensions field delimiter character
   */
  Extensions(wchar_t extensionFieldDelimiter);

  /**
   * Adds a new {@link ParserExtension} instance associated with the given key.
   *
   * @param key
   *          the parser extension key
   * @param extension
   *          the parser extension
   */
  virtual void add(const std::wstring &key,
                   std::shared_ptr<ParserExtension> extension);

  /**
   * Returns the {@link ParserExtension} instance for the given key or
   * <code>null</code> if no extension can be found for the key.
   *
   * @param key
   *          the extension key
   * @return the {@link ParserExtension} instance for the given key or
   *         <code>null</code> if no extension can be found for the key.
   */
  std::shared_ptr<ParserExtension> getExtension(const std::wstring &key);

  /**
   * Returns the extension field delimiter
   *
   * @return the extension field delimiter
   */
  virtual wchar_t getExtensionFieldDelimiter();

  /**
   * Splits a extension field and returns the field / extension part as a
   * {@link Pair}. This method tries to split on the first occurrence of the
   * extension field delimiter, if the delimiter is not present in the string
   * the result will contain a <code>null</code> value for the extension key and
   * the given field string as the field value. If the given extension field
   * string contains no field identifier the result pair will carry the given
   * default field as the field value.
   *
   * @param defaultField
   *          the default query field
   * @param field
   *          the extension field string
   * @return a {@link Pair} with the field name as the {@link Pair#cur} and the
   *         extension key as the {@link Pair#cud}
   */
  virtual std::shared_ptr<Pair<std::wstring, std::wstring>>
  splitExtensionField(const std::wstring &defaultField,
                      const std::wstring &field);

  /**
   * Escapes an extension field. The default implementation is equivalent to
   * {@link QueryParser#escape(std::wstring)}.
   *
   * @param extfield
   *          the extension field identifier
   * @return the extension field identifier with all special chars escaped with
   *         a backslash character.
   */
  virtual std::wstring escapeExtensionField(const std::wstring &extfield);

  /**
   * Builds an extension field string from a given extension key and the default
   * query field. The default field and the key are delimited with the extension
   * field delimiter character. This method makes no assumption about the order
   * of the extension key and the field. By default the extension key is
   * appended to the end of the returned string while the field is added to the
   * beginning. Special Query characters are escaped in the result.
   * <p>
   * Note: {@link Extensions} subclasses must maintain the contract between
   * {@link #buildExtensionField(std::wstring)} and
GET_CLASS_NAME(es)
   * {@link #splitExtensionField(std::wstring, std::wstring)} where the latter inverts the
   * former.
   * </p>
   */
  virtual std::wstring buildExtensionField(const std::wstring &extensionKey);

  /**
   * Builds an extension field string from a given extension key and the
   * extensions field. The field and the key are delimited with the extension
   * field delimiter character. This method makes no assumption about the order
   * of the extension key and the field. By default the extension key is
   * appended to the end of the returned string while the field is added to the
   * beginning. Special Query characters are escaped in the result.
   * <p>
   * Note: {@link Extensions} subclasses must maintain the contract between
   * {@link #buildExtensionField(std::wstring, std::wstring)} and
GET_CLASS_NAME(es)
   * {@link #splitExtensionField(std::wstring, std::wstring)} where the latter inverts the
   * former.
   * </p>
   *
   * @param extensionKey
   *          the extension key
   * @param field
   *          the field to apply the extension on.
   * @return escaped extension field identifier
   * @see #buildExtensionField(std::wstring) to use the default query field
   */
  virtual std::wstring buildExtensionField(const std::wstring &extensionKey,
                                           const std::wstring &field);

  /**
   * This class represents a generic pair.
   *
   * @param <Cur>
   *          the pairs first element
   * @param <Cud>
   *          the pairs last element of the pair.
   */
public:
  template <typename Cur, typename Cud>
  class Pair : public std::enable_shared_from_this<Pair>
  {
    GET_CLASS_NAME(Pair)

  public:
    const Cur cur;
    const Cud cud;

    /**
     * Creates a new Pair
     *
     * @param cur
     *          the pairs first element
     * @param cud
     *          the pairs last element
     */
    Pair(Cur cur, Cud cud) : cur(cur), cud(cud) {}
  };
};

} // #include  "core/src/java/org/apache/lucene/queryparser/ext/
