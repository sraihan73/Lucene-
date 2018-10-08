using namespace std;

#include "Document.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using BytesRef = org::apache::lucene::util::BytesRef;

Document::Document() {}

shared_ptr<Iterator<std::shared_ptr<IndexableField>>> Document::iterator()
{
  return fields.begin();
}

void Document::add(shared_ptr<IndexableField> field)
{
  fields.push_back(field);
}

void Document::removeField(const wstring &name)
{
  deque<std::shared_ptr<IndexableField>>::const_iterator it = fields.begin();
  while (it != fields.end()) {
    shared_ptr<IndexableField> field = *it;
    if (field->name() == name) {
      (*it)->remove();
      return;
    }
    it++;
  }
}

void Document::removeFields(const wstring &name)
{
  deque<std::shared_ptr<IndexableField>>::const_iterator it = fields.begin();
  while (it != fields.end()) {
    shared_ptr<IndexableField> field = *it;
    if (field->name() == name) {
      (*it)->remove();
    }
    it++;
  }
}

std::deque<std::shared_ptr<BytesRef>>
Document::getBinaryValues(const wstring &name)
{
  const deque<std::shared_ptr<BytesRef>> result =
      deque<std::shared_ptr<BytesRef>>();
  for (auto field : fields) {
    if (field->name() == name) {
      shared_ptr<BytesRef> *const bytes = field->binaryValue();
      if (bytes != nullptr) {
        result.push_back(bytes);
      }
    }
  }

  return result.toArray(std::deque<std::shared_ptr<BytesRef>>(result.size()));
}

shared_ptr<BytesRef> Document::getBinaryValue(const wstring &name)
{
  for (auto field : fields) {
    if (field->name() == name) {
      shared_ptr<BytesRef> *const bytes = field->binaryValue();
      if (bytes != nullptr) {
        return bytes;
      }
    }
  }
  return nullptr;
}

shared_ptr<IndexableField> Document::getField(const wstring &name)
{
  for (auto field : fields) {
    if (field->name() == name) {
      return field;
    }
  }
  return nullptr;
}

std::deque<std::shared_ptr<IndexableField>>
Document::getFields(const wstring &name)
{
  deque<std::shared_ptr<IndexableField>> result =
      deque<std::shared_ptr<IndexableField>>();
  for (auto field : fields) {
    if (field->name() == name) {
      result.push_back(field);
    }
  }

  return result.toArray(
      std::deque<std::shared_ptr<IndexableField>>(result.size()));
}

deque<std::shared_ptr<IndexableField>> Document::getFields()
{
  return Collections::unmodifiableList(fields);
}

std::deque<wstring> const Document::NO_STRINGS = std::deque<wstring>(0);

std::deque<wstring> Document::getValues(const wstring &name)
{
  deque<wstring> result = deque<wstring>();
  for (auto field : fields) {
    if (field->name() == name && field->stringValue() != L"") {
      result.push_back(field->stringValue());
    }
  }

  if (result.empty()) {
    return NO_STRINGS;
  }

  return result.toArray(std::deque<wstring>(result.size()));
}

wstring Document::get(const wstring &name)
{
  for (auto field : fields) {
    if (field->name() == name && field->stringValue() != L"") {
      return field->stringValue();
    }
  }
  return L"";
}

wstring Document::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"Document<");
  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<IndexableField> field = fields[i];
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(field->toString());
    if (i != fields.size() - 1) {
      buffer->append(L" ");
    }
  }
  buffer->append(L">");
  return buffer->toString();
}

void Document::clear() { fields.clear(); }
} // namespace org::apache::lucene::document