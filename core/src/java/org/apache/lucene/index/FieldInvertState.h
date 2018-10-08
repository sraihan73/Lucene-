#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TermFrequencyAttribute.h"

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

using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * This class tracks the number and position / offset parameters of terms
 * being added to the index. The information collected in this class is
 * also used to calculate the normalization factor for a field.
 *
 * @lucene.experimental
 */
class FieldInvertState final
    : public std::enable_shared_from_this<FieldInvertState>
{
  GET_CLASS_NAME(FieldInvertState)
public:
  const int indexCreatedVersionMajor;
  const std::wstring name;
  int position = 0;
  int length = 0;
  int numOverlap = 0;
  int offset = 0;
  int maxTermFrequency = 0;
  int uniqueTermCount = 0;
  // we must track these across field instances (multi-valued case)
  int lastStartOffset = 0;
  int lastPosition = 0;
  std::shared_ptr<AttributeSource> attributeSource;

  std::shared_ptr<OffsetAttribute> offsetAttribute;
  std::shared_ptr<PositionIncrementAttribute> posIncrAttribute;
  std::shared_ptr<PayloadAttribute> payloadAttribute;
  std::shared_ptr<TermToBytesRefAttribute> termAttribute;
  std::shared_ptr<TermFrequencyAttribute> termFreqAttribute;

  /** Creates {code FieldInvertState} for the specified
   *  field name. */
  FieldInvertState(int indexCreatedVersionMajor, const std::wstring &name);

  /** Creates {code FieldInvertState} for the specified
   *  field name and values for all fields. */
  FieldInvertState(int indexCreatedVersionMajor, const std::wstring &name,
                   int position, int length, int numOverlap, int offset);

  /**
   * Re-initialize the state
   */
  void reset();

  // TODO: better name?
  /**
   * Sets attributeSource to a new instance.
   */
  void setAttributeSource(std::shared_ptr<AttributeSource> attributeSource);

  /**
   * Get the last processed term position.
   * @return the position
   */
  int getPosition();

  /**
   * Get total number of terms in this field.
   * @return the length
   */
  int getLength();

  /** Set length value. */
  void setLength(int length);

  /**
   * Get the number of terms with <code>positionIncrement == 0</code>.
   * @return the numOverlap
   */
  int getNumOverlap();

  /** Set number of terms with {@code positionIncrement ==
   *  0}. */
  void setNumOverlap(int numOverlap);

  /**
   * Get end offset of the last processed term.
   * @return the offset
   */
  int getOffset();

  /**
   * Get the maximum term-frequency encountered for any term in the field.  A
   * field containing "the quick brown fox jumps over the lazy dog" would have
   * a value of 2, because "the" appears twice.
   */
  int getMaxTermFrequency();

  /**
   * Return the number of unique terms encountered in this field.
   */
  int getUniqueTermCount();

  /** Returns the {@link AttributeSource} from the {@link
   *  TokenStream} that provided the indexed tokens for this
   *  field. */
  std::shared_ptr<AttributeSource> getAttributeSource();

  /**
   * Return the field's name
   */
  std::wstring getName();

  /**
   * Return the version that was used to create the index, or 6 if it was
   * created before 7.0.
   */
  int getIndexCreatedVersionMajor();
};

} // #include  "core/src/java/org/apache/lucene/index/
