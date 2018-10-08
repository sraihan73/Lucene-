#pragma once
#include "../../util/Attribute.h"
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
namespace org::apache::lucene::analysis::tokenattributes
{

using Attribute = org::apache::lucene::util::Attribute;

/** Determines how many positions this
 *  token spans.  Very few analyzer components actually
 *  produce this attribute, and indexing ignores it, but
 *  it's useful to express the graph structure naturally
 *  produced by decompounding, word splitting/joining,
 *  synonym filtering, etc.
 *
 * <p>NOTE: this is optional, and most analyzers
 *  don't change the default value (1). */

class PositionLengthAttribute : public Attribute
{
  GET_CLASS_NAME(PositionLengthAttribute)
  /**
   * Set the position length of this Token.
   * <p>
   * The default value is one.
   * @param positionLength how many positions this token
   *  spans.
   * @throws IllegalArgumentException if <code>positionLength</code>
   *         is zero or negative.
   * @see #getPositionLength()
   */
public:
  virtual void setPositionLength(int positionLength) = 0;

  /** Returns the position length of this Token.
   * @see #setPositionLength
   */
  virtual int getPositionLength() = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
