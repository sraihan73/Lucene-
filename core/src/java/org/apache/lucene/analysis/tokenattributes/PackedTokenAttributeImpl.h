#pragma once
#include "CharTermAttributeImpl.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "PositionLengthAttribute.h"
#include "TermFrequencyAttribute.h"
#include "TypeAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"

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
namespace org::apache::lucene::analysis::tokenattributes
{

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

/** Default implementation of the common attributes used by Lucene:<ul>
 * <li>{@link CharTermAttribute}
 * <li>{@link TypeAttribute}
 * <li>{@link PositionIncrementAttribute}
 * <li>{@link PositionLengthAttribute}
 * <li>{@link OffsetAttribute}
 * <li>{@link TermFrequencyAttribute}
 * </ul>*/
class PackedTokenAttributeImpl : public CharTermAttributeImpl,
                                 public TypeAttribute,
                                 public PositionIncrementAttribute,
                                 public PositionLengthAttribute,
                                 public OffsetAttribute,
                                 public TermFrequencyAttribute
{
  GET_CLASS_NAME(PackedTokenAttributeImpl)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int startOffset_ = 0, endOffset_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::wstring type_ = DEFAULT_TYPE;
  int positionIncrement = 1;
  int positionLength = 1;
  int termFrequency = 1;

  /** Constructs the attribute implementation. */
public:
  PackedTokenAttributeImpl();

  /**
   * {@inheritDoc}
   * @see PositionIncrementAttribute
   */
  void setPositionIncrement(int positionIncrement) override;

  /**
   * {@inheritDoc}
   * @see PositionIncrementAttribute
   */
  int getPositionIncrement() override;

  /**
   * {@inheritDoc}
   * @see PositionLengthAttribute
   */
  void setPositionLength(int positionLength) override;

  /**
   * {@inheritDoc}
   * @see PositionLengthAttribute
   */
  int getPositionLength() override;

  /**
   * {@inheritDoc}
   * @see OffsetAttribute
   */
  int startOffset() override;

  /**
   * {@inheritDoc}
   * @see OffsetAttribute
   */
  int endOffset() override;

  /**
   * {@inheritDoc}
   * @see OffsetAttribute
   */
  void setOffset(int startOffset, int endOffset) override;

  /**
   * {@inheritDoc}
   * @see TypeAttribute
   */
  std::wstring type() override;

  /**
   * {@inheritDoc}
   * @see TypeAttribute
   */
  void setType(const std::wstring &type) override;

  void setTermFrequency(int termFrequency) override;

  int getTermFrequency() override;

  /** Resets the attributes
   */
  void clear() override;

  /** Resets the attributes at end
   */
  void end() override;

  std::shared_ptr<PackedTokenAttributeImpl> clone() override;

  bool equals(std::any obj) override;

  virtual int hashCode();

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<PackedTokenAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<PackedTokenAttributeImpl>(
        CharTermAttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
