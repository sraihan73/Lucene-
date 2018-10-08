#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Attribute.h"
#include "stringhelper.h"
#include <deque>

// -*- c-basic-offset: 2 -*-
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
namespace org::apache::lucene::analysis::morfologik
{

using Attribute = org::apache::lucene::util::Attribute;

/**
 * Morfologik provides morphosyntactic annotations for
 * surface forms. For the exact format and description of these,
 * see the project's documentation.
 */
class MorphosyntacticTagsAttribute : public Attribute
{
  GET_CLASS_NAME(MorphosyntacticTagsAttribute)
  /**
   * Set the POS tag. The default value (no-value) is null.
   *
   * @param tags A deque of POS tags corresponding to current lemma.
   */
public:
  virtual void setTags(std::deque<std::shared_ptr<StringBuilder>> &tags) = 0;

  /**
   * Returns the POS tag of the term. A single word may have multiple POS tags,
   * depending on the interpretation (context disambiguation is typically needed
   * to determine which particular tag is appropriate).
   */
  virtual std::deque<std::shared_ptr<StringBuilder>> getTags() = 0;

  /** Clear to default value. */
  virtual void clear() = 0;
};

} // namespace org::apache::lucene::analysis::morfologik
