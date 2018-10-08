using namespace std;

#include "TestSameTokenSamePosition.h"

namespace org::apache::lucene::index
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSameTokenSamePosition::test() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"eng", make_shared<BugReproTokenStream>()));
  riw->addDocument(doc);
  delete riw;
  delete dir;
}

void TestSameTokenSamePosition::testMoreDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<TextField>(L"eng", make_shared<BugReproTokenStream>()));
    riw->addDocument(doc);
  }
  delete riw;
  delete dir;
}

bool BugReproTokenStream::incrementToken()
{
  if (nextTokenIndex < TOKEN_COUNT) {
    termAtt->setEmpty()->append(terms[nextTokenIndex]);
    offsetAtt->setOffset(starts[nextTokenIndex], ends[nextTokenIndex]);
    posIncAtt->setPositionIncrement(incs[nextTokenIndex]);
    nextTokenIndex++;
    return true;
  } else {
    return false;
  }
}

void BugReproTokenStream::reset() 
{
  TokenStream::reset();
  this->nextTokenIndex = 0;
}
} // namespace org::apache::lucene::index