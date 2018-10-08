using namespace std;

#include "DocumentStoredFieldVisitor.h"

namespace org::apache::lucene::document
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;

DocumentStoredFieldVisitor::DocumentStoredFieldVisitor(
    shared_ptr<Set<wstring>> fieldsToAdd)
    : fieldsToAdd(fieldsToAdd)
{
}

DocumentStoredFieldVisitor::DocumentStoredFieldVisitor(deque<wstring> &fields)
    : fieldsToAdd(unordered_set<>(fields->length))
{
  for (wstring field : fields) {
    fieldsToAdd->add(field);
  }
}

DocumentStoredFieldVisitor::DocumentStoredFieldVisitor()
    : fieldsToAdd(this->fieldsToAdd.reset())
{
}

void DocumentStoredFieldVisitor::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  doc->push_back(make_shared<StoredField>(fieldInfo->name, value));
}

void DocumentStoredFieldVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  shared_ptr<FieldType> *const ft =
      make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(fieldInfo->hasVectors());
  ft->setOmitNorms(fieldInfo->omitsNorms());
  ft->setIndexOptions(fieldInfo->getIndexOptions());
  doc->push_back(make_shared<StoredField>(
      fieldInfo->name, wstring(value, StandardCharsets::UTF_8), ft));
}

void DocumentStoredFieldVisitor::intField(shared_ptr<FieldInfo> fieldInfo,
                                          int value)
{
  doc->push_back(make_shared<StoredField>(fieldInfo->name, value));
}

void DocumentStoredFieldVisitor::longField(shared_ptr<FieldInfo> fieldInfo,
                                           int64_t value)
{
  doc->push_back(make_shared<StoredField>(fieldInfo->name, value));
}

void DocumentStoredFieldVisitor::floatField(shared_ptr<FieldInfo> fieldInfo,
                                            float value)
{
  doc->push_back(make_shared<StoredField>(fieldInfo->name, value));
}

void DocumentStoredFieldVisitor::doubleField(shared_ptr<FieldInfo> fieldInfo,
                                             double value)
{
  doc->push_back(make_shared<StoredField>(fieldInfo->name, value));
}

Status DocumentStoredFieldVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  return fieldsToAdd == nullptr || fieldsToAdd->contains(fieldInfo->name)
             ? Status::YES
             : Status::NO;
}

shared_ptr<Document> DocumentStoredFieldVisitor::getDocument() { return doc; }
} // namespace org::apache::lucene::document