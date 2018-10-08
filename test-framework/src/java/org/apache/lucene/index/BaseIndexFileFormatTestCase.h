#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
namespace org::apache::lucene::index
{

using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;

/**
 * Common tests to all index formats.
 */
class BaseIndexFileFormatTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseIndexFileFormatTestCase)

  // metadata or Directory-level objects
private:
  static const std::shared_ptr<Set<std::type_info>> EXCLUDED_CLASSES;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static BaseIndexFileFormatTestCase::StaticConstructor staticConstructor;

public:
  class Accumulator : public RamUsageTester::Accumulator
  {
    GET_CLASS_NAME(Accumulator)

  private:
    const std::any root;

  public:
    Accumulator(std::any root);

    int64_t accumulateObject(
        std::any o, int64_t shallowSize,
        std::unordered_map<std::shared_ptr<java::lang::reflect::Field>,
                           std::any> &fieldValues,
        std::shared_ptr<std::deque<std::any>> queue) override;

    int64_t
    accumulateArray(std::any array_, int64_t shallowSize,
                    std::deque<std::any> &values,
                    std::shared_ptr<std::deque<std::any>> queue) override;

  protected:
    std::shared_ptr<Accumulator> shared_from_this()
    {
      return std::static_pointer_cast<Accumulator>(
          org.apache.lucene.util.RamUsageTester
              .Accumulator::shared_from_this());
    }
  };

  /** Returns the codec to run tests against */
protected:
  virtual std::shared_ptr<Codec> getCodec() = 0;

  /** Returns the major version that this codec is compatible with. */
  virtual int getCreatedVersionMajor();

  /** Set the created version of the given {@link Directory} and return it. */
  template <typename D>
  D applyCreatedVersionMajor(D d) ;

private:
  std::shared_ptr<Codec> savedCodec;

public:
  void setUp()  override;

  void tearDown()  override;

  /** Add random fields to the provided document. */
protected:
  virtual void addRandomFields(std::shared_ptr<Document> doc) = 0;

private:
  std::unordered_map<std::wstring, int64_t>
  bytesUsedByExtension(std::shared_ptr<Directory> d) ;

  /**
   * Return the deque of extensions that should be excluded from byte counts when
   * comparing indices that store the same content.
   */
protected:
  virtual std::shared_ptr<std::deque<std::wstring>>
  excludedExtensionsFromByteCounts();

  /** The purpose of this test is to make sure that bulk merge doesn't
   * accumulate useless data over runs. */
public:
  virtual void testMergeStability() ;

protected:
  virtual bool mergeIsStable();

  /** Test the accuracy of the ramBytesUsed estimations. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testRamBytesUsed() throws
  // java.io.IOException
  virtual void testRamBytesUsed() ;

  /** Calls close multiple times on closeable codec apis */
  virtual void testMultiClose() ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<BaseIndexFileFormatTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::index::FieldInfo> field;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<BaseIndexFileFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::index::FieldInfo> field);

    std::shared_ptr<NumericDocValues>
    getNumeric(std::shared_ptr<FieldInfo> field) override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance);

      int docID = 0;

      int docID() override;

      int nextDoc() override;

      int advance(int target) override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int64_t longValue() override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            NumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

private:
  class NormsProducerAnonymousInnerClass : public NormsProducer
  {
    GET_CLASS_NAME(NormsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<BaseIndexFileFormatTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::index::FieldInfo> field;

  public:
    NormsProducerAnonymousInnerClass(
        std::shared_ptr<BaseIndexFileFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::index::FieldInfo> field);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> field) override;

  private:
    class NumericDocValuesAnonymousInnerClass2 : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass2)
    private:
      std::shared_ptr<NormsProducerAnonymousInnerClass> outerInstance;

    public:
      NumericDocValuesAnonymousInnerClass2(
          std::shared_ptr<NormsProducerAnonymousInnerClass> outerInstance);

      int docID = 0;

      int docID() override;

      int nextDoc() override;

      int advance(int target) override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int64_t longValue() override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass2>(
            NumericDocValues::shared_from_this());
      }
    };

  public:
    void checkIntegrity() override;

    virtual ~NormsProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<NormsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NormsProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.NormsProducer::shared_from_this());
    }
  };

  /** Tests exception handling on write and openInput/createOutput */
  // TODO: this is really not ideal. each BaseXXXTestCase should have unit tests
  // doing this. but we use this shotgun approach to prevent bugs in the
  // meantime: it just ensures the codec does not corrupt the index or leak file
  // handles.
public:
  virtual void testRandomExceptions() ;

private:
  void handleFakeIOException(std::shared_ptr<IOException> e,
                             std::shared_ptr<PrintStream> exceptionStream);

protected:
  std::shared_ptr<BaseIndexFileFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseIndexFileFormatTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
