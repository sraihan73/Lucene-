using namespace std;

#include "LazyDocument.h"

namespace org::apache::lucene::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

LazyDocument::LazyDocument(shared_ptr<IndexReader> reader, int docID)
    : reader(reader), docID(docID)
{
}

shared_ptr<IndexableField>
LazyDocument::getField(shared_ptr<FieldInfo> fieldInfo)
{

  fieldNames->add(fieldInfo->name);
  deque<std::shared_ptr<LazyField>> values = fields[fieldInfo->number];
  if (nullptr == values) {
    values = deque<>();
    fields.emplace(fieldInfo->number, values);
  }

  shared_ptr<LazyField> value = make_shared<LazyField>(
      shared_from_this(), fieldInfo->name, fieldInfo->number);
  values.push_back(value);

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    // edge case: if someone asks this LazyDoc for more LazyFields
    // after other LazyFields from the same LazyDoc have been
    // actuallized, we need to force the doc to be re-fetched
    // so the new LazyFields are also populated.
    doc.reset();
  }
  return value;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Document> LazyDocument::getDocument()
{
  if (doc->empty()) {
    try {
      doc = reader->document(docID, fieldNames);
    } catch (const IOException &ioe) {
      throw make_shared<IllegalStateException>(L"unable to load document", ioe);
    }
  }
  return doc;
}

void LazyDocument::fetchRealValues(const wstring &name, int fieldNum)
{
  shared_ptr<Document> d = getDocument();

  deque<std::shared_ptr<LazyField>> lazyValues = fields[fieldNum];
  std::deque<std::shared_ptr<IndexableField>> realValues = d->getFields(name);

  assert((realValues.size() <= lazyValues.size(),
          L"More lazy values then real values for field: " + name));

  for (int i = 0; i < lazyValues.size(); i++) {
    shared_ptr<LazyField> f = lazyValues[i];
    if (nullptr != f) {
      f->realValue = realValues[i];
    }
  }
}

LazyDocument::LazyField::LazyField(shared_ptr<LazyDocument> outerInstance,
                                   const wstring &name, int fieldNum)
    : outerInstance(outerInstance)
{
  this->name_ = name;
  this->fieldNum = fieldNum;
}

bool LazyDocument::LazyField::hasBeenLoaded() { return nullptr != realValue; }

shared_ptr<IndexableField> LazyDocument::LazyField::getRealValue()
{
  if (nullptr == realValue) {
    outerInstance->fetchRealValues(name_, fieldNum);
  }
  assert((hasBeenLoaded(), L"field value was not lazy loaded"));
  assert((realValue->name() == name(),
          L"realvalue name != name: " + realValue->name() + L" != " + name()));

  return realValue;
}

wstring LazyDocument::LazyField::name() { return name_; }

shared_ptr<BytesRef> LazyDocument::LazyField::binaryValue()
{
  return getRealValue()->binaryValue();
}

wstring LazyDocument::LazyField::stringValue()
{
  return getRealValue()->stringValue();
}

shared_ptr<Reader> LazyDocument::LazyField::readerValue()
{
  return getRealValue()->readerValue();
}

shared_ptr<Number> LazyDocument::LazyField::numericValue()
{
  return getRealValue()->numericValue();
}

shared_ptr<IndexableFieldType> LazyDocument::LazyField::fieldType()
{
  return getRealValue()->fieldType();
}

shared_ptr<TokenStream>
LazyDocument::LazyField::tokenStream(shared_ptr<Analyzer> analyzer,
                                     shared_ptr<TokenStream> reuse)
{
  return getRealValue()->tokenStream(analyzer, reuse);
}
} // namespace org::apache::lucene::document