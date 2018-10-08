#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

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

/**
 * std::deque of {@link FieldInfo}s (accessible by number or by name).
 *  @lucene.experimental
 */
class FieldInfos : public std::enable_shared_from_this<FieldInfos>,
                   public std::deque<std::shared_ptr<FieldInfo>>
{
  GET_CLASS_NAME(FieldInfos)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasFreq_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasProx_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPayloads_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasOffsets_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasVectors_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasNorms_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasDocValues_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPointValues_;
  const std::wstring softDeletesField;

  // used only by fieldInfo(int)
  std::deque<std::shared_ptr<FieldInfo>> const byNumber;

  const std::unordered_map<std::wstring, std::shared_ptr<FieldInfo>> byName =
      std::unordered_map<std::wstring, std::shared_ptr<FieldInfo>>();
  const std::shared_ptr<std::deque<std::shared_ptr<FieldInfo>>>
      values; // for an unmodifiable iterator

  /**
   * Constructs a new FieldInfos from an array of FieldInfo objects
   */
public:
  FieldInfos(std::deque<std::shared_ptr<FieldInfo>> &infos);

  /** Returns true if any fields have freqs */
  virtual bool hasFreq();

  /** Returns true if any fields have positions */
  virtual bool hasProx();

  /** Returns true if any fields have payloads */
  virtual bool hasPayloads();

  /** Returns true if any fields have offsets */
  virtual bool hasOffsets();

  /** Returns true if any fields have vectors */
  virtual bool hasVectors();

  /** Returns true if any fields have norms */
  virtual bool hasNorms();

  /** Returns true if any fields have DocValues */
  virtual bool hasDocValues();

  /** Returns true if any fields have PointValues */
  virtual bool hasPointValues();

  /** Returns the soft-deletes field name if exists; otherwise returns null */
  virtual std::wstring getSoftDeletesField();

  /** Returns the number of fields */
  virtual int size();

  /**
   * Returns an iterator over all the fieldinfo objects present,
   * ordered by ascending field number
   */
  // TODO: what happens if in fact a different order is used?
  std::shared_ptr<Iterator<std::shared_ptr<FieldInfo>>> iterator() override;

  /**
   * Return the fieldinfo object referenced by the field name
   * @return the FieldInfo object or null when the given fieldName
   * doesn't exist.
   */
  virtual std::shared_ptr<FieldInfo> fieldInfo(const std::wstring &fieldName);

  /**
   * Return the fieldinfo object referenced by the fieldNumber.
   * @param fieldNumber field's number.
   * @return the FieldInfo object or null when the given fieldNumber
   * doesn't exist.
   * @throws IllegalArgumentException if fieldNumber is negative
   */
  virtual std::shared_ptr<FieldInfo> fieldInfo(int fieldNumber);

public:
  class FieldDimensions final
      : public std::enable_shared_from_this<FieldDimensions>
  {
    GET_CLASS_NAME(FieldDimensions)
  public:
    const int dimensionCount;
    const int dimensionNumBytes;

    FieldDimensions(int dimensionCount, int dimensionNumBytes);
  };

public:
  class FieldNumbers final : public std::enable_shared_from_this<FieldNumbers>
  {
    GET_CLASS_NAME(FieldNumbers)

  private:
    const std::unordered_map<int, std::wstring> numberToName;
    const std::unordered_map<std::wstring, int> nameToNumber;
    // We use this to enforce that a given field never
    // changes DV type, even across segments / IndexWriter
    // sessions:
    const std::unordered_map<std::wstring, DocValuesType> docValuesType;

    const std::unordered_map<std::wstring, std::shared_ptr<FieldDimensions>>
        dimensions;

    // TODO: we should similarly catch an attempt to turn
    // norms back on after they were already ommitted; today
    // we silently discard the norm but this is badly trappy
    int lowestUnassignedFieldNumber = -1;

    // The soft-deletes field from IWC to enforce a single soft-deletes field
    const std::wstring softDeletesFieldName;

  public:
    FieldNumbers(const std::wstring &softDeletesFieldName);

    /**
     * Returns the global field number for the given field name. If the name
     * does not exist yet it tries to add it with the given preferred field
     * number assigned if possible otherwise the first unassigned field number
     * is used as the field number.
     */
    // C++ WARNING: The following method was originally marked 'synchronized':
    int addOrGet(const std::wstring &fieldName, int preferredFieldNumber,
                 DocValuesType dvType, int dimensionCount,
                 int dimensionNumBytes, bool isSoftDeletesField);

    // C++ WARNING: The following method was originally marked 'synchronized':
    void verifyConsistent(std::optional<int> &number, const std::wstring &name,
                          DocValuesType dvType);

    // C++ WARNING: The following method was originally marked 'synchronized':
    void verifyConsistentDimensions(std::optional<int> &number,
                                    const std::wstring &name,
                                    int dimensionCount, int dimensionNumBytes);

    /**
     * Returns true if the {@code fieldName} exists in the map_obj and is of the
     * same {@code dvType}.
     */
    // C++ WARNING: The following method was originally marked 'synchronized':
    bool contains(const std::wstring &fieldName, DocValuesType dvType);

    // C++ WARNING: The following method was originally marked 'synchronized':
    std::shared_ptr<Set<std::wstring>> getFieldNames();

    // C++ WARNING: The following method was originally marked 'synchronized':
    void clear();

    // C++ WARNING: The following method was originally marked 'synchronized':
    void setDocValuesType(int number, const std::wstring &name,
                          DocValuesType dvType);

    // C++ WARNING: The following method was originally marked 'synchronized':
    void setDimensions(int number, const std::wstring &name, int dimensionCount,
                       int dimensionNumBytes);
  };

public:
  class Builder final : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const std::unordered_map<std::wstring, std::shared_ptr<FieldInfo>> byName =
        std::unordered_map<std::wstring, std::shared_ptr<FieldInfo>>();

  public:
    const std::shared_ptr<FieldNumbers> globalFieldNumbers;

  private:
    bool finished = false;

    /**
     * Creates a new instance with the given {@link FieldNumbers}.
     */
  public:
    Builder(std::shared_ptr<FieldNumbers> globalFieldNumbers);

    void add(std::shared_ptr<FieldInfos> other);

    /** Create a new field, or return existing one. */
    std::shared_ptr<FieldInfo> getOrAdd(const std::wstring &name);

  private:
    std::shared_ptr<FieldInfo> addOrUpdateInternal(
        const std::wstring &name, int preferredFieldNumber,
        bool storeTermVector, bool omitNorms, bool storePayloads,
        IndexOptions indexOptions, DocValuesType docValues, int64_t dvGen,
        int dimensionCount, int dimensionNumBytes, bool isSoftDeletesField);

  public:
    std::shared_ptr<FieldInfo> add(std::shared_ptr<FieldInfo> fi);

    std::shared_ptr<FieldInfo> add(std::shared_ptr<FieldInfo> fi,
                                   int64_t dvGen);

    std::shared_ptr<FieldInfo> fieldInfo(const std::wstring &fieldName);

    /** Called only from assert */
  private:
    bool assertNotFinished();

  public:
    std::shared_ptr<FieldInfos> finish();
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
