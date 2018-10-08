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
 * Controls how much information is stored in the postings lists.
 * @lucene.experimental
 */

enum class IndexOptions {
  GET_CLASS_NAME(IndexOptions)
  // NOTE: order is important here; FieldInfo uses this
  // order to merge two conflicting IndexOptions (always
  // "downgrades" by picking the lowest).
  /** Not indexed */
  NONE,
  /**
   * Only documents are indexed: term frequencies and positions are omitted.
   * Phrase and other positional queries on the field will throw an exception,
   * and scoring will behave as if any term in the document appears only once.
   */
  DOCS,
  /**
   * Only documents and term frequencies are indexed: positions are omitted.
   * This enables normal scoring, except Phrase and other positional queries
   * will throw an exception.
   */
  DOCS_AND_FREQS,
  /**
   * Indexes documents, frequencies and positions.
   * This is a typical default for full-text search: full scoring is enabled
   * and positional queries are supported.
   */
  DOCS_AND_FREQS_AND_POSITIONS,
  /**
   * Indexes documents, frequencies, positions and offsets.
   * Character offsets are encoded alongside the positions.
   */
  DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
};

} // namespace org::apache::lucene::index
