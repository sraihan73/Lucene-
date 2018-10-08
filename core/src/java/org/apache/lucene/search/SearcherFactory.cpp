using namespace std;

#include "SearcherFactory.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Similarity = org::apache::lucene::search::similarities::Similarity;

shared_ptr<IndexSearcher> SearcherFactory::newSearcher(
    shared_ptr<IndexReader> reader,
    shared_ptr<IndexReader> previousReader) 
{
  return make_shared<IndexSearcher>(reader);
}
} // namespace org::apache::lucene::search