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

/** Sets the custom term frequency of a term within one document.  If this
 * attribute is present in your analysis chain for a given field, that field
 * must be indexed with
 *  {@link IndexOptions#DOCS_AND_FREQS}. */
class TermFrequencyAttribute : public Attribute
{
  GET_CLASS_NAME(TermFrequencyAttribute)

  /** Set the custom term frequency of the current term within one document. */
public:
  virtual void setTermFrequency(int termFrequency) = 0;

  /** Returns the custom term frequencey. */
  virtual int getTermFrequency() = 0;
};

} // namespace org::apache::lucene::analysis::tokenattributes
