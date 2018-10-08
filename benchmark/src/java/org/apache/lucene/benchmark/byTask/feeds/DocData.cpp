using namespace std;

#include "DocData.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/DateTools.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using DateTools = org::apache::lucene::document::DateTools;

void DocData::clear()
{
  name = L"";
  body = L"";
  title = L"";
  date = L"";
  props.reset();
  id = -1;
}

wstring DocData::getBody() { return body; }

wstring DocData::getDate() { return date; }

wstring DocData::getName() { return name; }

int DocData::getID() { return id; }

shared_ptr<Properties> DocData::getProps() { return props; }

wstring DocData::getTitle() { return title; }

void DocData::setBody(const wstring &body) { this->body = body; }

void DocData::setDate(Date date)
{
  if (date != nullptr) {
    setDate(DateTools::dateToString(date, DateTools::Resolution::SECOND));
  } else {
    this->date = L"";
  }
}

void DocData::setDate(const wstring &date) { this->date = date; }

void DocData::setName(const wstring &name) { this->name = name; }

void DocData::setID(int id) { this->id = id; }

void DocData::setProps(shared_ptr<Properties> props) { this->props = props; }

void DocData::setTitle(const wstring &title) { this->title = title; }
} // namespace org::apache::lucene::benchmark::byTask::feeds