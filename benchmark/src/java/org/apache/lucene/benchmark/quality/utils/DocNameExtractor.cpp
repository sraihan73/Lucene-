using namespace std;

#include "DocNameExtractor.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"

namespace org::apache::lucene::benchmark::quality::utils
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

DocNameExtractor::DocNameExtractor(const wstring &docNameField)
    : docNameField(docNameField)
{
}

wstring DocNameExtractor::docName(shared_ptr<IndexSearcher> searcher,
                                  int docid) 
{
  const deque<wstring> name = deque<wstring>();
  searcher->getIndexReader()->document(
      docid, make_shared<StoredFieldVisitorAnonymousInnerClass>(
                 shared_from_this(), name));
  if (name.size() != 0) {
    return name[0];
  } else {
    return L"";
  }
}

DocNameExtractor::StoredFieldVisitorAnonymousInnerClass::
    StoredFieldVisitorAnonymousInnerClass(
        shared_ptr<DocNameExtractor> outerInstance, deque<wstring> &name)
{
  this->outerInstance = outerInstance;
  this->name = name;
}

void DocNameExtractor::StoredFieldVisitorAnonymousInnerClass::stringField(
    shared_ptr<FieldInfo> fieldInfo, std::deque<char> &bytes)
{
  wstring value = wstring(bytes, StandardCharsets::UTF_8);
  name.push_back(value);
}

Status DocNameExtractor::StoredFieldVisitorAnonymousInnerClass::needsField(
    shared_ptr<FieldInfo> fieldInfo)
{
  if (!name.empty()) {
    return Status::STOP;
  } else if (fieldInfo->name == outerInstance->docNameField) {
    return Status::YES;
  } else {
    return Status::NO;
  }
}
} // namespace org::apache::lucene::benchmark::quality::utils