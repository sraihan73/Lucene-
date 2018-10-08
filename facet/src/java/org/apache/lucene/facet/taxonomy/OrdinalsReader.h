#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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
namespace org::apache::lucene::facet::taxonomy
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IntsRef = org::apache::lucene::util::IntsRef;

/** Provides per-document ordinals. */

class OrdinalsReader : public std::enable_shared_from_this<OrdinalsReader>
{
  GET_CLASS_NAME(OrdinalsReader)

  /** Returns ordinals for documents in one segment. */
public:
  class OrdinalsSegmentReader
      : public std::enable_shared_from_this<OrdinalsSegmentReader>
  {
    GET_CLASS_NAME(OrdinalsSegmentReader)
    /** Get the ordinals for this document.  ordinals.offset
     *  must always be 0! */
  public:
    virtual void get(int doc, std::shared_ptr<IntsRef> ordinals) = 0;

    /** Default constructor. */
    OrdinalsSegmentReader();
  };

  /** Default constructor. */
public:
  OrdinalsReader();

  /** Set current atomic reader. */
  virtual std::shared_ptr<OrdinalsSegmentReader>
  getReader(std::shared_ptr<LeafReaderContext> context) = 0;

  /** Returns the indexed field name this {@code
   *  OrdinalsReader} is reading from. */
  virtual std::wstring getIndexFieldName() = 0;
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
