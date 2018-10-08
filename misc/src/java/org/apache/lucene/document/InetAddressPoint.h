#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class Query;
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

using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * An indexed 128-bit {@code InetAddress} field.
 * <p>
 * Finding all documents within a range at search time is
 * efficient.  Multiple values for the same field in one document
 * is allowed.
 * <p>
 * This field defines static factory methods for creating common queries:
 * <ul>
 *   <li>{@link #newExactQuery(std::wstring, InetAddress)} for matching an exact
 * network address. <li>{@link #newPrefixQuery(std::wstring, InetAddress, int)} for
 * matching a network based on CIDR prefix. <li>{@link #newRangeQuery(std::wstring,
 * InetAddress, InetAddress)} for matching arbitrary network address ranges.
 *   <li>{@link #newSetQuery(std::wstring, InetAddress...)} for matching a set of
 * network addresses.
 * </ul>
 * <p>
 * This field supports both IPv4 and IPv6 addresses: IPv4 addresses are
 * converted to <a
 * href="https://tools.ietf.org/html/rfc4291#section-2.5.5">IPv4-Mapped IPv6
 * Addresses</a>: indexing {@code 1.2.3.4} is the same as indexing {@code
 * ::FFFF:1.2.3.4}.
 * @see PointValues
 */
class InetAddressPoint : public Field
{
  GET_CLASS_NAME(InetAddressPoint)

  // implementation note: we convert all addresses to IPv6: we expect prefix
  // compression of values, so its not wasteful, but allows one field to handle
  // both IPv4 and IPv6.
  /** The number of bytes per dimension: 128 bits */
public:
  static constexpr int BYTES = 16;

  // rfc4291 prefix
  static std::deque<char> const IPV4_PREFIX;

private:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static InetAddressPoint::StaticConstructor staticConstructor;

  /** The minimum value that an ip address can hold. */
public:
  static const std::shared_ptr<InetAddress> MIN_VALUE;
  /** The maximum value that an ip address can hold. */
  static const std::shared_ptr<InetAddress> MAX_VALUE;

  /**
   * Return the {@link InetAddress} that compares immediately greater than
   * {@code address}.
   * @throws ArithmeticException if the provided address is the
   *              {@link #MAX_VALUE maximum ip address}
   */
  static std::shared_ptr<InetAddress>
  nextUp(std::shared_ptr<InetAddress> address);

  /**
   * Return the {@link InetAddress} that compares immediately less than
   * {@code address}.
   * @throws ArithmeticException if the provided address is the
   *              {@link #MIN_VALUE minimum ip address}
   */
  static std::shared_ptr<InetAddress>
  nextDown(std::shared_ptr<InetAddress> address);

  /** Change the values of this field */
  virtual void setInetAddressValue(std::shared_ptr<InetAddress> value);

  void setBytesValue(std::shared_ptr<BytesRef> bytes) override;

  /** Creates a new InetAddressPoint, indexing the
   *  provided address.
   *
   *  @param name field name
   *  @param point InetAddress value
   *  @throws IllegalArgumentException if the field name or value is null.
   */
  InetAddressPoint(const std::wstring &name,
                   std::shared_ptr<InetAddress> point);

  virtual std::wstring toString();

  // public helper methods (e.g. for queries)

  /** Encode InetAddress value into binary encoding */
  static std::deque<char> encode(std::shared_ptr<InetAddress> value);

  /** Decodes InetAddress value from binary encoding */
  static std::shared_ptr<InetAddress> decode(std::deque<char> &value);

  // static methods for generating queries

  /**
   * Create a query for matching a network address.
   *
   * @param field field name. must not be {@code null}.
   * @param value exact value
   * @throws IllegalArgumentException if {@code field} is null.
   * @return a query matching documents with this exact value
   */
  static std::shared_ptr<Query>
  newExactQuery(const std::wstring &field, std::shared_ptr<InetAddress> value);

  /**
   * Create a prefix query for matching a CIDR network range.
   *
   * @param field field name. must not be {@code null}.
   * @param value any host address
   * @param prefixLength the network prefix length for this address. This is
   * also known as the subnet mask in the context of IPv4 addresses.
   * @throws IllegalArgumentException if {@code field} is null, or prefixLength
   * is invalid.
   * @return a query matching documents with addresses contained within this
   * network
   */
  static std::shared_ptr<Query>
  newPrefixQuery(const std::wstring &field, std::shared_ptr<InetAddress> value,
                 int prefixLength);

  /**
   * Create a range query for network addresses.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue = InetAddressPoint.MIN_VALUE} or
   * {@code upperValue = InetAddressPoint.MAX_VALUE}.
   * <p> Ranges are inclusive. For exclusive ranges, pass {@code
   * InetAddressPoint#nextUp(lowerValue)} or {@code
   * InetAddressPoint#nexDown(upperValue)}.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive). must not be null.
   * @param upperValue upper portion of the range (inclusive). must not be null.
   * @throws IllegalArgumentException if {@code field} is null, {@code
   * lowerValue} is null, or {@code upperValue} is null
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field,
                std::shared_ptr<InetAddress> lowerValue,
                std::shared_ptr<InetAddress> upperValue);

private:
  class PointRangeQueryAnonymousInnerClass : public PointRangeQuery
  {
    GET_CLASS_NAME(PointRangeQueryAnonymousInnerClass)
  public:
    PointRangeQueryAnonymousInnerClass(const std::wstring &field,
                                       std::deque<char> &encode,
                                       std::deque<char> &encode);

  protected:
    std::wstring toString(int dimension, std::deque<char> &value) override;

  protected:
    std::shared_ptr<PointRangeQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointRangeQueryAnonymousInnerClass>(
          org.apache.lucene.search.PointRangeQuery::shared_from_this());
    }
  };

  /**
   * Create a query matching any of the specified 1D values.  This is the points
   * equivalent of {@code TermsQuery}.
   *
   * @param field field name. must not be {@code null}.
   * @param values all values to match
   */
public:
  static std::shared_ptr<Query> newSetQuery(const std::wstring &field,
                                            std::deque<InetAddress> &values);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<char>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::deque<char> &a, std::deque<char> &b);
  };

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<std::deque<char>> sortedValues;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(std::deque<std::deque<char>> &sortedValues,
                              std::shared_ptr<BytesRef> encoded);

    int upto = 0;

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass>(
          org.apache.lucene.search.PointInSetQuery.Stream::shared_from_this());
    }
  };

protected:
  std::shared_ptr<InetAddressPoint> shared_from_this()
  {
    return std::static_pointer_cast<InetAddressPoint>(
        Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
