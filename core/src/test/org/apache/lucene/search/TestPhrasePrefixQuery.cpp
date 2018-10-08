using namespace std;

#include "TestPhrasePrefixQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPhrasePrefixQuery::testPhrasePrefix() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  shared_ptr<Document> doc1 = make_shared<Document>();
  shared_ptr<Document> doc2 = make_shared<Document>();
  shared_ptr<Document> doc3 = make_shared<Document>();
  shared_ptr<Document> doc4 = make_shared<Document>();
  shared_ptr<Document> doc5 = make_shared<Document>();
  doc1->push_back(newTextField(L"body", L"blueberry pie", Field::Store::YES));
  doc2->push_back(
      newTextField(L"body", L"blueberry strudel", Field::Store::YES));
  doc3->push_back(newTextField(L"body", L"blueberry pizza", Field::Store::YES));
  doc4->push_back(
      newTextField(L"body", L"blueberry chewing gum", Field::Store::YES));
  doc5->push_back(
      newTextField(L"body", L"piccadilly circus", Field::Store::YES));
  writer->addDocument(doc1);
  writer->addDocument(doc2);
  writer->addDocument(doc3);
  writer->addDocument(doc4);
  writer->addDocument(doc5);
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // PhrasePrefixQuery query1 = new PhrasePrefixQuery();
  shared_ptr<MultiPhraseQuery::Builder> query1builder =
      make_shared<MultiPhraseQuery::Builder>();
  // PhrasePrefixQuery query2 = new PhrasePrefixQuery();
  shared_ptr<MultiPhraseQuery::Builder> query2builder =
      make_shared<MultiPhraseQuery::Builder>();
  query1builder->add(make_shared<Term>(L"body", L"blueberry"));
  query2builder->add(make_shared<Term>(L"body", L"strawberry"));

  deque<std::shared_ptr<Term>> termsWithPrefix = deque<std::shared_ptr<Term>>();

  // this TermEnum gives "piccadilly", "pie" and "pizza".
  wstring prefix = L"pi";
  shared_ptr<TermsEnum> te = MultiFields::getTerms(reader, L"body")->begin();
  te->seekCeil(make_shared<BytesRef>(prefix));
  do {
    wstring s = te->term()->utf8ToString();
    if (StringHelper::startsWith(s, prefix)) {
      termsWithPrefix.push_back(make_shared<Term>(L"body", s));
    } else {
      break;
    }
  } while (te->next() != nullptr);

  query1builder->add(
      termsWithPrefix.toArray(std::deque<std::shared_ptr<Term>>(0)));
  query2builder->add(
      termsWithPrefix.toArray(std::deque<std::shared_ptr<Term>>(0)));

  std::deque<std::shared_ptr<ScoreDoc>> result;
  result = searcher->search(query1builder->build(), 1000)->scoreDocs;
  assertEquals(2, result.size());

  result = searcher->search(query2builder->build(), 1000)->scoreDocs;
  assertEquals(0, result.size());
  delete reader;
  delete indexStore;
}
} // namespace org::apache::lucene::search