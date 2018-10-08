#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::codecs::simpletext
{
class DocValuesIterator;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs::simpletext
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.END;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.FIELD;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.LENGTH;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.MAXLENGTH;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.MINVALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.NUMVALUES;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.ORDPATTERN;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.PATTERN;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.TYPE;

class SimpleTextDocValuesReader : public DocValuesProducer
{
  GET_CLASS_NAME(SimpleTextDocValuesReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          SimpleTextDocValuesReader::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(BytesRef::typeid);

public:
  class OneField : public std::enable_shared_from_this<OneField>
  {
    GET_CLASS_NAME(OneField)
  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(OneField::typeid);

  public:
    int64_t dataStartFilePointer = 0;
    std::wstring pattern;
    std::wstring ordPattern;
    int maxLength = 0;
    bool fixedLength = false;
    int64_t minValue = 0;
    int64_t numValues = 0;
  };

public:
  const int maxDoc;
  const std::shared_ptr<IndexInput> data;
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  const std::unordered_map<std::wstring, std::shared_ptr<OneField>> fields =
      std::unordered_map<std::wstring, std::shared_ptr<OneField>>();

  SimpleTextDocValuesReader(std::shared_ptr<SegmentReadState> state,
                            const std::wstring &ext) ;

  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> fieldInfo)  override;

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::function<int64_t(int)> values;
    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::DocValuesIterator>
        docsWithField;

  public:
    NumericDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::function<int64_t(int)> &values,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::DocValuesIterator>
            docsWithField);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t longValue()  override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<internal> std::function <
      int64_t(int) ;

private:
  class DocValuesIterator : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocValuesIterator)
  public:
    virtual bool advanceExact(int target) = 0;

  protected:
    std::shared_ptr<DocValuesIterator> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesIterator>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

private:
  std::shared_ptr<DocValuesIterator> getNumericDocsWithField(
      std::shared_ptr<FieldInfo> fieldInfo) ;

private:
  class DocValuesIteratorAnonymousInnerClass : public DocValuesIterator
  {
    GET_CLASS_NAME(DocValuesIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::OneField>
        field;
    std::shared_ptr<IndexInput> in_;
    std::shared_ptr<BytesRefBuilder> scratch;

  public:
    DocValuesIteratorAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::OneField>
            field,
        std::shared_ptr<IndexInput> in_,
        std::shared_ptr<BytesRefBuilder> scratch);

    int doc = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<DocValuesIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesIteratorAnonymousInnerClass>(
          DocValuesIterator::shared_from_this());
    }
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> fieldInfo)  override;

private:
  class functionAnonymousInnerClass : public std::function<BytesRef *(int)>
  {
    GET_CLASS_NAME(functionAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::OneField>
        field;
    std::shared_ptr<IndexInput> in_;
    std::shared_ptr<BytesRefBuilder> scratch;
    std::shared_ptr<DecimalFormat> decoder;

  public:
    functionAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<UnknownType> int,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::OneField>
            field,
        std::shared_ptr<IndexInput> in_,
        std::shared_ptr<BytesRefBuilder> scratch,
        std::shared_ptr<DecimalFormat> decoder);

    const std::shared_ptr<BytesRefBuilder> term;

    std::shared_ptr<BytesRef> apply(int docID) override;

  protected:
    std::shared_ptr<functionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<functionAnonymousInnerClass>(
          std::function<org.apache.lucene.util.BytesRef *(
              int)>::shared_from_this());
    }
  };

private:
  class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
  {
    GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::DocValuesIterator>
        docsWithField;
    std::function<BytesRef *(int)> values;

  public:
    BinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::DocValuesIterator>
            docsWithField,
        std::function<BytesRef *(int)> &values);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    std::shared_ptr<BytesRef> binaryValue()  override;

  protected:
    std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
          BinaryDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<DocValuesIterator> getBinaryDocsWithField(
      std::shared_ptr<FieldInfo> fieldInfo) ;

private:
  class DocValuesIteratorAnonymousInnerClass2 : public DocValuesIterator
  {
    GET_CLASS_NAME(DocValuesIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::OneField>
        field;
    std::shared_ptr<IndexInput> in_;
    std::shared_ptr<BytesRefBuilder> scratch;
    std::shared_ptr<DecimalFormat> decoder;

  public:
    DocValuesIteratorAnonymousInnerClass2(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::OneField>
            field,
        std::shared_ptr<IndexInput> in_,
        std::shared_ptr<BytesRefBuilder> scratch,
        std::shared_ptr<DecimalFormat> decoder);

    int doc = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<DocValuesIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesIteratorAnonymousInnerClass2>(
          DocValuesIterator::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> fieldInfo)  override;

private:
  class SortedDocValuesAnonymousInnerClass : public SortedDocValues
  {
    GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::OneField>
        field;
    std::shared_ptr<IndexInput> in_;
    std::shared_ptr<BytesRefBuilder> scratch;
    std::shared_ptr<DecimalFormat> decoder;
    std::shared_ptr<DecimalFormat> ordDecoder;

  public:
    SortedDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::OneField>
            field,
        std::shared_ptr<IndexInput> in_,
        std::shared_ptr<BytesRefBuilder> scratch,
        std::shared_ptr<DecimalFormat> decoder,
        std::shared_ptr<DecimalFormat> ordDecoder);

    int doc = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int ord = 0;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int ordValue() override;

    const std::shared_ptr<BytesRefBuilder> term;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

  protected:
    std::shared_ptr<SortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass>(
          SortedDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::index::BinaryDocValues> binary;

  public:
    SortedNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::index::BinaryDocValues> binary);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    std::deque<int64_t> values;
    int index = 0;

  private:
    void setCurrentDoc() ;

  public:
    int64_t nextValue()  override;

    int docValueCount() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          SortedNumericDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedSetDocValues> getSortedSet(
      std::shared_ptr<FieldInfo> fieldInfo)  override;

private:
  class SortedSetDocValuesAnonymousInnerClass : public SortedSetDocValues
  {
    GET_CLASS_NAME(SortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextDocValuesReader> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::simpletext::
                        SimpleTextDocValuesReader::OneField>
        field;
    std::shared_ptr<IndexInput> in_;
    std::shared_ptr<BytesRefBuilder> scratch;
    std::shared_ptr<DecimalFormat> decoder;

  public:
    SortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleTextDocValuesReader> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::simpletext::
                            SimpleTextDocValuesReader::OneField>
            field,
        std::shared_ptr<IndexInput> in_,
        std::shared_ptr<BytesRefBuilder> scratch,
        std::shared_ptr<DecimalFormat> decoder);

    std::deque<std::wstring> currentOrds;
    int currentIndex = 0;
    const std::shared_ptr<BytesRefBuilder> term;
    int doc = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t nextOrd()  override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t getValueCount() override;

  protected:
    std::shared_ptr<SortedSetDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetDocValuesAnonymousInnerClass>(
          SortedSetDocValues::shared_from_this());
    }
  };

public:
  virtual ~SimpleTextDocValuesReader();

  /** Used only in ctor: */
private:
  void readLine() ;

  /** Used only in ctor: */
  bool startsWith(std::shared_ptr<BytesRef> prefix);

  /** Used only in ctor: */
  std::wstring stripPrefix(std::shared_ptr<BytesRef> prefix);

public:
  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<SimpleTextDocValuesReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextDocValuesReader>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
