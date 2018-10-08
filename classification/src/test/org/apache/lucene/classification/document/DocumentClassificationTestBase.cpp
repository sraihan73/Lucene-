using namespace std;

#include "DocumentClassificationTestBase.h"

namespace org::apache::lucene::classification::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using EnglishAnalyzer = org::apache::lucene::analysis::en::EnglishAnalyzer;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using ClassificationTestBase =
    org::apache::lucene::classification::ClassificationTestBase;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Before;
} // namespace org::apache::lucene::classification::document