#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergeState.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"

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

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using MergeState = org::apache::lucene::index::MergeState;
using Terms = org::apache::lucene::index::Terms;

/**
 * Utility class to update the {@link MergeState} instance to be restricted to a
 * set of fields. <p> Warning: the input {@linkplain MergeState} instance will
 * be updated when calling {@link #apply(std::deque)}. <p> It should be called
 * within a {@code try &#123;...&#125; finally &#123;...&#125;} block to make
 * sure that the mergeState instance is restored to its original state: <pre>
 * PerFieldMergeState pfMergeState = new PerFieldMergeState(mergeState);
 * try {
 *   doSomething(pfMergeState.apply(fields));
 *   ...
 * } finally {
 *   pfMergeState.reset();
 * }
 * </pre>
 */
class PerFieldMergeState final
    : public std::enable_shared_from_this<PerFieldMergeState>
{
  GET_CLASS_NAME(PerFieldMergeState)
private:
  const std::shared_ptr<MergeState> in_;
  const std::shared_ptr<FieldInfos> orgMergeFieldInfos;
  std::deque<std::shared_ptr<FieldInfos>> const orgFieldInfos;
  std::deque<std::shared_ptr<FieldsProducer>> const orgFieldsProducers;

public:
  PerFieldMergeState(std::shared_ptr<MergeState> in_);

  /**
   * Update the input {@link MergeState} instance to restrict the fields to the
   * given ones.
   *
   * @param fields The fields to keep in the updated instance.
   * @return The updated instance.
   */
  std::shared_ptr<MergeState>
  apply(std::shared_ptr<std::deque<std::wstring>> fields);

  /**
   * Resets the input {@link MergeState} instance to its original state.
   *
   * @return The reset instance.
   */
  std::shared_ptr<MergeState> reset();

private:
  class FilterFieldInfos : public FieldInfos
  {
    GET_CLASS_NAME(FilterFieldInfos)
  private:
    const std::shared_ptr<Set<std::wstring>> filteredNames;
    const std::deque<std::shared_ptr<FieldInfo>> filtered;

    // Copy of the private fields from FieldInfos
    // Renamed so as to be less confusing about which fields we're referring to
    const bool filteredHasVectors;
    const bool filteredHasProx;
    const bool filteredHasPayloads;
    const bool filteredHasOffsets;
    const bool filteredHasFreq;
    const bool filteredHasNorms;
    const bool filteredHasDocValues;
    const bool filteredHasPointValues;

  public:
    FilterFieldInfos(std::shared_ptr<FieldInfos> src,
                     std::shared_ptr<std::deque<std::wstring>> filterFields);

  private:
    static std::deque<std::shared_ptr<FieldInfo>>
    toArray(std::shared_ptr<FieldInfos> src);

  public:
    std::shared_ptr<Iterator<std::shared_ptr<FieldInfo>>> iterator() override;

    bool hasFreq() override;

    bool hasProx() override;

    bool hasPayloads() override;

    bool hasOffsets() override;

    bool hasVectors() override;

    bool hasNorms() override;

    bool hasDocValues() override;

    bool hasPointValues() override;

    int size() override;

    std::shared_ptr<FieldInfo>
    fieldInfo(const std::wstring &fieldName) override;

    std::shared_ptr<FieldInfo> fieldInfo(int fieldNumber) override;

  protected:
    std::shared_ptr<FilterFieldInfos> shared_from_this()
    {
      return std::static_pointer_cast<FilterFieldInfos>(
          org.apache.lucene.index.FieldInfos::shared_from_this());
    }
  };

private:
  class FilterFieldsProducer : public FieldsProducer
  {
    GET_CLASS_NAME(FilterFieldsProducer)
  private:
    const std::shared_ptr<FieldsProducer> in_;
    const std::deque<std::wstring> filtered;

  public:
    FilterFieldsProducer(
        std::shared_ptr<FieldsProducer> in_,
        std::shared_ptr<std::deque<std::wstring>> filterFields);

    int64_t ramBytesUsed() override;

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

    virtual ~FilterFieldsProducer();

    void checkIntegrity()  override;

  protected:
    std::shared_ptr<FilterFieldsProducer> shared_from_this()
    {
      return std::static_pointer_cast<FilterFieldsProducer>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/codecs/perfield/
