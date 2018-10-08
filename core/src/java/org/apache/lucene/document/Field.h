#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexableFieldType;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class BytesTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
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
namespace org::apache::lucene::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Expert: directly create a field for a document.  Most
 * users should use one of the sugar subclasses:
 * <ul>
 *    <li>{@link TextField}: {@link Reader} or {@link std::wstring} indexed for
 * full-text search <li>{@link StringField}: {@link std::wstring} indexed verbatim as
 * a single token <li>{@link IntPoint}: {@code int} indexed for exact/range
 * queries. <li>{@link LongPoint}: {@code long} indexed for exact/range queries.
 *    <li>{@link FloatPoint}: {@code float} indexed for exact/range queries.
 *    <li>{@link DoublePoint}: {@code double} indexed for exact/range queries.
 *    <li>{@link SortedDocValuesField}: {@code byte[]} indexed column-wise for
 * sorting/faceting <li>{@link SortedSetDocValuesField}: {@code
 * SortedSet<byte[]>} indexed column-wise for sorting/faceting <li>{@link
 * NumericDocValuesField}: {@code long} indexed column-wise for sorting/faceting
 *    <li>{@link SortedNumericDocValuesField}: {@code SortedSet<long>} indexed
 * column-wise for sorting/faceting <li>{@link StoredField}: Stored-only value
 * for retrieving in summary results
 * </ul>
 *
 * <p> A field is a section of a Document. Each field has three
 * parts: name, type and value. Values may be text
 * (std::wstring, Reader or pre-analyzed TokenStream), binary
 * (byte[]), or numeric (a Number).  Fields are optionally stored in the
 * index, so that they may be returned with hits on the document.
 *
 * <p>
 * NOTE: the field type is an {@link IndexableFieldType}.  Making changes
 * to the state of the IndexableFieldType will impact any
 * Field it is used in.  It is strongly recommended that no
 * changes be made after Field instantiation.
 */
class Field : public std::enable_shared_from_this<Field>, public IndexableField
{
  GET_CLASS_NAME(Field)

  /**
   * Field's type
   */
protected:
  const std::shared_ptr<IndexableFieldType> type;

  /**
   * Field's name
   */
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::wstring name_;

  /** Field's value */
  std::any fieldsData;

  /** Pre-analyzed tokenStream for indexed fields; this is
   * separate from fieldsData because you are allowed to
   * have both; eg maybe field has a std::wstring value but you
   * customize how it's tokenized */
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<TokenStream> tokenStream_;

  /**
   * Expert: creates a field with no initial value.
   * Intended only for custom Field subclasses.
   * @param name field name
   * @param type field type
   * @throws IllegalArgumentException if either the name or type
   *         is null.
   */
  Field(const std::wstring &name, std::shared_ptr<IndexableFieldType> type);

  /**
   * Create field with Reader value.
   * @param name field name
   * @param reader reader value
   * @param type field type
   * @throws IllegalArgumentException if either the name or type
   *         is null, or if the field's type is stored(), or
   *         if tokenized() is false.
   * @throws NullPointerException if the reader is null
   */
public:
  Field(const std::wstring &name, std::shared_ptr<Reader> reader,
        std::shared_ptr<IndexableFieldType> type);

  /**
   * Create field with TokenStream value.
   * @param name field name
   * @param tokenStream TokenStream value
   * @param type field type
   * @throws IllegalArgumentException if either the name or type
   *         is null, or if the field's type is stored(), or
   *         if tokenized() is false, or if indexed() is false.
   * @throws NullPointerException if the tokenStream is null
   */
  Field(const std::wstring &name, std::shared_ptr<TokenStream> tokenStream,
        std::shared_ptr<IndexableFieldType> type);

  /**
   * Create field with binary value.
   *
   * <p>NOTE: the provided byte[] is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param value byte array pointing to binary content (not copied)
   * @param type field type
   * @throws IllegalArgumentException if the field name is null,
   *         or the field's type is indexed()
   * @throws NullPointerException if the type is null
   */
  Field(const std::wstring &name, std::deque<char> &value,
        std::shared_ptr<IndexableFieldType> type);

  /**
   * Create field with binary value.
   *
   * <p>NOTE: the provided byte[] is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param value byte array pointing to binary content (not copied)
   * @param offset starting position of the byte array
   * @param length valid length of the byte array
   * @param type field type
   * @throws IllegalArgumentException if the field name is null,
   *         or the field's type is indexed()
   * @throws NullPointerException if the type is null
   */
  Field(const std::wstring &name, std::deque<char> &value, int offset,
        int length, std::shared_ptr<IndexableFieldType> type);

  /**
   * Create field with binary value.
   *
   * <p>NOTE: the provided BytesRef is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param bytes BytesRef pointing to binary content (not copied)
   * @param type field type
   * @throws IllegalArgumentException if the field name is null,
   *         or the field's type is indexed()
   * @throws NullPointerException if the type is null
   */
  Field(const std::wstring &name, std::shared_ptr<BytesRef> bytes,
        std::shared_ptr<IndexableFieldType> type);

