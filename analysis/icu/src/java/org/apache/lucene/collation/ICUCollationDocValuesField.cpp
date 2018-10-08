using namespace std;

#include "ICUCollationDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::collation
{
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using BytesRef = org::apache::lucene::util::BytesRef;
using com::ibm::icu::text::Collator;
using com::ibm::icu::text::RawCollationKey;

ICUCollationDocValuesField::ICUCollationDocValuesField(
    const wstring &name, shared_ptr<Collator> collator)
    : org::apache::lucene::document::Field(name, SortedDocValuesField::TYPE),
      name(name)
{
  try {
    this->collator = std::static_pointer_cast<Collator>(collator->clone());
  } catch (const CloneNotSupportedException &e) {
    throw runtime_error(e);
  }
  fieldsData = bytes; // so wrong setters cannot be called
}

wstring ICUCollationDocValuesField::name() { return name_; }

void ICUCollationDocValuesField::setStringValue(const wstring &value)
{
  collator->getRawCollationKey(value, key);
  bytes->bytes = key->bytes;
  bytes->offset = 0;
  bytes->length = key->size;
}
} // namespace org::apache::lucene::collation