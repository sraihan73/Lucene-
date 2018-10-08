using namespace std;

#include "FieldDoc.h"

namespace org::apache::lucene::search
{

FieldDoc::FieldDoc(int doc, float score) : ScoreDoc(doc, score) {}

FieldDoc::FieldDoc(int doc, float score, std::deque<any> &fields)
    : ScoreDoc(doc, score)
{
  this->fields = fields;
}

FieldDoc::FieldDoc(int doc, float score, std::deque<any> &fields,
                   int shardIndex)
    : ScoreDoc(doc, score, shardIndex)
{
  this->fields = fields;
}

wstring FieldDoc::toString()
{
  // super.toString returns the doc and score information, so just add the
  // fields information
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(ScoreDoc::toString());
  sb->append(L" fields=");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(Arrays->toString(fields));
  return sb->toString();
}
} // namespace org::apache::lucene::search