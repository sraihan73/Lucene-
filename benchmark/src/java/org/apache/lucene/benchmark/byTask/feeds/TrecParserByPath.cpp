using namespace std;

#include "TrecParserByPath.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

shared_ptr<DocData>
TrecParserByPath::parse(shared_ptr<DocData> docData, const wstring &name,
                        shared_ptr<TrecContentSource> trecSrc,
                        shared_ptr<StringBuilder> docBuf,
                        ParsePathType pathType) 
{
  return pathType2parser[pathType]->parse(docData, name, trecSrc, docBuf,
                                          pathType);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds