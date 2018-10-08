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

/**
 * The start and end character offset of a Token.
 */
class OffsetAttribute : public Attribute
{
  GET_CLASS_NAME(OffsetAttribute)
  /**
   * Returns this Token's starting offset, the position of the first character
   * corresponding to this token in the source text.
   * <p>
   * Note that the difference between {@link #endOffset()} and
   * <code>startOffset()</code> may not be equal to termText.length(), as the
   * term text may have been altered by a stemmer or some other filter.
   * @see #setOffset(int, int)
   */
public:
  virtual int startOffset() = 0;

  /**
   * Set the starting and ending offset.
   * @throws IllegalArgumentException If <code>startOffset</code> or
   * <code>endOffset</code> are negative, or if <code>startOffset</code> is
   * greater than <code>endOffset</code>
   * @see #startOffset()
   * @see #endOffset()
   */
  virtual void setOffset(int startOffset, int endOffset) = 0;

  /**
   * Returns this Token's ending offset, one greater than the position of the
   * last character corresponding to this token in the source text. The length
   * of the token in the source text is (<code>endOffset()</code> - {@link
   * #startOffset()}).
   * @see #setOffset(int, int)
   */
  virtual int endOffset() = 0;
};

} // namespace org::apache::lucene::analysis::tokenattributes
