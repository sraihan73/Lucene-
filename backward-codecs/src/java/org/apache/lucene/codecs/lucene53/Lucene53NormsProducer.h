#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/lucene53/NormsEntry.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/RandomAccessInput.h"

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
namespace org::apache::lucene::codecs::lucene53
{

using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_CURRENT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_START;

/**
 * Reader for {@link Lucene53NormsFormat}
 */
class Lucene53NormsProducer : public NormsProducer
{
  GET_CLASS_NAME(Lucene53NormsProducer)
  // metadata maps (just file pointers and minimal stuff)
private:
  const std::unordered_map<int, std::shared_ptr<NormsEntry>> norms =
      std::unordered_map<int, std::shared_ptr<NormsEntry>>();
  const std::shared_ptr<IndexInput> data;
  const int maxDoc;

public:
  Lucene53NormsProducer(std::shared_ptr<SegmentReadState> state,
                        const std::wstring &dataCodec,
                        const std::wstring &dataExtension,
                        const std::wstring &metaCodec,
                        const std::wstring &metaExtension) ;

private:
  void readFields(std::shared_ptr<IndexInput> meta,
                  std::shared_ptr<FieldInfos> infos) ;

public:
  std::shared_ptr<NumericDocValues>
  getNorms(std::shared_ptr<FieldInfo> field)  override;

private:
  class NormsIteratorAnonymousInnerClass : public NormsIterator
  {
    GET_CLASS_NAME(NormsIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene53NormsProducer> outerInstance;

    int64_t value = 0;

  public:
    NormsIteratorAnonymousInnerClass(
        std::shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        int64_t value);

    int64_t longValue() override;

  protected:
    std::shared_ptr<NormsIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NormsIteratorAnonymousInnerClass>(
          NormsIterator::shared_from_this());
    }
  };

private:
  class NormsIteratorAnonymousInnerClass2 : public NormsIterator
  {
    GET_CLASS_NAME(NormsIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene53NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    NormsIteratorAnonymousInnerClass2(
        std::shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<NormsIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<NormsIteratorAnonymousInnerClass2>(
          NormsIterator::shared_from_this());
    }
  };

private:
  class NormsIteratorAnonymousInnerClass3 : public NormsIterator
  {
    GET_CLASS_NAME(NormsIteratorAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene53NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    NormsIteratorAnonymousInnerClass3(
        std::shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<NormsIteratorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<NormsIteratorAnonymousInnerClass3>(
          NormsIterator::shared_from_this());
    }
  };

private:
  class NormsIteratorAnonymousInnerClass4 : public NormsIterator
  {
    GET_CLASS_NAME(NormsIteratorAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene53NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    NormsIteratorAnonymousInnerClass4(
        std::shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<NormsIteratorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<NormsIteratorAnonymousInnerClass4>(
          NormsIterator::shared_from_this());
    }
  };

private:
  class NormsIteratorAnonymousInnerClass5 : public NormsIterator
  {
    GET_CLASS_NAME(NormsIteratorAnonymousInnerClass5)
  private:
    std::shared_ptr<Lucene53NormsProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;

  public:
    NormsIteratorAnonymousInnerClass5(
        std::shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<NormsIteratorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<NormsIteratorAnonymousInnerClass5>(
          NormsIterator::shared_from_this());
    }
  };

public:
  virtual ~Lucene53NormsProducer();

  int64_t ramBytesUsed() override;

  void checkIntegrity()  override;

public:
  class NormsEntry : public std::enable_shared_from_this<NormsEntry>
  {
    GET_CLASS_NAME(NormsEntry)
  public:
    char bytesPerValue = 0;
    int64_t offset = 0;
  };

public:
  virtual std::wstring toString();

private:
  class NormsIterator : public NumericDocValues
  {
    GET_CLASS_NAME(NormsIterator)
  private:
    const int maxDoc;

  protected:
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

  public:
    NormsIterator(int maxDoc);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NormsIterator> shared_from_this()
    {
      return std::static_pointer_cast<NormsIterator>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<Lucene53NormsProducer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene53NormsProducer>(
        org.apache.lucene.codecs.NormsProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene53/
