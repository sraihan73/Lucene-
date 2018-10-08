using namespace std;

#include "TestFilterLeafReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestFilterLeafReader::TestReader::TestTerms::TestTerms(shared_ptr<Terms> in_)
    : FilterTerms(in_)
{
}

shared_ptr<TermsEnum>
TestFilterLeafReader::TestReader::TestTerms::iterator() 
{
  return make_shared<TestTermsEnum>(FilterTerms::begin());
}

TestFilterLeafReader::TestReader::TestTermsEnum::TestTermsEnum(
    shared_ptr<TermsEnum> in_)
    : FilterTermsEnum(in_)
{
}

shared_ptr<BytesRef>
TestFilterLeafReader::TestReader::TestTermsEnum::next() 
{
  shared_ptr<BytesRef> text;
  while ((text = in_->next()) != nullptr) {
    if (text->utf8ToString().find(L'e') != wstring::npos) {
      return text;
    }
  }
  return nullptr;
}

shared_ptr<PostingsEnum>
TestFilterLeafReader::TestReader::TestTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  return make_shared<TestPositions>(FilterTermsEnum::postings(
      reuse == nullptr
          ? nullptr
          : (std::static_pointer_cast<FilterPostingsEnum>(reuse))->in_,
      flags));
}

TestFilterLeafReader::TestReader::TestPositions::TestPositions(
    shared_ptr<PostingsEnum> in_)
    : FilterPostingsEnum(in_)
{
}

int TestFilterLeafReader::TestReader::TestPositions::nextDoc() throw(
    IOException)
{
  int doc;
  while ((doc = in_->nextDoc()) != NO_MORE_DOCS) {
    if ((doc % 2) == 1) {
      return doc;
    }
  }
  return NO_MORE_DOCS;
}

TestFilterLeafReader::TestReader::TestReader(
    shared_ptr<LeafReader> reader) 
    : FilterLeafReader(reader)
{
}

shared_ptr<Terms>
TestFilterLeafReader::TestReader::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  return terms == nullptr ? nullptr : make_shared<TestTerms>(terms);
}

shared_ptr<CacheHelper> TestFilterLeafReader::TestReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> TestFilterLeafReader::TestReader::getReaderCacheHelper()
{
  return nullptr;
}

void TestFilterLeafReader::testFilterIndexReader() 
{
  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"default", L"one two", Field::Store::YES));
  writer->addDocument(d1);

  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"default", L"one three", Field::Store::YES));
  writer->addDocument(d2);

  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"default", L"two four", Field::Store::YES));
  writer->addDocument(d3);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<Directory> target = newDirectory();

  // We mess with the postings so this can fail:
  (std::static_pointer_cast<BaseDirectoryWrapper>(target))
      ->setCrossCheckTermVectorsOnClose(false);

  writer = make_shared<IndexWriter>(
      target, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (LeafReader reader = new
  // TestReader(getOnlyLeafReader(DirectoryReader.open(directory))))
  {
    LeafReader reader =
        TestReader(getOnlyLeafReader(DirectoryReader::open(directory)));
    writer->addIndexes({SlowCodecReaderWrapper::wrap(reader)});
  }
  delete writer;
  IndexReader reader = DirectoryReader::open(target);

  shared_ptr<TermsEnum> terms =
      MultiFields::getTerms(reader, L"default")->begin();
  while (terms->next() != nullptr) {
    assertTrue(terms->term()->utf8ToString().find(L'e') != wstring::npos);
  }

  assertEquals(TermsEnum::SeekStatus::FOUND,
               terms->seekCeil(make_shared<BytesRef>(L"one")));

  shared_ptr<PostingsEnum> positions =
      terms->postings(nullptr, PostingsEnum::ALL);
  while (positions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    assertTrue((positions->docID() % 2) == 1);
  }

  delete reader;
  delete directory;
  delete target;
}

void TestFilterLeafReader::checkOverrideMethods(type_info clazz) throw(
    NoSuchMethodException, SecurityException)
{
  constexpr type_info superClazz = clazz.getSuperclass();
  for (shared_ptr<Method> m : superClazz.getMethods()) {
    constexpr int mods = m->getModifiers();
    if (Modifier::isStatic(mods) || Modifier::isAbstract(mods) ||
        Modifier::isFinal(mods) || m->isSynthetic() ||
        m->getName().equals(L"attributes") ||
        m->getName().equals(L"getStats")) {
      continue;
    }
    // The point of these checks is to ensure that methods that have a default
    // impl through other methods are not overridden. This makes the number of
    // methods to override to have a working impl minimal and prevents from some
    // traps: for example, think about having getCoreCacheKey delegate to the
    // filtered impl by default
    shared_ptr<Method> *const subM =
        clazz.getMethod(m->getName(), m->getParameterTypes());
    if (subM->getDeclaringClass() == clazz &&
        m->getDeclaringClass() != any::typeid &&
        m->getDeclaringClass() != subM->getDeclaringClass()) {
      fail(clazz + L" overrides " + m + L" although it has a default impl");
    }
  }
}

void TestFilterLeafReader::testOverrideMethods() 
{
  checkOverrideMethods(FilterLeafReader::typeid);
  checkOverrideMethods(FilterLeafReader::FilterFields::typeid);
  checkOverrideMethods(FilterLeafReader::FilterTerms::typeid);
  checkOverrideMethods(FilterLeafReader::FilterTermsEnum::typeid);
  checkOverrideMethods(FilterLeafReader::FilterPostingsEnum::typeid);
}

void TestFilterLeafReader::testUnwrap() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> dr = w->getReader();
  shared_ptr<LeafReader> r = dr->leaves()->get(0).reader();
  shared_ptr<FilterLeafReader> r2 =
      make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(), r);
  assertEquals(r, r2->getDelegate());
  assertEquals(r, FilterLeafReader::unwrap(r2));
  delete w;
  dr->close();
  delete dir;
}

TestFilterLeafReader::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<TestFilterLeafReader> outerInstance,
        shared_ptr<org::apache::lucene::index::LeafReader> r)
    : FilterLeafReader(r)
{
  this->outerInstance = outerInstance;
}

shared_ptr<CacheHelper>
TestFilterLeafReader::FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return in_::getCoreCacheHelper();
}

shared_ptr<CacheHelper> TestFilterLeafReader::
    FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return in_::getReaderCacheHelper();
}
} // namespace org::apache::lucene::index