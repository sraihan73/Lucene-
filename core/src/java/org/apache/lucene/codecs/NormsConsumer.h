#pragma once
#include "../index/DocIDMerger.h"
#include "../index/NumericDocValues.h"
#include "NormsProducer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/index/MergeState.h"
namespace org::apache::lucene::index
{
template <typename T>
class DocIDMerger;
}
#include  "core/src/java/org/apache/lucene/index/DocMap.h"

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

using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;

/**
 * Abstract API that consumes normalization values.
 * Concrete implementations of this
 * actually do "something" with the norms (write it into
 * the index in a specific format).
 * <p>
 * The lifecycle is:
 * <ol>
 *   <li>NormsConsumer is created by
 *       {@link NormsFormat#normsConsumer(SegmentWriteState)}.
 *   <li>{@link #addNormsField} is called for each field with
 *       normalization values. The API is a "pull" rather
 *       than "push", and the implementation is free to iterate over the
 *       values multiple times ({@link Iterable#iterator()}).
 *   <li>After all fields are added, the consumer is {@link #close}d.
 * </ol>
 *
 * @lucene.experimental
 */
class NormsConsumer : public std::enable_shared_from_this<NormsConsumer>
{
  GET_CLASS_NAME(NormsConsumer)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  NormsConsumer();

  /**
   * Writes normalization values for a field.
   * @param field field information
   * @param normsProducer NormsProducer of the numeric norm values
   * @throws IOException if an I/O error occurred.
   */
public:
  virtual void addNormsField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<NormsProducer> normsProducer) = 0;

  /** Merges in the fields from the readers in
   *  <code>mergeState</code>. The default implementation
   *  calls {@link #mergeNormsField} for each field,
   *  filling segments with missing norms for the field with zeros.
   *  Implementations can override this method
   *  for more sophisticated merging (bulk-byte copying, etc). */
  virtual void merge(std::shared_ptr<MergeState> mergeState) ;

  /** Tracks state of one numeric sub-reader that we are merging */
private:
  class NumericDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(NumericDocValuesSub)

  private:
    const std::shared_ptr<NumericDocValues> values;

  public:
    NumericDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                        std::shared_ptr<NumericDocValues> values);

    int nextDoc()  override;

  protected:
    std::shared_ptr<NumericDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the norms from <code>toMerge</code>.
   * <p>
   * The default implementation calls {@link #addNormsField}, passing
   * an Iterable that merges and filters deleted documents on the fly.
   */
public:
  virtual void
  mergeNormsField(std::shared_ptr<FieldInfo> mergeFieldInfo,
                  std::shared_ptr<MergeState> mergeState) ;

private:
  class NormsProducerAnonymousInnerClass : public NormsProducer
  {
    GET_CLASS_NAME(NormsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<NormsConsumer> outerInstance;

    std::shared_ptr<FieldInfo> mergeFieldInfo;
    std::shared_ptr<MergeState> mergeState;

  public:
    NormsProducerAnonymousInnerClass(
        std::shared_ptr<NormsConsumer> outerInstance,
        std::shared_ptr<FieldInfo> mergeFieldInfo,
        std::shared_ptr<MergeState> mergeState);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> fieldInfo)  override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<NormsProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
          docIDMerger;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<NormsProducerAnonymousInnerClass> outerInstance,
          std::shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
              docIDMerger);

    private:
      int docID = 0;
      std::shared_ptr<NumericDocValuesSub> current;

    public:
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int64_t longValue()  override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.NumericDocValues::shared_from_this());
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
          NormsProducer::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/codecs/
