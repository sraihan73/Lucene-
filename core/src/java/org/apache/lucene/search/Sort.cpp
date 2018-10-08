using namespace std;

#include "Sort.h"

namespace org::apache::lucene::search
{

const shared_ptr<Sort> Sort::RELEVANCE = make_shared<Sort>();
const shared_ptr<Sort> Sort::INDEXORDER =
    make_shared<Sort>(SortField::FIELD_DOC);

Sort::Sort() : Sort(SortField::FIELD_SCORE) {}

Sort::Sort(shared_ptr<SortField> field) { setSort({field}); }

Sort::Sort(deque<SortField> &fields) { setSort(fields); }

void Sort::setSort(shared_ptr<SortField> field)
{
  this->fields = std::deque<std::shared_ptr<SortField>>{field};
}

void Sort::setSort(deque<SortField> &fields)
{
  if (fields.empty()) {
    throw invalid_argument(L"There must be at least 1 sort field");
  }
  this->fields = fields;
}

std::deque<std::shared_ptr<SortField>> Sort::getSort() { return fields; }

shared_ptr<Sort>
Sort::rewrite(shared_ptr<IndexSearcher> searcher) 
{
  bool changed = false;

  std::deque<std::shared_ptr<SortField>> rewrittenSortFields(fields.size());
  for (int i = 0; i < fields.size(); i++) {
    rewrittenSortFields[i] = fields[i]->rewrite(searcher);
    if (fields[i] != rewrittenSortFields[i]) {
      changed = true;
    }
  }

  return (changed) ? make_shared<Sort>(rewrittenSortFields)
                   : shared_from_this();
}

wstring Sort::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();

  for (int i = 0; i < fields.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(fields[i]->toString());
    if ((i + 1) < fields.size()) {
      buffer->append(L',');
    }
  }

  return buffer->toString();
}

bool Sort::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<Sort>(o) != nullptr)) {
    return false;
  }
  shared_ptr<Sort> *const other = any_cast<std::shared_ptr<Sort>>(o);
  return Arrays::equals(this->fields, other->fields);
}

int Sort::hashCode() { return 0x45aaf665 + Arrays::hashCode(fields); }

bool Sort::needsScores()
{
  for (auto sortField : fields) {
    if (sortField->needsScores()) {
      return true;
    }
  }
  return false;
}
} // namespace org::apache::lucene::search