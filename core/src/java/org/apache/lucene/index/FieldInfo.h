#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
 *  Access to the Field Info file that describes document fields and whether or
 *  not they are indexed. Each segment has a separate Field Info file. Objects
 *  of this class are thread-safe for multiple readers, but only one thread can
 *  be adding documents at a time, with no other reader or writer threads
 *  accessing this object.
 **/

class FieldInfo final : public std::enable_shared_from_this<FieldInfo>
{
  GET_CLASS_NAME(FieldInfo)
  /** Field's name */
public:
  const std::wstring name;
  /** Internal field number */
  const int number;

private:
  DocValuesType docValuesType = DocValuesType::NONE;

  // True if any document indexed term vectors
  bool storeTermVector = false;

  bool omitNorms = false; // omit norms associated with indexed fields

  IndexOptions indexOptions = IndexOptions::NONE;
  bool storePayloads =
      false; // whether this field stores payloads together with term positions

  // C++ NOTE: Fields cannot have the same name as methods:
  const std::unordered_map<std::wstring, std::wstring> attributes_;

  int64_t dvGen = 0;

  /** If both of these are positive it means this field indexed points
   *  (see {@link org.apache.lucene.codecs.PointsFormat}). */
  int pointDimensionCount = 0;
  int pointNumBytes = 0;

  // whether this field is used as the soft-deletes field
  const bool softDeletesField;

  /**
   * Sole constructor.
   *
   * @lucene.experimental
   */
public:
  FieldInfo(const std::wstring &name, int number, bool storeTermVector,
            bool omitNorms, bool storePayloads, IndexOptions indexOptions,
            DocValuesType docValues, int64_t dvGen,
            std::unordered_map<std::wstring, std::wstring> &attributes,
            int pointDimensionCount, int pointNumBytes, bool softDeletesField);

  /**
   * Performs internal consistency checks.
   * Always returns true (or throws IllegalStateException)
   */
  bool checkConsistency();

  // should only be called by FieldInfos#addOrUpdate
  void update(bool storeTermVector, bool omitNorms, bool storePayloads,
              IndexOptions indexOptions, int dimensionCount,
              int dimensionNumBytes);

  /** Record that this field is indexed with points, with the
   *  specified number of dimensions and bytes per dimension. */
  void setPointDimensions(int count, int numBytes);

  /** Return point dimension count */
  int getPointDimensionCount();

  /** Return number of bytes per dimension */
  int getPointNumBytes();

  /** Record that this field is indexed with docvalues, with the specified type
   */
  void setDocValuesType(DocValuesType type);

  /** Returns IndexOptions for the field, or IndexOptions.NONE if the field is
   * not indexed */
  IndexOptions getIndexOptions();

  /** Record the {@link IndexOptions} to use with this field. */
  void setIndexOptions(IndexOptions newIndexOptions);

  /**
   * Returns {@link DocValuesType} of the docValues; this is
   * {@code DocValuesType.NONE} if the field has no docvalues.
   */
  DocValuesType getDocValuesType();

  /** Sets the docValues generation of this field. */
  void setDocValuesGen(int64_t dvGen);

  /**
   * Returns the docValues generation of this field, or -1 if no docValues
   * updates exist for it.
   */
  int64_t getDocValuesGen();

  void setStoreTermVectors();

  void setStorePayloads();

  /**
   * Returns true if norms are explicitly omitted for this field
   */
  bool omitsNorms();

  /** Omit norms for this field. */
  void setOmitsNorms();

  /**
   * Returns true if this field actually has any norms.
   */
  bool hasNorms();

  /**
   * Returns true if any payloads exist for this field.
   */
  bool hasPayloads();

  /**
   * Returns true if any term vectors exist for this field.
   */
  bool hasVectors();

  /**
   * Get a codec attribute value, or null if it does not exist
   */
  std::wstring getAttribute(const std::wstring &key);

  /**
   * Puts a codec attribute value.
   * <p>
   * This is a key-value mapping for the field that the codec can use
   * to store additional metadata, and will be available to the codec
   * when reading the segment via {@link #getAttribute(std::wstring)}
   * <p>
   * If a value already exists for the field, it will be replaced with
   * the new value.
   */
  std::wstring putAttribute(const std::wstring &key, const std::wstring &value);

  /**
   * Returns internal codec attributes map_obj.
   */
  std::unordered_map<std::wstring, std::wstring> attributes();

  /**
   * Returns true if this field is configured and used as the soft-deletes
   * field. See {@link IndexWriterConfig#softDeletesField}
   */
  bool isSoftDeletesField();
};

} // namespace org::apache::lucene::index
