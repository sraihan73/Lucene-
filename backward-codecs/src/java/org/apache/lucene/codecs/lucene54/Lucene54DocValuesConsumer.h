#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include  "core/src/java/org/apache/lucene/util/LongsRef.h"

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
namespace org::apache::lucene::codecs::lucene54
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongsRef = org::apache::lucene::util::LongsRef;

using namespace org::apache::lucene::codecs::lucene54;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.codecs.lucene54.Lucene54DocValuesFormat.*;

/** writer for {@link Lucene54DocValuesFormat} */
class Lucene54DocValuesConsumer final : public DocValuesConsumer
{
  GET_CLASS_NAME(Lucene54DocValuesConsumer)

public:
  enum class NumberType {
    GET_CLASS_NAME(NumberType)
    /** Dense ordinals */
    ORDINAL,
    /** Random long values */
    VALUE
  };

public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;

  /** expert: Creates a new writer */
  Lucene54DocValuesConsumer(
      std::shared_ptr<SegmentWriteState> state, const std::wstring &dataCodec,
      const std::wstring &dataExtension, const std::wstring &metaCodec,
      const std::wstring &metaExtension) ;

  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<DocValuesProducer>
                           valuesProducer)  override;

  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::deque<std::shared_ptr<Number>> &values,
                       NumberType numberType) ;

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesConsumer> outerInstance;

    std::deque<std::shared_ptr<Number>> values;

  public:
    IterableAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        std::deque<std::shared_ptr<Number>> &values);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();
  };

private:
  class IterableAnonymousInnerClass2
      : public std::enable_shared_from_this<IterableAnonymousInnerClass2>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesConsumer> outerInstance;

    std::deque<std::shared_ptr<Number>> values;

  public:
    IterableAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        std::deque<std::shared_ptr<Number>> &values);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();
  };

  // TODO: in some cases representing missing with minValue-1 wouldn't take up
  // additional space and so on, but this is very simple, and algorithms only
  // check this for values of 0 anyway (doesnt slow down normal decode)
public:
  template <typename T1>
  void writeMissingBitset(std::deque<T1> values) ;

  int64_t
  writeSparseMissingBitset(std::deque<std::shared_ptr<Number>> &values,
                           NumberType numberType,
                           int64_t numDocsWithValue) ;

  void addBinaryField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void addBinaryField(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<BytesRef>> &values) ;

  /** expert: writes a value dictionary for a sorted/sortedset field */
  void addTermsDict(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<BytesRef>> &values) ;
  // writes term dictionary "block"
  // first term is absolute encoded as vint length + bytes.
  // lengths of subsequent N terms are encoded as either N bytes or N shorts.
  // in the double-byte case, the first byte is indicated with -1.
  // subsequent terms are encoded as byte suffixLength + bytes.
  void flushTermsDictBlock(std::shared_ptr<RAMOutputStream> headerBuffer,
                           std::shared_ptr<RAMOutputStream> bytesBuffer,
                           std::deque<int> &suffixDeltas) ;

  // writes reverse term index: used for binary searching a term into a range of
  // 64 blocks for every 64 blocks (1024 terms) we store a term, trimming any
  // suffix unnecessary for comparison terms are written as a contiguous byte[],
  // but never spanning 2^15 byte boundaries.
  void addReverseTermIndex(std::shared_ptr<FieldInfo> field,
                           std::deque<std::shared_ptr<BytesRef>> &values,
                           int maxLength) ;

public:
  void addSortedField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void
  addSortedField(std::shared_ptr<FieldInfo> field,
                 std::deque<std::shared_ptr<BytesRef>> &values,
                 std::deque<std::shared_ptr<Number>> &ords) ;

public:
  void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 valuesProducer)  override;

  void addSortedSetField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

private:
  std::shared_ptr<SortedSet<std::shared_ptr<LongsRef>>>
  uniqueValueSets(std::deque<std::shared_ptr<Number>> &docToValueCount,
                  std::deque<std::shared_ptr<Number>> &values);

  void writeDictionary(std::shared_ptr<SortedSet<std::shared_ptr<LongsRef>>>
                           uniqueValueSets) ;

  std::deque<std::shared_ptr<Number>> docToSetId(
      std::shared_ptr<SortedSet<std::shared_ptr<LongsRef>>> uniqueValueSets,
      std::deque<std::shared_ptr<Number>> &docToValueCount,
      std::deque<std::shared_ptr<Number>> &values);

private:
  class IterableAnonymousInnerClass3
      : public std::enable_shared_from_this<IterableAnonymousInnerClass3>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene54DocValuesConsumer> outerInstance;

    std::deque<std::shared_ptr<Number>> docToValueCount;
    std::deque<std::shared_ptr<Number>> values;
    std::unordered_map<std::shared_ptr<LongsRef>, int> setIds;
    int i = 0;

  public:
    IterableAnonymousInnerClass3(
        std::shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        std::deque<std::shared_ptr<Number>> &docToValueCount,
        std::deque<std::shared_ptr<Number>> &values,
        std::unordered_map<std::shared_ptr<LongsRef>, int> &setIds, int i);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass3> outerInstance;

      std::deque<std::shared_ptr<Number>>::const_iterator valueCountIterator;
      std::deque<std::shared_ptr<Number>>::const_iterator valueIterator;
      std::shared_ptr<LongsRef> docValues;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass3> outerInstance,
          std::deque<Number>::const_iterator valueCountIterator,
          std::deque<Number>::const_iterator valueIterator,
          std::shared_ptr<LongsRef> docValues);

      bool hasNext();

      std::shared_ptr<Number> next();
    };
  };

  // writes addressing information as MONOTONIC_COMPRESSED integer
private:
  void
  addOrdIndex(std::shared_ptr<FieldInfo> field,
              std::deque<std::shared_ptr<Number>> &values) ;

public:
  virtual ~Lucene54DocValuesConsumer();

protected:
  std::shared_ptr<Lucene54DocValuesConsumer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene54DocValuesConsumer>(
        org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene54/
