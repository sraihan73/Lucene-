using namespace std;

#include "IndexWriterMaxDocsChanger.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

IndexWriterMaxDocsChanger::IndexWriterMaxDocsChanger() {}

void IndexWriterMaxDocsChanger::setMaxDocs(int limit)
{
  IndexWriter::setMaxDocs(limit);
}

void IndexWriterMaxDocsChanger::restoreMaxDocs()
{
  IndexWriter::setMaxDocs(IndexWriter::MAX_DOCS);
}
} // namespace org::apache::lucene::index