#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::index
{

using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/** Iterates over terms in across multiple fields.  The caller must
 *  check {@link #field} after each {@link #next} to see if the field
 *  changed, but {@code ==} can be used since the iterator
 *  implementation ensures it will use the same std::wstring instance for
 *  a given field. */

class FieldTermIterator
    : public std::enable_shared_from_this<FieldTermIterator>,
      public BytesRefIterator
{
  GET_CLASS_NAME(FieldTermIterator)
  /** Returns current field.  This method should not be called
   *  after iteration is done.  Note that you may use == to
   *  detect a change in field. */
public:
  virtual std::wstring field() = 0;

  /** Del gen of the current term. */
  // TODO: this is really per-iterator not per term, but when we use
  // MergedPrefixCodedTermsIterator we need to know which iterator we are on
  virtual int64_t delGen() = 0;
};

} // namespace org::apache::lucene::index
