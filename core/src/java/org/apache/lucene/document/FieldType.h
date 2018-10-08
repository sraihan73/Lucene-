#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexableFieldType.h"

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
namespace org::apache::lucene::document
{

using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;

/**
 * Describes the properties of a field.
 */
class FieldType : public std::enable_shared_from_this<FieldType>,
                  public IndexableFieldType
{
  GET_CLASS_NAME(FieldType)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool stored_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool tokenized_ = true;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool storeTermVectors_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool storeTermVectorOffsets_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool storeTermVectorPositions_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool storeTermVectorPayloads_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool omitNorms_ = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  IndexOptions indexOptions_ = IndexOptions::NONE;
  bool frozen = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  DocValuesType docValuesType_ = DocValuesType::NONE;
  int dimensionCount = 0;
  int dimensionNumBytes = 0;

  /**
   * Create a new mutable FieldType with all of the properties from
   * <code>ref</code>
   */
public:
  FieldType(std::shared_ptr<IndexableFieldType> ref);

  /**
   * Create a new FieldType with default properties.
   */
  FieldType();

  /**
   * Throws an exception if this FieldType is frozen. Subclasses should
   * call this within setters for additional state.
   */
protected:
  virtual void checkIfFrozen();

  /**
   * Prevents future changes. Note, it is recommended that this is called once
   * the FieldTypes's properties have been set, to prevent unintentional state
   * changes.
   */
public:
  virtual void freeze();

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setStored(bool)
   */
  bool stored() override;

  /**
   * Set to <code>true</code> to store this field.
   * @param value true if this field should be stored.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #stored()
   */
  virtual void setStored(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>true</code>.
   * @see #setTokenized(bool)
   */
  bool tokenized() override;

  /**
   * Set to <code>true</code> to tokenize this field's contents via the
   * configured {@link Analyzer}.
   * @param value true if this field should be tokenized.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #tokenized()
   */
  virtual void setTokenized(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setStoreTermVectors(bool)
   */
  bool storeTermVectors() override;

  /**
   * Set to <code>true</code> if this field's indexed form should be also stored
   * into term vectors.
   * @param value true if this field should store term vectors.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #storeTermVectors()
   */
  virtual void setStoreTermVectors(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setStoreTermVectorOffsets(bool)
   */
  bool storeTermVectorOffsets() override;

  /**
   * Set to <code>true</code> to also store token character offsets into the
   * term deque for this field.
   * @param value true if this field should store term deque offsets.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #storeTermVectorOffsets()
   */
  virtual void setStoreTermVectorOffsets(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setStoreTermVectorPositions(bool)
   */
  bool storeTermVectorPositions() override;

  /**
   * Set to <code>true</code> to also store token positions into the term
   * deque for this field.
   * @param value true if this field should store term deque positions.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #storeTermVectorPositions()
   */
  virtual void setStoreTermVectorPositions(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setStoreTermVectorPayloads(bool)
   */
  bool storeTermVectorPayloads() override;

  /**
   * Set to <code>true</code> to also store token payloads into the term
   * deque for this field.
   * @param value true if this field should store term deque payloads.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #storeTermVectorPayloads()
   */
  virtual void setStoreTermVectorPayloads(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>false</code>.
   * @see #setOmitNorms(bool)
   */
  bool omitNorms() override;

  /**
   * Set to <code>true</code> to omit normalization values for the field.
   * @param value true if this field should omit norms.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #omitNorms()
   */
  virtual void setOmitNorms(bool value);

  /**
   * {@inheritDoc}
   * <p>
   * The default is {@link IndexOptions#DOCS_AND_FREQS_AND_POSITIONS}.
   * @see #setIndexOptions(IndexOptions)
   */
  IndexOptions indexOptions() override;

  /**
   * Sets the indexing options for the field:
   * @param value indexing options
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #indexOptions()
   */
  virtual void setIndexOptions(IndexOptions value);

  /**
   * Enables points indexing.
   */
  virtual void setDimensions(int dimensionCount, int dimensionNumBytes);

  int pointDimensionCount() override;

  int pointNumBytes() override;

  /** Prints a Field for human consumption. */
  virtual std::wstring toString();

  /**
   * {@inheritDoc}
   * <p>
   * The default is <code>null</code> (no docValues)
   * @see #setDocValuesType(DocValuesType)
   */
  DocValuesType docValuesType() override;

  /**
   * Sets the field's DocValuesType
   * @param type DocValues type, or null if no DocValues should be stored.
   * @throws IllegalStateException if this FieldType is frozen against
   *         future modifications.
   * @see #docValuesType()
   */
  virtual void setDocValuesType(DocValuesType type);

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // #include  "core/src/java/org/apache/lucene/document/