  // TODO: allow direct construction of int, long, float, double value too..?

  /**
   * Create field with std::wstring value.
   * @param name field name
   * @param value string value
   * @param type field type
   * @throws IllegalArgumentException if either the name or value
   *         is null, or if the field's type is neither indexed() nor stored(),
   *         or if indexed() is false but storeTermVectors() is true.
   * @throws NullPointerException if the type is null
   */
  Field(const std::wstring &name, const std::wstring &value,
        std::shared_ptr<IndexableFieldType> type);

  /**
   * The value of the field as a std::wstring, or null. If null, the Reader value or
   * binary value is used. Exactly one of stringValue(), readerValue(), and
   * binaryValue() must be set.
   */
  std::wstring stringValue() override;

  /**
   * The value of the field as a Reader, or null. If null, the std::wstring value or
   * binary value is used. Exactly one of stringValue(), readerValue(), and
   * binaryValue() must be set.
   */
  std::shared_ptr<Reader> readerValue() override;

  /**
   * The TokenStream for this field to be used when indexing, or null. If null,
   * the Reader value or std::wstring value is analyzed to produce the indexed tokens.
   */
  virtual std::shared_ptr<TokenStream> tokenStreamValue();

  /**
   * <p>
   * Expert: change the value of this field. This can be used during indexing to
   * re-use a single Field instance to improve indexing speed by avoiding GC
   * cost of new'ing and reclaiming Field instances. Typically a single
   * {@link Document} instance is re-used as well. This helps most on small
   * documents.
   * </p>
   *
   * <p>
   * Each Field instance should only be used once within a single
   * {@link Document} instance. See <a
   * href="http://wiki.apache.org/lucene-java/ImproveIndexingSpeed"
   * >ImproveIndexingSpeed</a> for details.
   * </p>
   */
  virtual void setStringValue(const std::wstring &value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setReaderValue(std::shared_ptr<Reader> value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setBytesValue(std::deque<char> &value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   *
   * <p>NOTE: the provided BytesRef is not copied so be sure
   * not to change it until you're done with this field.
   */
  virtual void setBytesValue(std::shared_ptr<BytesRef> value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setByteValue(char value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setShortValue(short value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setIntValue(int value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setLongValue(int64_t value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setFloatValue(float value);

  /**
   * Expert: change the value of this field. See
   * {@link #setStringValue(std::wstring)}.
   */
  virtual void setDoubleValue(double value);

  /**
   * Expert: sets the token stream to be used for indexing and causes
   * isIndexed() and isTokenized() to return true. May be combined with stored
   * values from stringValue() or binaryValue()
   */
  virtual void setTokenStream(std::shared_ptr<TokenStream> tokenStream);

  std::wstring name() override;

  std::shared_ptr<Number> numericValue() override;

  std::shared_ptr<BytesRef> binaryValue() override;

  /** Prints a Field for human consumption. */
  virtual std::wstring toString();

  /** Returns the {@link FieldType} for this field. */
  std::shared_ptr<IndexableFieldType> fieldType() override;

  std::shared_ptr<TokenStream>
  tokenStream(std::shared_ptr<Analyzer> analyzer,
              std::shared_ptr<TokenStream> reuse) override;

private:
  class BinaryTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(BinaryTokenStream)
  private:
    const std::shared_ptr<BytesTermAttribute> bytesAtt =
        addAttribute(BytesTermAttribute::typeid);
    bool used = true;
    std::shared_ptr<BytesRef> value;

    /** Creates a new TokenStream that returns a BytesRef as single token.
     * <p>Warning: Does not initialize the value, you must call
     * {@link #setValue(BytesRef)} afterwards!
     */
  public:
    BinaryTokenStream();

    void setValue(std::shared_ptr<BytesRef> value);

    bool incrementToken() override;

    void reset() override;

    virtual ~BinaryTokenStream();

  protected:
    std::shared_ptr<BinaryTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<BinaryTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  class StringTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(StringTokenStream)
  private:
    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAttribute =
        addAttribute(OffsetAttribute::typeid);
    bool used = true;
    std::wstring value = L"";

    /** Creates a new TokenStream that returns a std::wstring as single token.
     * <p>Warning: Does not initialize the value, you must call
     * {@link #setValue(std::wstring)} afterwards!
     */
  public:
    StringTokenStream();

    /** Sets the string value. */
    void setValue(const std::wstring &value);

    bool incrementToken() override;

    void end()  override;

    void reset() override;

    virtual ~StringTokenStream();

  protected:
    std::shared_ptr<StringTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<StringTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

  /** Specifies whether and how a field should be stored. */
public:
  enum class Store {
    GET_CLASS_NAME(Store)

    /** Store the original field value in the index. This is useful for short
     * texts like a document's title which should be displayed with the results.
     * The value is stored in its original form, i.e. no analyzer is used before
     * it is stored.
     */
    YES,

    /** Do not store the field value in the index. */
    NO
  };
};

} // namespace org::apache::lucene::document
