#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

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
namespace org::apache::lucene::facet
{

/** Single label and its value, usually contained in a
 *  {@link FacetResult}. */
class LabelAndValue final : public std::enable_shared_from_this<LabelAndValue>
{
  GET_CLASS_NAME(LabelAndValue)
  /** Facet's label. */
public:
  const std::wstring label;

  /** Value associated with this label. */
  const std::shared_ptr<Number> value;

  /** Sole constructor. */
  LabelAndValue(const std::wstring &label, std::shared_ptr<Number> value);

  virtual std::wstring toString();

  bool equals(std::any _other) override;

  virtual int hashCode();
};

} // namespace org::apache::lucene::facet
