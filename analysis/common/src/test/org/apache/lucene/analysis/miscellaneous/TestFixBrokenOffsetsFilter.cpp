using namespace std;

#include "TestFixBrokenOffsetsFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/FixBrokenOffsetsFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;

void TestFixBrokenOffsetsFilter::testBogusTermVectors() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  shared_ptr<Field> field = make_shared<Field>(L"foo", L"", ft);
  field->setTokenStream(make_shared<FixBrokenOffsetsFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"bar", 5, 10),
                                     make_shared<Token>(L"bar", 1, 4))));
  doc->push_back(field);
  iw->addDocument(doc);
  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::analysis::miscellaneous