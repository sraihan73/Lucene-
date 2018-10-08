#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"

#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"

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

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Fields = org::apache::lucene::index::Fields;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * Enables per field postings support.
 * <p>
 * Note, when extending this class, the name ({@link #getName}) is
 * written into the index. In order for the field to be read, the
 * name must resolve to your implementation via {@link #forName(std::wstring)}.
 * This method uses Java's
 * {@link ServiceLoader Service Provider Interface} to resolve format names.
 * <p>
 * Files written by each posting format have an additional suffix containing the
 * format name. For example, in a per-field configuration instead of
 * <tt>_1.prx</tt> filenames would look like <tt>_1_Lucene40_0.prx</tt>.
 * @see ServiceLoader
 * @lucene.experimental
 */

class PerFieldPostingsFormat : public PostingsFormat
{
  GET_CLASS_NAME(PerFieldPostingsFormat)
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
  PerFieldPostingsFormat();

  /** Group of fields written by one PostingsFormat */
public:
  class FieldsGroup : public std::enable_shared_from_this<FieldsGroup>
  {
    GET_CLASS_NAME(FieldsGroup)
  public:
    const std::shared_ptr<Set<std::wstring>> fields = std::set<std::wstring>();
    int suffix = 0;

    /** Custom SegmentWriteState for this group of fields,
     *  with the segmentSuffix uniqueified for this
     *  PostingsFormat */
    std::shared_ptr<SegmentWriteState> state;
  };

public:
  static std::wstring getSuffix(const std::wstring &formatName,
                                const std::wstring &suffix);

  static std::wstring
  getFullSegmentSuffix(const std::wstring &fieldName,
                       const std::wstring &outerSegmentSuffix,
                       const std::wstring &segmentSuffix);

private:
  class FieldsWriter : public FieldsConsumer
  {
    GET_CLASS_NAME(FieldsWriter)
  private:
    std::shared_ptr<PerFieldPostingsFormat> outerInstance;

  public:
    const std::shared_ptr<SegmentWriteState> writeState;
    const std::deque<std::shared_ptr<Closeable>> toClose =
        std::deque<std::shared_ptr<Closeable>>();

    FieldsWriter(std::shared_ptr<PerFieldPostingsFormat> outerInstance,
                 std::shared_ptr<SegmentWriteState> writeState);

    void write(std::shared_ptr<Fields> fields)  override;

  private:
    class FilterFieldsAnonymousInnerClass : public FilterFields
    {
      GET_CLASS_NAME(FilterFieldsAnonymousInnerClass)
    private:
      std::shared_ptr<FieldsWriter> outerInstance;

      std::shared_ptr<org::apache::lucene::codecs::perfield::
                          PerFieldPostingsFormat::FieldsGroup>
          group;

    public:
      FilterFieldsAnonymousInnerClass(
          std::shared_ptr<FieldsWriter> outerInstance,
          std::shared_ptr<Fields> fields,
          std::shared_ptr<org::apache::lucene::codecs::perfield::
                              PerFieldPostingsFormat::FieldsGroup>
              group);

      std::shared_ptr<Iterator<std::wstring>> iterator() override;

    protected:
      std::shared_ptr<FilterFieldsAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterFieldsAnonymousInnerClass>(
            org.apache.lucene.index.FilterLeafReader
                .FilterFields::shared_from_this());
      }
    };

  public:
    void
    merge(std::shared_ptr<MergeState> mergeState)  override;

  private:
    std::unordered_map<std::shared_ptr<PostingsFormat>,
                       std::shared_ptr<FieldsGroup>>
    buildFieldsGroupMapping(std::shared_ptr<Fields> fields);

  public:
    virtual ~FieldsWriter();

  protected:
    std::shared_ptr<FieldsWriter> shared_from_this()
    {
      return std::static_pointer_cast<FieldsWriter>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

private:
  class FieldsReader : public FieldsProducer
  {
    GET_CLASS_NAME(FieldsReader)

  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(FieldsReader::typeid);

    const std::unordered_map<std::wstring, std::shared_ptr<FieldsProducer>>
        fields = std::map_obj<std::wstring, std::shared_ptr<FieldsProducer>>();
    const std::unordered_map<std::wstring, std::shared_ptr<FieldsProducer>>
        formats =
            std::unordered_map<std::wstring, std::shared_ptr<FieldsProducer>>();
    const std::wstring segment;

    // clone for merge
  public:
    FieldsReader(std::shared_ptr<FieldsReader> other) ;

    FieldsReader(std::shared_ptr<SegmentReadState> readState) throw(
        IOException);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

    virtual ~FieldsReader();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<FieldsProducer>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<FieldsReader> shared_from_this()
    {
      return std::static_pointer_cast<FieldsReader>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

public:
  std::shared_ptr<FieldsConsumer>
  fieldsConsumer(std::shared_ptr<SegmentWriteState> state) throw(
      IOException) override final;

  std::shared_ptr<FieldsProducer>
  fieldsProducer(std::shared_ptr<SegmentReadState> state) throw(
      IOException) override final;

  /**
   * Returns the postings format that should be used for writing
   * new segments of <code>field</code>.
   * <p>
   * The field to format mapping is written to the index, so
   * this method is only invoked when writing, not when reading. */
  virtual std::shared_ptr<PostingsFormat>
  getPostingsFormatForField(const std::wstring &field) = 0;

protected:
  std::shared_ptr<PerFieldPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<PerFieldPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/perfield/
