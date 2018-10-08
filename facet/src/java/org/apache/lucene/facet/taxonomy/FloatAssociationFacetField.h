#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

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
 *  a facet label associated with a float.  Use {@link
 *  TaxonomyFacetSumFloatAssociations} to aggregate float values
 *  per facet label at search time.
 *
 *  @lucene.experimental */
class FloatAssociationFacetField : public AssociationFacetField
{
  GET_CLASS_NAME(FloatAssociationFacetField)

  /** Creates this from {@code dim} and {@code path} and a
   *  float association */
public:
  FloatAssociationFacetField(float assoc, const std::wstring &dim,
                             std::deque<std::wstring> &path);

  /** Encodes a {@code float} as a 4-byte {@link BytesRef}. */
  static std::shared_ptr<BytesRef> floatToBytesRef(float v);

  /** Decodes a previously encoded {@code float}. */
  static float bytesRefToFloat(std::shared_ptr<BytesRef> b);

  virtual std::wstring toString();

protected:
  std::shared_ptr<FloatAssociationFacetField> shared_from_this()
  {
    return std::static_pointer_cast<FloatAssociationFacetField>(
        AssociationFacetField::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
