#pragma once
#include "stringhelper.h"
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
 * Describes the properties of a field.
 * @lucene.experimental
 */
class IndexableFieldType
{
  GET_CLASS_NAME(IndexableFieldType)

  /** True if the field's value should be stored */
public:
  virtual bool stored() = 0;

  /**
   * True if this field's value should be analyzed by the
   * {@link Analyzer}.
   * <p>
   * This has no effect if {@link #indexOptions()} returns
   * IndexOptions.NONE.
   */
  // TODO: shouldn't we remove this?  Whether/how a field is
  // tokenized is an impl detail under Field?
  virtual bool tokenized() = 0;

  /**
   * True if this field's indexed form should be also stored
   * into term vectors.
   * <p>
   * This builds a miniature inverted-index for this field which
   * can be accessed in a document-oriented way from
   * {@link IndexReader#getTermVector(int,std::wstring)}.
   * <p>
   * This option is illegal if {@link #indexOptions()} returns
   * IndexOptions.NONE.
   */
  virtual bool storeTermVectors() = 0;

  /**
   * True if this field's token character offsets should also
   * be stored into term vectors.
   * <p>
   * This option is illegal if term vectors are not enabled for the field
   * ({@link #storeTermVectors()} is false)
   */
  virtual bool storeTermVectorOffsets() = 0;

  /**
   * True if this field's token positions should also be stored
   * into the term vectors.
   * <p>
   * This option is illegal if term vectors are not enabled for the field
   * ({@link #storeTermVectors()} is false).
   */
  virtual bool storeTermVectorPositions() = 0;

  /**
   * True if this field's token payloads should also be stored
   * into the term vectors.
   * <p>
   * This option is illegal if term deque positions are not enabled
   * for the field ({@link #storeTermVectors()} is false).
   */
  virtual bool storeTermVectorPayloads() = 0;

  /**
   * True if normalization values should be omitted for the field.
   * <p>
   * This saves memory, but at the expense of scoring quality (length
   * normalization will be disabled), and if you omit norms, you cannot use
   * index-time boosts.
   */
  virtual bool omitNorms() = 0;

  /** {@link IndexOptions}, describing what should be
   *  recorded into the inverted index */
  virtual IndexOptions indexOptions() = 0;

  /**
   * DocValues {@link DocValuesType}: how the field's value will be indexed
   * into docValues.
   */
  virtual DocValuesType docValuesType() = 0;

  /**
   * If this is positive, the field is indexed as a point.
   */
  virtual int pointDimensionCount() = 0;

  /**
   * The number of bytes in each dimension's values.
   */
  virtual int pointNumBytes() = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
