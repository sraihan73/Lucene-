using namespace std;

#include "SearchWithSortTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;

SearchWithSortTask::SearchWithSortTask(shared_ptr<PerfRunData> runData)
    : ReadTask(runData)
{
}

void SearchWithSortTask::setParams(const wstring &sortField)
{
  ReadTask::setParams(sortField);
  std::deque<wstring> fields = sortField.split(L",");
  std::deque<std::shared_ptr<SortField>> sortFields(fields.size());
  int upto = 0;
  for (int i = 0; i < fields.size(); i++) {
    wstring field = fields[i];
    shared_ptr<SortField> sortField0;
    if (field == L"doc") {
      sortField0 = SortField::FIELD_DOC;
    } else if (field == L"score") {
      sortField0 = SortField::FIELD_SCORE;
    } else if (field == L"noscore") {
      doScore = false;
      continue;
    } else if (field == L"nomaxscore") {
      doMaxScore = false;
      continue;
    } else {
      int index = (int)field.rfind(L":");
      wstring fieldName;
      wstring typeString;
      if (index != -1) {
        fieldName = field.substr(0, index);
        typeString = field.substr(1 + index, field.length() - (1 + index));
      } else {
        throw runtime_error(
            L"You must specify the sort type ie page:int,subject:string");
      }
      sortField0 = make_shared<SortField>(
          fieldName,
          SortField::Type::valueOf(typeString.toUpperCase(Locale::ROOT)));
    }
    sortFields[upto++] = sortField0;
  }

  if (upto < sortFields.size()) {
    std::deque<std::shared_ptr<SortField>> newSortFields(upto);
    System::arraycopy(sortFields, 0, newSortFields, 0, upto);
    sortFields = newSortFields;
  }
  this->sort = make_shared<Sort>(sortFields);
}

bool SearchWithSortTask::supportsParams() { return true; }

shared_ptr<QueryMaker> SearchWithSortTask::getQueryMaker()
{
  return getRunData()->getQueryMaker(shared_from_this());
}

bool SearchWithSortTask::withRetrieve() { return false; }

bool SearchWithSortTask::withSearch() { return true; }

bool SearchWithSortTask::withTraverse() { return false; }

bool SearchWithSortTask::withWarm() { return false; }

bool SearchWithSortTask::withScore() { return doScore; }

bool SearchWithSortTask::withMaxScore() { return doMaxScore; }

shared_ptr<Sort> SearchWithSortTask::getSort()
{
  if (sort == nullptr) {
    throw make_shared<IllegalStateException>(L"No sort field was set");
  }
  return sort;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks