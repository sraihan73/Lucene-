using namespace std;

#include "TestTermQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTermQuery::testEquals() 
{
  QueryUtils::checkEqual(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")));
  QueryUtils::checkUnequal(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")));
  QueryUtils::checkEqual(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<TermQuery>(
          make_shared<Term>(L"foo", L"bar"),
          TermContext::build((make_shared<MultiReader>())->getContext(),
                             make_shared<Term>(L"foo", L"bar"))));
}

void TestTermQuery::testCreateWeightDoesNotSeekIfScoresAreNotNeeded() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setMergePolicy(NoMergePolicy::INSTANCE));
  // segment that contains the term
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  w->addDocument(doc);
  w->getReader()->close();
  // segment that does not contain the term
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"baz", Store::NO));
  w->addDocument(doc);
  w->getReader()->close();
  // segment that does not contain the field
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<FilterDirectoryReader> noSeekReader =
      make_shared<NoSeekDirectoryReader>(reader);
  shared_ptr<IndexSearcher> noSeekSearcher =
      make_shared<IndexSearcher>(noSeekReader);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<AssertionError> e = expectThrows(AssertionError::typeid, [&]() {
    noSeekSearcher->createWeight(noSeekSearcher->rewrite(query), true, 1);
  });
  assertEquals(L"no seek", e->getMessage());

  noSeekSearcher->createWeight(noSeekSearcher->rewrite(query), false,
                               1); // no exception
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  // use a collector rather than searcher.count() which would just read the
  // doc freq instead of creating a scorer
  shared_ptr<TotalHitCountCollector> collector =
      make_shared<TotalHitCountCollector>();
  searcher->search(query, collector);
  assertEquals(1, collector->getTotalHits());
  shared_ptr<TermQuery> queryWithContext = make_shared<TermQuery>(
      make_shared<Term>(L"foo", L"bar"),
      TermContext::build(reader->getContext(),
                         make_shared<Term>(L"foo", L"bar")));
  collector = make_shared<TotalHitCountCollector>();
  searcher->search(queryWithContext, collector);
  assertEquals(1, collector->getTotalHits());

  IOUtils::close({reader, w, dir});
}

TestTermQuery::NoSeekDirectoryReader::NoSeekDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  FilterDirectoryReader(in_,
                        make_shared<SubReaderWrapperAnonymousInnerClass>());
}

TestTermQuery::NoSeekDirectoryReader::SubReaderWrapperAnonymousInnerClass::
    SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader>
TestTermQuery::NoSeekDirectoryReader::SubReaderWrapperAnonymousInnerClass::wrap(
    shared_ptr<LeafReader> reader)
{
  return make_shared<NoSeekLeafReader>(reader);
}

shared_ptr<DirectoryReader>
TestTermQuery::NoSeekDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<NoSeekDirectoryReader>(in_);
}

shared_ptr<CacheHelper>
TestTermQuery::NoSeekDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

TestTermQuery::NoSeekLeafReader::NoSeekLeafReader(shared_ptr<LeafReader> in_)
    : org::apache::lucene::index::FilterLeafReader(in_)
{
}

shared_ptr<Terms>
TestTermQuery::NoSeekLeafReader::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  return terms == nullptr ? nullptr
                          : make_shared<FilterTermsAnonymousInnerClass>(
                                shared_from_this(), terms);
}

TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsAnonymousInnerClass(shared_ptr<NoSeekLeafReader> outerInstance,
                                   shared_ptr<Terms> terms)
    : FilterTerms(terms)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TermsEnum> TestTermQuery::NoSeekLeafReader::
    FilterTermsAnonymousInnerClass::iterator() 
{
  return make_shared<FilterTermsEnumAnonymousInnerClass>(
      shared_from_this(), outerInstance->super->begin());
}

TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsEnumAnonymousInnerClass::FilterTermsEnumAnonymousInnerClass(
        shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterTermsEnum(iterator)
{
  this->outerInstance = outerInstance;
}

SeekStatus TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsEnumAnonymousInnerClass::seekCeil(
        shared_ptr<BytesRef> text) 
{
  throw make_shared<AssertionError>(L"no seek");
}

void TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsEnumAnonymousInnerClass::seekExact(
        shared_ptr<BytesRef> term,
        shared_ptr<TermState> state) 
{
  throw make_shared<AssertionError>(L"no seek");
}

bool TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsEnumAnonymousInnerClass::seekExact(
        shared_ptr<BytesRef> text) 
{
  throw make_shared<AssertionError>(L"no seek");
}

void TestTermQuery::NoSeekLeafReader::FilterTermsAnonymousInnerClass::
    FilterTermsEnumAnonymousInnerClass::seekExact(int64_t ord) throw(
        IOException)
{
  throw make_shared<AssertionError>(L"no seek");
}

shared_ptr<CacheHelper> TestTermQuery::NoSeekLeafReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper> TestTermQuery::NoSeekLeafReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}
} // namespace org::apache::lucene::search