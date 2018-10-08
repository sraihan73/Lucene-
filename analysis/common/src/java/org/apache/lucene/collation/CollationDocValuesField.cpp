using namespace std;

#include "CollationDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::collation
{
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using BytesRef = org::apache::lucene::util::BytesRef;

CollationDocValuesField::CollationDocValuesField(const wstring &name,
                                                 shared_ptr<Collator> collator)
    : org::apache::lucene::document::Field(name, SortedDocValuesField::TYPE),
      name(name),
      collator(std::static_pointer_cast<Collator>(collator->clone()))
{
  fieldsData = bytes; // so wrong setters cannot be called
}

wstring CollationDocValuesField::name() { return name_; }

void CollationDocValuesField::setStringValue(const wstring &value)
{
  bytes->bytes = collator->getCollationKey(value).toByteArray();
  bytes->offset = 0;
  bytes->length = bytes->bytes.size();
}
} // namespace org::apache::lucene::collation