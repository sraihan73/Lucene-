#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::perfield
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Enables per field docvalues support.
 * <p>
 * Note, when extending this class, the name ({@link #getName}) is
 * written into the index. In order for the field to be read, the
 * name must resolve to your implementation via {@link #forName(std::wstring)}.
 * This method uses Java's
 * {@link ServiceLoader Service Provider Interface} to resolve format names.
 * <p>
 * Files written by each docvalues format have an additional suffix containing
 * the format name. For example, in a per-field configuration instead of
 * <tt>_1.dat</tt> filenames would look like <tt>_1_Lucene40_0.dat</tt>.
 * @see ServiceLoader
 * @lucene.experimental
 */

class PerFieldDocValuesFormat : public DocValuesFormat
{
  GET_CLASS_NAME(PerFieldDocValuesFormat)
  /** Name of this {@link PostingsFormat}. */
public:
  static const std::wstring PER_FIELD_NAME;

  /** {@link FieldInfo} attribute name used to store the
   *  format name for each field. */
  static const std::wstring PER_FIELD_FORMAT_KEY;

  /** {@link FieldInfo} attribute name used to store the
   *  segment suffix name for each field. */
  static const std::wstring PER_FIELD_SUFFIX_KEY;

  /** Sole constructor. */
  PerFieldDocValuesFormat();

  std::shared_ptr<DocValuesConsumer>
  fieldsConsumer(std::shared_ptr<SegmentWriteState> state) throw(
      IOException) override final;

public:
  class ConsumerAndSuffix
      : public std::enable_shared_from_this<ConsumerAndSuffix>
  {
    GET_CLASS_NAME(ConsumerAndSuffix)
  public:
    std::shared_ptr<DocValuesConsumer> consumer;
    int suffix = 0;

    virtual ~ConsumerAndSuffix();
  };

private:
  class FieldsWriter : public DocValuesConsumer
  {
    GET_CLASS_NAME(FieldsWriter)
  private:
    std::shared_ptr<PerFieldDocValuesFormat> outerInstance;

    const std::unordered_map<std::shared_ptr<DocValuesFormat>,
                             std::shared_ptr<ConsumerAndSuffix>>
        formats = std::unordered_map<std::shared_ptr<DocValuesFormat>,
                                     std::shared_ptr<ConsumerAndSuffix>>();
    const std::unordered_map<std::wstring, int> suffixes =
        std::unordered_map<std::wstring, int>();

    const std::shared_ptr<SegmentWriteState> segmentWriteState;

  public:
    FieldsWriter(std::shared_ptr<PerFieldDocValuesFormat> outerInstance,
                 std::shared_ptr<SegmentWriteState> state);

    void addNumericField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

    void addBinaryField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                               std::shared_ptr<DocValuesProducer>
                                   valuesProducer)  override;

    void addSortedSetField(std::shared_ptr<FieldInfo> field,
                           std::shared_ptr<DocValuesProducer>
                               valuesProducer)  override;

    void
    merge(std::shared_ptr<MergeState> mergeState)  override;

  private:
    std::shared_ptr<DocValuesConsumer>
    getInstance(std::shared_ptr<FieldInfo> field) ;

  public:
    virtual ~FieldsWriter();

  protected:
    std::shared_ptr<FieldsWriter> shared_from_this()
    {
      return std::static_pointer_cast<FieldsWriter>(
          org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
    }
  };

public:
  static std::wstring getSuffix(const std::wstring &formatName,
                                const std::wstring &suffix);

  static std::wstring
  getFullSegmentSuffix(const std::wstring &outerSegmentSuffix,
                       const std::wstring &segmentSuffix);

private:
  class FieldsReader : public DocValuesProducer
  {
    GET_CLASS_NAME(FieldsReader)
  private:
    std::shared_ptr<PerFieldDocValuesFormat> outerInstance;

    const std::unordered_map<std::wstring, std::shared_ptr<DocValuesProducer>>
        fields = std::map_obj<std::wstring, std::shared_ptr<DocValuesProducer>>();
    const std::unordered_map<std::wstring, std::shared_ptr<DocValuesProducer>>
        formats = std::unordered_map<std::wstring,
                                     std::shared_ptr<DocValuesProducer>>();

    // clone for merge
  public:
    FieldsReader(std::shared_ptr<PerFieldDocValuesFormat> outerInstance,
                 std::shared_ptr<FieldsReader> other) ;

    FieldsReader(
        std::shared_ptr<PerFieldDocValuesFormat> outerInstance,
        std::shared_ptr<SegmentReadState> readState) ;

    std::shared_ptr<NumericDocValues>
    getNumeric(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedDocValues>
    getSorted(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
        std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedSetDocValues>
    getSortedSet(std::shared_ptr<FieldInfo> field)  override;

    virtual ~FieldsReader();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<DocValuesProducer>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<FieldsReader> shared_from_this()
    {
      return std::static_pointer_cast<FieldsReader>(
          org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocValuesProducer>
  fieldsProducer(std::shared_ptr<SegmentReadState> state) throw(
      IOException) override final;

  /**
   * Returns the doc values format that should be used for writing
   * new segments of <code>field</code>.
   * <p>
   * The field to format mapping is written to the index, so
   * this method is only invoked when writing, not when reading. */
  virtual std::shared_ptr<DocValuesFormat>
  getDocValuesFormatForField(const std::wstring &field) = 0;

protected:
  std::shared_ptr<PerFieldDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<PerFieldDocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/perfield/
