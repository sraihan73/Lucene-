using namespace std;

#include "StringField.h"

namespace org::apache::lucene::document
{
using IndexOptions = org::apache::lucene::index::IndexOptions;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<FieldType> StringField::TYPE_NOT_STORED =
    make_shared<FieldType>();
const shared_ptr<FieldType> StringField::TYPE_STORED = make_shared<FieldType>();

StringField::StaticConstructor::StaticConstructor()
{
  TYPE_NOT_STORED->setOmitNorms(true);
  TYPE_NOT_STORED->setIndexOptions(IndexOptions::DOCS);
  TYPE_NOT_STORED->setTokenized(false);
  TYPE_NOT_STORED->freeze();

  TYPE_STORED->setOmitNorms(true);
  TYPE_STORED->setIndexOptions(IndexOptions::DOCS);
  TYPE_STORED->setStored(true);
  TYPE_STORED->setTokenized(false);
  TYPE_STORED->freeze();
}

StringField::StaticConstructor StringField::staticConstructor;

StringField::StringField(const wstring &name, const wstring &value,
                         Store stored)
    : Field(name, value, stored == Store::YES ? TYPE_STORED : TYPE_NOT_STORED)
{
}

StringField::StringField(const wstring &name, shared_ptr<BytesRef> value,
                         Store stored)
    : Field(name, value, stored == Store::YES ? TYPE_STORED : TYPE_NOT_STORED)
{
}
} // namespace org::apache::lucene::document