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
 * This attribute can be used to pass different flags down the {@link Tokenizer}
 * chain, e.g. from one TokenFilter to another one. <p> This is completely
 * distinct from {@link TypeAttribute}, although they do share similar purposes.
 * The flags can be used to encode information about the token for use by other
 * {@link org.apache.lucene.analysis.TokenFilter}s.
 * @lucene.experimental While we think this is here to stay, we may want to
 * change it to be a long.
 */
class FlagsAttribute : public Attribute
{
  GET_CLASS_NAME(FlagsAttribute)
  /**
   * Get the bitset for any bits that have been set.
   * @return The bits
   * @see #getFlags()
   */
public:
  virtual int getFlags() = 0;

  /**
   * Set the flags to a new bitset.
   * @see #getFlags()
   */
  virtual void setFlags(int flags) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
