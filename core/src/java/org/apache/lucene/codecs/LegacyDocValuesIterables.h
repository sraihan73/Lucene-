#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedDocValues;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::codecs
{
class NormsProducer;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
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

namespace org::apache::lucene::codecs
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Bridge helper methods for legacy codecs to map_obj sorted doc values to
 * iterables. */

class LegacyDocValuesIterables
    : public std::enable_shared_from_this<LegacyDocValuesIterables>
{
  GET_CLASS_NAME(LegacyDocValuesIterables)

private:
  LegacyDocValuesIterables();

  /** Converts {@link SortedDocValues} into an {@code Iterable&lt;BytesRef&gt;}
   * for all the values.
   *
   * @deprecated Consume {@link SortedDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // Iterable<org.apache.lucene.util.BytesRef> valuesIterable(final
  // org.apache.lucene.index.SortedDocValues values)
  static std::deque<std::shared_ptr<BytesRef>>
  valuesIterable(std::shared_ptr<SortedDocValues> values);

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<SortedDocValues> values;

  public:
    IterableAnonymousInnerClass(std::shared_ptr<SortedDocValues> values);

    std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<BytesRef>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance);

    private:
      int nextOrd = 0;

    public:
      bool hasNext();

      std::shared_ptr<BytesRef> next();
    };
  };

  /** Converts {@link SortedSetDocValues} into an {@code
   * Iterable&lt;BytesRef&gt;} for all the values.
   *
   * @deprecated Consume {@link SortedSetDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // Iterable<org.apache.lucene.util.BytesRef> valuesIterable(final
  // org.apache.lucene.index.SortedSetDocValues values)
  static std::deque<std::shared_ptr<BytesRef>>
  valuesIterable(std::shared_ptr<SortedSetDocValues> values);

private:
  class IterableAnonymousInnerClass2
      : public std::enable_shared_from_this<IterableAnonymousInnerClass2>,
        public std::deque<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass2)
  private:
    std::shared_ptr<SortedSetDocValues> values;

  public:
    IterableAnonymousInnerClass2(std::shared_ptr<SortedSetDocValues> values);

    std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> iterator();

  private:
    class IteratorAnonymousInnerClass2
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass2>,
          public Iterator<std::shared_ptr<BytesRef>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass2)
    private:
      std::shared_ptr<IterableAnonymousInnerClass2> outerInstance;

    public:
      IteratorAnonymousInnerClass2(
          std::shared_ptr<IterableAnonymousInnerClass2> outerInstance);

    private:
      int64_t nextOrd = 0;

    public:
      bool hasNext();

      std::shared_ptr<BytesRef> next();
    };
  };

  /** Converts {@link SortedDocValues} into the ord for each document as an
   * {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link SortedDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // sortedOrdIterable(final DocValuesProducer valuesProducer,
  // org.apache.lucene.index.FieldInfo fieldInfo, int maxDoc)
  static std::deque<std::shared_ptr<Number>>
  sortedOrdIterable(std::shared_ptr<DocValuesProducer> valuesProducer,
                    std::shared_ptr<FieldInfo> fieldInfo, int maxDoc);

private:
  class IterableAnonymousInnerClass3
      : public std::enable_shared_from_this<IterableAnonymousInnerClass3>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass3)
  private:
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    std::shared_ptr<FieldInfo> fieldInfo;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass3(
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        std::shared_ptr<FieldInfo> fieldInfo, int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass3
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass3>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass3)
    private:
      std::shared_ptr<IterableAnonymousInnerClass3> outerInstance;

      std::shared_ptr<SortedDocValues> values;

    public:
      IteratorAnonymousInnerClass3(
          std::shared_ptr<IterableAnonymousInnerClass3> outerInstance,
          std::shared_ptr<SortedDocValues> values);

    private:
      int nextDocID = 0;

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts number-of-ords per document from {@link SortedSetDocValues} into
   * {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link SortedSetDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // sortedSetOrdCountIterable(final DocValuesProducer valuesProducer, final
  // org.apache.lucene.index.FieldInfo fieldInfo, final int maxDoc)
  static std::deque<std::shared_ptr<Number>>
  sortedSetOrdCountIterable(std::shared_ptr<DocValuesProducer> valuesProducer,
                            std::shared_ptr<FieldInfo> fieldInfo,
                            int const maxDoc);

private:
  class IterableAnonymousInnerClass4
      : public std::enable_shared_from_this<IterableAnonymousInnerClass4>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass4)
  private:
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    std::shared_ptr<FieldInfo> fieldInfo;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass4(
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        std::shared_ptr<FieldInfo> fieldInfo, int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass4
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass4>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass4)
    private:
      std::shared_ptr<IterableAnonymousInnerClass4> outerInstance;

      std::shared_ptr<SortedSetDocValues> values;

    public:
      IteratorAnonymousInnerClass4(
          std::shared_ptr<IterableAnonymousInnerClass4> outerInstance,
          std::shared_ptr<SortedSetDocValues> values);

    private:
      int nextDocID = 0;
      int ordCount = 0;

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts all concatenated ords (in docID order) from {@link
   * SortedSetDocValues} into {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link SortedSetDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // sortedSetOrdsIterable(final DocValuesProducer valuesProducer, final
  // org.apache.lucene.index.FieldInfo fieldInfo)
  static std::deque<std::shared_ptr<Number>>
  sortedSetOrdsIterable(std::shared_ptr<DocValuesProducer> valuesProducer,
                        std::shared_ptr<FieldInfo> fieldInfo);

private:
  class IterableAnonymousInnerClass5
      : public std::enable_shared_from_this<IterableAnonymousInnerClass5>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass5)
  private:
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    std::shared_ptr<FieldInfo> fieldInfo;

  public:
    IterableAnonymousInnerClass5(
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        std::shared_ptr<FieldInfo> fieldInfo);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass5
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass5>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass5)
    private:
      std::shared_ptr<IterableAnonymousInnerClass5> outerInstance;

      std::shared_ptr<SortedSetDocValues> values;

    public:
      IteratorAnonymousInnerClass5(
          std::shared_ptr<IterableAnonymousInnerClass5> outerInstance,
          std::shared_ptr<SortedSetDocValues> values);

    private:
      bool nextIsSet = false;
      int64_t nextOrd = 0;

      void setNext();

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts number-of-values per document from {@link SortedNumericDocValues}
   * into {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link SortedDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // sortedNumericToDocCount(final DocValuesProducer valuesProducer, final
  // org.apache.lucene.index.FieldInfo fieldInfo, int maxDoc)
  static std::deque<std::shared_ptr<Number>>
  sortedNumericToDocCount(std::shared_ptr<DocValuesProducer> valuesProducer,
                          std::shared_ptr<FieldInfo> fieldInfo, int maxDoc);

private:
  class IterableAnonymousInnerClass6
      : public std::enable_shared_from_this<IterableAnonymousInnerClass6>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass6)
  private:
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    std::shared_ptr<FieldInfo> fieldInfo;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass6(
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        std::shared_ptr<FieldInfo> fieldInfo, int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass6
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass6>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass6)
    private:
      std::shared_ptr<IterableAnonymousInnerClass6> outerInstance;

      std::shared_ptr<SortedNumericDocValues> values;

    public:
      IteratorAnonymousInnerClass6(
          std::shared_ptr<IterableAnonymousInnerClass6> outerInstance,
          std::shared_ptr<SortedNumericDocValues> values);

    private:
      int nextDocID = 0;

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts all concatenated values (in docID order) from {@link
   * SortedNumericDocValues} into {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link SortedDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // sortedNumericToValues(final DocValuesProducer valuesProducer, final
  // org.apache.lucene.index.FieldInfo fieldInfo)
  static std::deque<std::shared_ptr<Number>>
  sortedNumericToValues(std::shared_ptr<DocValuesProducer> valuesProducer,
                        std::shared_ptr<FieldInfo> fieldInfo);

private:
  class IterableAnonymousInnerClass7
      : public std::enable_shared_from_this<IterableAnonymousInnerClass7>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass7)
  private:
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    std::shared_ptr<FieldInfo> fieldInfo;

  public:
    IterableAnonymousInnerClass7(
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        std::shared_ptr<FieldInfo> fieldInfo);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass7
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass7>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass7)
    private:
      std::shared_ptr<IterableAnonymousInnerClass7> outerInstance;

      std::shared_ptr<SortedNumericDocValues> values;

    public:
      IteratorAnonymousInnerClass7(
          std::shared_ptr<IterableAnonymousInnerClass7> outerInstance,
          std::shared_ptr<SortedNumericDocValues> values);

    private:
      bool nextIsSet = false;
      int nextCount = 0;
      int upto = 0;
      int64_t nextValue = 0;

      void setNext();

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts norms into {@code Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link NumericDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // normsIterable(final org.apache.lucene.index.FieldInfo field, final
  // NormsProducer normsProducer, final int maxDoc)
  static std::deque<std::shared_ptr<Number>>
  normsIterable(std::shared_ptr<FieldInfo> field,
                std::shared_ptr<NormsProducer> normsProducer, int const maxDoc);

private:
  class IterableAnonymousInnerClass8
      : public std::enable_shared_from_this<IterableAnonymousInnerClass8>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass8)
  private:
    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<org::apache::lucene::codecs::NormsProducer> normsProducer;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass8(
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<org::apache::lucene::codecs::NormsProducer>
            normsProducer,
        int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass8
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass8>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass8)
    private:
      std::shared_ptr<IterableAnonymousInnerClass8> outerInstance;

      std::shared_ptr<NumericDocValues> values;

    public:
      IteratorAnonymousInnerClass8(
          std::shared_ptr<IterableAnonymousInnerClass8> outerInstance,
          std::shared_ptr<NumericDocValues> values);

    private:
      int docIDUpto = 0;

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  /** Converts values from {@link BinaryDocValues} into {@code
   * Iterable&lt;BytesRef&gt;}.
   *
   * @deprecated Consume {@link BinaryDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // Iterable<org.apache.lucene.util.BytesRef> binaryIterable(final
  // org.apache.lucene.index.FieldInfo field, final DocValuesProducer
  // valuesProducer, final int maxDoc)
  static std::deque<std::shared_ptr<BytesRef>>
  binaryIterable(std::shared_ptr<FieldInfo> field,
                 std::shared_ptr<DocValuesProducer> valuesProducer,
                 int const maxDoc);

private:
  class IterableAnonymousInnerClass9
      : public std::enable_shared_from_this<IterableAnonymousInnerClass9>,
        public std::deque<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass9)
  private:
    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass9(
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> iterator();

  private:
    class IteratorAnonymousInnerClass9
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass9>,
          public Iterator<std::shared_ptr<BytesRef>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass9)
    private:
      std::shared_ptr<IterableAnonymousInnerClass9> outerInstance;

      std::shared_ptr<BinaryDocValues> values;

    public:
      IteratorAnonymousInnerClass9(
          std::shared_ptr<IterableAnonymousInnerClass9> outerInstance,
          std::shared_ptr<BinaryDocValues> values);

    private:
      int docIDUpto = 0;

    public:
      bool hasNext();

      std::shared_ptr<BytesRef> next();
    };
  };

  /** Converts values from {@link NumericDocValues} into {@code
   * Iterable&lt;Number&gt;}.
   *
   * @deprecated Consume {@link NumericDocValues} instead. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static Iterable<Number>
  // numericIterable(final org.apache.lucene.index.FieldInfo field, final
  // DocValuesProducer valuesProducer, final int maxDoc)
  static std::deque<std::shared_ptr<Number>>
  numericIterable(std::shared_ptr<FieldInfo> field,
                  std::shared_ptr<DocValuesProducer> valuesProducer,
                  int const maxDoc);

private:
  class IterableAnonymousInnerClass10
      : public std::enable_shared_from_this<IterableAnonymousInnerClass10>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass10)
  private:
    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
        valuesProducer;
    int maxDoc = 0;

  public:
    IterableAnonymousInnerClass10(
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        int maxDoc);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass10
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass10>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass10)
    private:
      std::shared_ptr<IterableAnonymousInnerClass10> outerInstance;

      std::shared_ptr<NumericDocValues> values;

    public:
      IteratorAnonymousInnerClass10(
          std::shared_ptr<IterableAnonymousInnerClass10> outerInstance,
          std::shared_ptr<NumericDocValues> values);

    private:
      int docIDUpto = 0;

    public:
      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };
};

} // namespace org::apache::lucene::codecs
