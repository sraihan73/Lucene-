#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Fields.h"

#include  "core/src/java/org/apache/lucene/index/MergeState.h"

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
namespace org::apache::lucene::codecs
{

using Fields = org::apache::lucene::index::Fields;
using MergeState = org::apache::lucene::index::MergeState;

/**
 * Abstract API that consumes terms, doc, freq, prox, offset and
 * payloads postings.  Concrete implementations of this
 * actually do "something" with the postings (write it into
 * the index in a specific format).
 *
 * @lucene.experimental
 */

class FieldsConsumer : public std::enable_shared_from_this<FieldsConsumer>
{
  GET_CLASS_NAME(FieldsConsumer)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  FieldsConsumer();

  // TODO: can we somehow compute stats for you...?

  // TODO: maybe we should factor out "limited" (only
  // iterables, no counts/stats) base classes from
  // Fields/Terms/Docs/AndPositions?

  /** Write all fields, terms and postings.  This the "pull"
   *  API, allowing you to iterate more than once over the
   *  postings, somewhat analogous to using a DOM API to
   *  traverse an XML tree.
   *
   *  <p><b>Notes</b>:
   *
   *  <ul>
   *    <li> You must compute index statistics,
   *         including each Term's docFreq and totalTermFreq,
   *         as well as the summary sumTotalTermFreq,
   *         sumTotalDocFreq and docCount.
   *
   *    <li> You must skip terms that have no docs and
   *         fields that have no terms, even though the provided
   *         Fields API will expose them; this typically
   *         requires lazily writing the field or term until
   *         you've actually seen the first term or
   *         document.
   *
   *    <li> The provided Fields instance is limited: you
   *         cannot call any methods that return
   *         statistics/counts; you cannot pass a non-null
   *         live docs when pulling docs/positions enums.
   *  </ul>
   */
public:
  virtual void write(std::shared_ptr<Fields> fields) = 0;

  /** Merges in the fields from the readers in
   *  <code>mergeState</code>. The default implementation skips
   *  and maps around deleted documents, and calls {@link #write(Fields)}.
   *  Implementations can override this method for more sophisticated
   *  merging (bulk-byte copying, etc). */
  virtual void merge(std::shared_ptr<MergeState> mergeState) ;

  // NOTE: strange but necessary so javadocs linting is happy:
  void close() = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/codecs/
