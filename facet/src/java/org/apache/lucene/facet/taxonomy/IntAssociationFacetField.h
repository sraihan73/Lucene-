#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;

/** Add an instance of this to your {@link Document} to add
 *  a facet label associated with an int.  Use {@link
 *  TaxonomyFacetSumIntAssociations} to aggregate int values
 *  per facet label at search time.
 *
 *  @lucene.experimental */
class IntAssociationFacetField : public AssociationFacetField
{
  GET_CLASS_NAME(IntAssociationFacetField)

  /** Creates this from {@code dim} and {@code path} and an
   *  int association */
public:
  IntAssociationFacetField(int assoc, const std::wstring &dim,
                           std::deque<std::wstring> &path);

  /** Encodes an {@code int} as a 4-byte {@link BytesRef},
   *  big-endian. */
  static std::shared_ptr<BytesRef> intToBytesRef(int v);

  /** Decodes a previously encoded {@code int}. */
  static int bytesRefToInt(std::shared_ptr<BytesRef> b);

  virtual std::wstring toString();

protected:
  std::shared_ptr<IntAssociationFacetField> shared_from_this()
  {
    return std::static_pointer_cast<IntAssociationFacetField>(
        AssociationFacetField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
