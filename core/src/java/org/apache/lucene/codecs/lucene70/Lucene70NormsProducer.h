#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::lucene70
{
class NormsEntry;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::codecs::lucene70
{
class IndexedDISI;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::store
{
class RandomAccessInput;
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
namespace org::apache::lucene::codecs::lucene70
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_CURRENT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_START;

using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;

/**
 * Reader for {@link Lucene70NormsFormat}
 */
class Lucene70NormsProducer final : public NormsProducer
{
  GET_CLASS_NAME(Lucene70NormsProducer)
  // metadata maps (just file pointers and minimal stuff)
private:
  const std::unordered_map<int, std::shared_ptr<NormsEntry>> norms =
      std::unordered_map<int, std::shared_ptr<NormsEntry>>();
  const std::shared_ptr<IndexInput> data;
  const int maxDoc;

public:
  Lucene70NormsProducer(std::shared_ptr<SegmentReadState> state,
                        const std::wstring &dataCodec,
                        const std::wstring &dataExtension,
                        const std::wstring &metaCodec,
                        const std::wstring &metaExtension) ;

public:
  class NormsEntry : public std::enable_shared_from_this<NormsEntry>
  {
    GET_CLASS_NAME(NormsEntry)
  public:
    char bytesPerNorm = 0;
    int64_t docsWithFieldOffset = 0;
    int64_t docsWithFieldLength = 0;
    int numDocsWithField = 0;
    int64_t normsOffset = 0;
  };

public:
  class DenseNormsIterator : public NumericDocValues
  {
    GET_CLASS_NAME(DenseNormsIterator)

  public:
    const int maxDoc;
    int doc = -1;

    DenseNormsIterator(int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DenseNormsIterator> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIterator>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

public:
  class SparseNormsIterator : public NumericDocValues
  {
    GET_CLASS_NAME(SparseNormsIterator)

  public:
    const std::shared_ptr<IndexedDISI> disi;

    SparseNormsIterator(std::shared_ptr<IndexedDISI> disi);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SparseNormsIterator> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIterator>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

private:
  void readFields(std::shared_ptr<IndexInput> meta,
                  std::shared_ptr<FieldInfos> infos) ;

public:
  std::shared_ptr<NumericDocValues>
  getNorms(std::shared_ptr<FieldInfo> field)  override;

private:
  class DenseNormsIteratorAnonymousInnerClass : public DenseNormsIterator
  {
    GET_CLASS_NAME(DenseNormsIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70NormsProducer::NormsEntry>
        entry;

  public:
    DenseNormsIteratorAnonymousInnerClass(
        std::shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70NormsProducer::NormsEntry>
            entry);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNormsIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIteratorAnonymousInnerClass>(
          DenseNormsIterator::shared_from_this());
    }
  };

private:
  class DenseNormsIteratorAnonymousInnerClass2 : public DenseNormsIterator
  {
    GET_CLASS_NAME(DenseNormsIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    DenseNormsIteratorAnonymousInnerClass2(
        std::shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNormsIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIteratorAnonymousInnerClass2>(
          DenseNormsIterator::shared_from_this());
    }
  };

private:
  class DenseNormsIteratorAnonymousInnerClass3 : public DenseNormsIterator
  {
    GET_CLASS_NAME(DenseNormsIteratorAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    DenseNormsIteratorAnonymousInnerClass3(
        std::shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNormsIteratorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIteratorAnonymousInnerClass3>(
          DenseNormsIterator::shared_from_this());
    }
  };

private:
  class DenseNormsIteratorAnonymousInnerClass4 : public DenseNormsIterator
  {
    GET_CLASS_NAME(DenseNormsIteratorAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    DenseNormsIteratorAnonymousInnerClass4(
        std::shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNormsIteratorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIteratorAnonymousInnerClass4>(
          DenseNormsIterator::shared_from_this());
    }
  };

private:
  class DenseNormsIteratorAnonymousInnerClass5 : public DenseNormsIterator
  {
    GET_CLASS_NAME(DenseNormsIteratorAnonymousInnerClass5)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    DenseNormsIteratorAnonymousInnerClass5(
        std::shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNormsIteratorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<DenseNormsIteratorAnonymousInnerClass5>(
          DenseNormsIterator::shared_from_this());
    }
  };

private:
  class SparseNormsIteratorAnonymousInnerClass : public SparseNormsIterator
  {
    GET_CLASS_NAME(SparseNormsIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70NormsProducer::NormsEntry>
        entry;

  public:
    SparseNormsIteratorAnonymousInnerClass(
        std::shared_ptr<Lucene70NormsProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70NormsProducer::NormsEntry>
            entry);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNormsIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIteratorAnonymousInnerClass>(
          SparseNormsIterator::shared_from_this());
    }
  };

private:
  class SparseNormsIteratorAnonymousInnerClass2 : public SparseNormsIterator
  {
    GET_CLASS_NAME(SparseNormsIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<RandomAccessInput> slice;

  public:
    SparseNormsIteratorAnonymousInnerClass2(
        std::shared_ptr<Lucene70NormsProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNormsIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIteratorAnonymousInnerClass2>(
          SparseNormsIterator::shared_from_this());
    }
  };

private:
  class SparseNormsIteratorAnonymousInnerClass3 : public SparseNormsIterator
  {
    GET_CLASS_NAME(SparseNormsIteratorAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<RandomAccessInput> slice;

  public:
    SparseNormsIteratorAnonymousInnerClass3(
        std::shared_ptr<Lucene70NormsProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNormsIteratorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIteratorAnonymousInnerClass3>(
          SparseNormsIterator::shared_from_this());
    }
  };

private:
  class SparseNormsIteratorAnonymousInnerClass4 : public SparseNormsIterator
  {
    GET_CLASS_NAME(SparseNormsIteratorAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<RandomAccessInput> slice;

  public:
    SparseNormsIteratorAnonymousInnerClass4(
        std::shared_ptr<Lucene70NormsProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNormsIteratorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIteratorAnonymousInnerClass4>(
          SparseNormsIterator::shared_from_this());
    }
  };

private:
  class SparseNormsIteratorAnonymousInnerClass5 : public SparseNormsIterator
  {
    GET_CLASS_NAME(SparseNormsIteratorAnonymousInnerClass5)
  private:
    std::shared_ptr<Lucene70NormsProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<RandomAccessInput> slice;

  public:
    SparseNormsIteratorAnonymousInnerClass5(
        std::shared_ptr<Lucene70NormsProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNormsIteratorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<SparseNormsIteratorAnonymousInnerClass5>(
          SparseNormsIterator::shared_from_this());
    }
  };

public:
  virtual ~Lucene70NormsProducer();

  int64_t ramBytesUsed() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<Lucene70NormsProducer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70NormsProducer>(
        org.apache.lucene.codecs.NormsProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
