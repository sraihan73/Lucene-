#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

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

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Add an instance of this to your {@link Document} to add
 *  a facet label associated with an arbitrary byte[].
 *  This will require a custom {@link Facets}
 *  implementation at search time; see {@link
 *  IntAssociationFacetField} and {@link
 *  FloatAssociationFacetField} to use existing {@link
 *  Facets} implementations.
 *
 *  @lucene.experimental */
class AssociationFacetField : public Field
{
  GET_CLASS_NAME(AssociationFacetField)

  /** Indexed {@link FieldType}. */
public:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static AssociationFacetField::StaticConstructor staticConstructor;

  /** Dimension for this field. */
public:
  const std::wstring dim;

  /** Facet path for this field. */
  std::deque<std::wstring> const path;

  /** Associated value. */
  const std::shared_ptr<BytesRef> assoc;

  /** Creates this from {@code dim} and {@code path} and an
   *  association */
  AssociationFacetField(std::shared_ptr<BytesRef> assoc,
                        const std::wstring &dim,
                        std::deque<std::wstring> &path);

  virtual std::wstring toString();

protected:
  std::shared_ptr<AssociationFacetField> shared_from_this()
  {
    return std::static_pointer_cast<AssociationFacetField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
