using namespace std;

#include "StoredField.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<FieldType> StoredField::TYPE;

StoredField::StaticConstructor::StaticConstructor()
{
  TYPE = make_shared<FieldType>();
  TYPE->setStored(true);
  TYPE->freeze();
}

StoredField::StaticConstructor StoredField::staticConstructor;

StoredField::StoredField(const wstring &name, shared_ptr<FieldType> type)
    : Field(name, type)
{
}

StoredField::StoredField(const wstring &name, shared_ptr<BytesRef> bytes,
                         shared_ptr<FieldType> type)
    : Field(name, bytes, type)
{
}

StoredField::StoredField(const wstring &name, std::deque<char> &value)
    : Field(name, value, TYPE)
{
}

StoredField::StoredField(const wstring &name, std::deque<char> &value,
                         int offset, int length)
    : Field(name, value, offset, length, TYPE)
{
}

StoredField::StoredField(const wstring &name, shared_ptr<BytesRef> value)
    : Field(name, value, TYPE)
{
}

StoredField::StoredField(const wstring &name, const wstring &value)
    : Field(name, value, TYPE)
{
}

StoredField::StoredField(const wstring &name, const wstring &value,
                         shared_ptr<FieldType> type)
    : Field(name, value, type)
{
}

StoredField::StoredField(const wstring &name, int value) : Field(name, TYPE)
{
  fieldsData = value;
}

StoredField::StoredField(const wstring &name, float value) : Field(name, TYPE)
{
  fieldsData = value;
}

StoredField::StoredField(const wstring &name, int64_t value)
    : Field(name, TYPE)
{
  fieldsData = value;
}

StoredField::StoredField(const wstring &name, double value) : Field(name, TYPE)
{
  fieldsData = value;
}
} // namespace org::apache::lucene::document