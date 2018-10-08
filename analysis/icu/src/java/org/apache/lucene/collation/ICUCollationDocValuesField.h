#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::collation
{

using Field = org::apache::lucene::document::Field;
using BytesRef = org::apache::lucene::util::BytesRef;

using com::ibm::icu::text::Collator;
using com::ibm::icu::text::RawCollationKey;

/**
 * Indexes collation keys as a single-valued {@link SortedDocValuesField}.
 * <p>
 * This is more efficient that {@link ICUCollationKeyAnalyzer} if the field
 * only has one value: no uninversion is necessary to sort on the field,
 * locale-sensitive range queries can still work via {@code
 * DocValuesRangeQuery}, and the underlying data structures built at index-time
 * are likely more efficient and use less memory than FieldCache.
 */
class ICUCollationDocValuesField final : public Field
{
  GET_CLASS_NAME(ICUCollationDocValuesField)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::wstring name_;
  const std::shared_ptr<Collator> collator;
  const std::shared_ptr<BytesRef> bytes = std::make_shared<BytesRef>();
  const std::shared_ptr<RawCollationKey> key =
      std::make_shared<RawCollationKey>();

  /**
   * Create a new ICUCollationDocValuesField.
   * <p>
   * NOTE: you should not create a new one for each document, instead
   * just make one and reuse it during your indexing process, setting
   * the value via {@link #setStringValue(std::wstring)}.
   * @param name field name
   * @param collator Collator for generating collation keys.
   */
  // TODO: can we make this trap-free? maybe just synchronize on the collator
  // instead?
public:
  ICUCollationDocValuesField(const std::wstring &name,
                             std::shared_ptr<Collator> collator);

  std::wstring name() override;

  void setStringValue(const std::wstring &value) override;

protected:
  std::shared_ptr<ICUCollationDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<ICUCollationDocValuesField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/collation/
