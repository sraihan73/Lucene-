#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
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
namespace org::apache::lucene::facet::sortedset
{

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;

/** Add an instance of this to your Document for every facet
 *  label to be indexed via SortedSetDocValues. */
class SortedSetDocValuesFacetField : public Field
{
  GET_CLASS_NAME(SortedSetDocValuesFacetField)

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
  static SortedSetDocValuesFacetField::StaticConstructor staticConstructor;

  /** Dimension. */
public:
  const std::wstring dim;

  /** Label. */
  const std::wstring label;

  /** Sole constructor. */
  SortedSetDocValuesFacetField(const std::wstring &dim,
                               const std::wstring &label);

  virtual std::wstring toString();

protected:
  std::shared_ptr<SortedSetDocValuesFacetField> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValuesFacetField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::sortedset
