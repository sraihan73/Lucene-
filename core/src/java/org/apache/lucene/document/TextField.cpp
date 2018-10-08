using namespace std;

#include "TextField.h"

namespace org::apache::lucene::document
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IndexOptions = org::apache::lucene::index::IndexOptions;
const shared_ptr<FieldType> TextField::TYPE_NOT_STORED =
    make_shared<FieldType>();
const shared_ptr<FieldType> TextField::TYPE_STORED = make_shared<FieldType>();

TextField::StaticConstructor::StaticConstructor()
{
  TYPE_NOT_STORED->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE_NOT_STORED->setTokenized(true);
  TYPE_NOT_STORED->freeze();

  TYPE_STORED->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE_STORED->setTokenized(true);
  TYPE_STORED->setStored(true);
  TYPE_STORED->freeze();
}

TextField::StaticConstructor TextField::staticConstructor;

TextField::TextField(const wstring &name, shared_ptr<Reader> reader)
    : Field(name, reader, TYPE_NOT_STORED)
{
}

TextField::TextField(const wstring &name, const wstring &value, Store store)
    : Field(name, value, store == Store::YES ? TYPE_STORED : TYPE_NOT_STORED)
{
}

TextField::TextField(const wstring &name, shared_ptr<TokenStream> stream)
    : Field(name, stream, TYPE_NOT_STORED)
{
}
} // namespace org::apache::lucene::document