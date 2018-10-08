using namespace std;

#include "TestPrefixCompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using Term = org::apache::lucene::index::Term;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.Entry; import
//    static
//    org.apache.lucene.search.suggest.document.TestSuggestField.assertSuggestions;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.iwcWithSuggestField;
//    import static org.hamcrest.core.IsEqual.equalTo;

TestPrefixCompletionQuery::NumericRangeBitsProducer::NumericRangeBitsProducer(
    const wstring &field, int64_t min, int64_t max)
    : field(field), min(min), max(max)
{
}

wstring TestPrefixCompletionQuery::NumericRangeBitsProducer::toString()
{
  return field + L"[" + to_wstring(min) + L".." + to_wstring(max) + L"]";
}

bool TestPrefixCompletionQuery::NumericRangeBitsProducer::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<NumericRangeBitsProducer> that =
      any_cast<std::shared_ptr<NumericRangeBitsProducer>>(obj);
  return field == that->field && min == that->min && max == that->max;
}

int TestPrefixCompletionQuery::NumericRangeBitsProducer::hashCode()
{
  return Objects::hash(getClass(), field, min, max);
}

shared_ptr<Bits> TestPrefixCompletionQuery::NumericRangeBitsProducer::getBits(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int maxDoc = context->reader()->maxDoc();
  shared_ptr<FixedBitSet> bits = make_shared<FixedBitSet>(maxDoc);
  shared_ptr<SortedNumericDocValues> *const values =
      DocValues::getSortedNumeric(context->reader(), field);
  int docID;
  while ((docID = values->nextDoc()) != NO_MORE_DOCS) {
    constexpr int count = values->docValueCount();
    for (int i = 0; i < count; ++i) {
      constexpr int64_t v = values->nextValue();
      if (v >= min && v <= max) {
        bits->set(docID);
        break;
      }
    }
  }
  return bits;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before() throws Exception
void TestPrefixCompletionQuery::before() 
{
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestPrefixCompletionQuery::after()  { delete dir; }

void TestPrefixCompletionQuery::testSimple() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abc", 3));
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abd", 4));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"The Foo Fighters", 2));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abcdd", 5));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"ab"));
  shared_ptr<TopSuggestDocs> lookupDocs =
      suggestIndexSearcher->suggest(query, 3, false);
  assertSuggestions(lookupDocs, make_shared<Entry>(L"abcdd", 5),
                    make_shared<Entry>(L"abd", 4),
                    make_shared<Entry>(L"abc", 3));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyPrefixQuery() throws Exception
void TestPrefixCompletionQuery::testEmptyPrefixQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion1", 1));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L""));

  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertEquals(0, suggest->scoreDocs.size());

  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testMostlyFilteredOutDocuments() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  int num = min(1000, atLeast(10));
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(L"suggest_field",
                                                  L"abc_" + to_wstring(i), i));
    document->push_back(
        make_shared<NumericDocValuesField>(L"filter_int_fld", i));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  int topScore = num / 2;
  shared_ptr<BitsProducer> filter =
      make_shared<NumericRangeBitsProducer>(L"filter_int_fld", 0, topScore);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"), filter);
  // if at most half of the top scoring documents have been filtered out
  // the search should be admissible for a single segment
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, num, false);
  assertTrue(suggest->totalHits >= 1);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertThat(suggest->scoreLookupDocs()[0]->key->toString(),
             equalTo(L"abc_" + to_wstring(topScore)));
  assertThat(suggest->scoreLookupDocs()[0]->score,
             equalTo(static_cast<float>(topScore)));

  filter = make_shared<NumericRangeBitsProducer>(L"filter_int_fld", 0, 0);
  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"), filter);
  // if more than half of the top scoring documents have been filtered out
  // search is not admissible, so # of suggestions requested is num instead of 1
  suggest = indexSearcher->suggest(query, num, false);
  assertSuggestions(suggest, make_shared<Entry>(L"abc_0", 0));

  filter = make_shared<NumericRangeBitsProducer>(L"filter_int_fld", num - 1,
                                                 num - 1);
  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"), filter);
  // if only lower scoring documents are filtered out
  // search is admissible
  suggest = indexSearcher->suggest(query, 1, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"abc_" + to_wstring(num - 1), num - 1));

  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testDocFiltering() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));

  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<NumericDocValuesField>(L"filter_int_fld", 9));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"apples", 3));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(
      make_shared<NumericDocValuesField>(L"filter_int_fld", 10));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"applle", 4));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<NumericDocValuesField>(L"filter_int_fld", 4));
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"apple", 5));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  // suggest without filter
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"app"));
  shared_ptr<TopSuggestDocs> suggest = indexSearcher->suggest(query, 3, false);
  assertSuggestions(suggest, make_shared<Entry>(L"apple", 5),
                    make_shared<Entry>(L"applle", 4),
                    make_shared<Entry>(L"apples", 3));

  // suggest with filter
  shared_ptr<BitsProducer> filter =
      make_shared<NumericRangeBitsProducer>(L"filter_int_fld", 5, 12);
  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"app"), filter);
  suggest = indexSearcher->suggest(query, 3, false);
  assertSuggestions(suggest, make_shared<Entry>(L"applle", 4),
                    make_shared<Entry>(L"apples", 3));

  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testAnalyzerDefaults() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(analyzer);
  const wstring field = getTestName();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(completionAnalyzer, field));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(field, L"foobar", 7));
  document->push_back(make_shared<SuggestField>(field, L"foo bar", 8));
  document->push_back(make_shared<SuggestField>(field, L"the fo", 9));
  document->push_back(make_shared<SuggestField>(field, L"the foo bar", 10));
  document->push_back(
      make_shared<SuggestField>(field, L"foo the bar", 11)); // middle stopword
  document->push_back(
      make_shared<SuggestField>(field, L"baz the", 12)); // trailing stopword

  iw->addDocument(document);

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer, make_shared<Term>(field, L"fo"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, 9, false); // matches all with "fo*"
  assertSuggestions(suggest, make_shared<Entry>(L"foo the bar", 11),
                    make_shared<Entry>(L"foo bar", 8),
                    make_shared<Entry>(L"foobar", 7));
  // with leading stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"the fo")); // becomes "_ fo*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(suggest, make_shared<Entry>(L"the foo bar", 10),
                    make_shared<Entry>(L"the fo", 9));
  // with middle stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"foo the bar")); // becomes "foo _ bar*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(suggest, make_shared<Entry>(L"foo the bar", 11));
  // no space
  query = make_shared<PrefixCompletionQuery>(completionAnalyzer,
                                             make_shared<Term>(field, L"foob"));
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(suggest, make_shared<Entry>(L"foobar", 7));
  // surrounding stopwords
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"the baz the")); // becomes "_ baz _"
  suggest = indexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest);
  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testAnalyzerWithoutSeparator() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  // note: when we don't preserve separators, the choice of preservePosInc is
  // irrelevant
  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(analyzer, false, random()->nextBoolean());
  const wstring field = getTestName();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(completionAnalyzer, field));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(field, L"foobar", 7));
  document->push_back(make_shared<SuggestField>(field, L"foo bar", 8));
  document->push_back(make_shared<SuggestField>(field, L"the fo", 9));
  document->push_back(make_shared<SuggestField>(field, L"the foo bar", 10));
  document->push_back(
      make_shared<SuggestField>(field, L"foo the bar", 11)); // middle stopword
  document->push_back(
      make_shared<SuggestField>(field, L"baz the", 12)); // trailing stopword

  iw->addDocument(document);

  // note we use the completionAnalyzer with the queries (instead of input
  // analyzer) because of non-default settings
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer, make_shared<Term>(field, L"fo"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, 9, false); // matches all with fo
  assertSuggestions(
      suggest, make_shared<Entry>(L"foo the bar", 11),
      make_shared<Entry>(L"the foo bar", 10), make_shared<Entry>(L"the fo", 9),
      make_shared<Entry>(L"foo bar", 8), make_shared<Entry>(L"foobar", 7));
  // with leading stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer, make_shared<Term>(field, L"the fo")); // becomes "fo*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(
      suggest, make_shared<Entry>(L"foo the bar", 11),
      make_shared<Entry>(L"the foo bar", 10), make_shared<Entry>(L"the fo", 9),
      make_shared<Entry>(L"foo bar", 8), make_shared<Entry>(L"foobar", 7));
  // with middle stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"foo the bar")); // becomes "foobar*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(suggest, make_shared<Entry>(L"foo the bar", 11),
                    make_shared<Entry>(L"the foo bar", 10),
                    make_shared<Entry>(L"foo bar", 8),
                    make_shared<Entry>(L"foobar", 7));
  // no space
  query = make_shared<PrefixCompletionQuery>(completionAnalyzer,
                                             make_shared<Term>(field, L"foob"));
  suggest = indexSearcher->suggest(query, 9,
                                   false); // no separators, thus match several
  assertSuggestions(suggest, make_shared<Entry>(L"foo the bar", 11),
                    make_shared<Entry>(L"the foo bar", 10),
                    make_shared<Entry>(L"foo bar", 8),
                    make_shared<Entry>(L"foobar", 7));
  // surrounding stopwords
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"the baz the")); // becomes "baz*"
  suggest = indexSearcher->suggest(
      query, 4, false); // stopwords in query get removed so we match
  assertSuggestions(suggest, make_shared<Entry>(L"baz the", 12));
  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testAnalyzerNoPreservePosInc() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(analyzer, true, false);
  const wstring field = getTestName();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(completionAnalyzer, field));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(field, L"foobar", 7));
  document->push_back(make_shared<SuggestField>(field, L"foo bar", 8));
  document->push_back(make_shared<SuggestField>(field, L"the fo", 9));
  document->push_back(make_shared<SuggestField>(field, L"the foo bar", 10));
  document->push_back(
      make_shared<SuggestField>(field, L"foo the bar", 11)); // middle stopword
  document->push_back(
      make_shared<SuggestField>(field, L"baz the", 12)); // trailing stopword

  iw->addDocument(document);

  // note we use the completionAnalyzer with the queries (instead of input
  // analyzer) because of non-default settings
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer, make_shared<Term>(field, L"fo"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, 9, false); // matches all with fo
  assertSuggestions(
      suggest, make_shared<Entry>(L"foo the bar", 11),
      make_shared<Entry>(L"the foo bar", 10), make_shared<Entry>(L"the fo", 9),
      make_shared<Entry>(L"foo bar", 8), make_shared<Entry>(L"foobar", 7));
  // with leading stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer, make_shared<Term>(field, L"the fo")); // becomes "fo*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(
      suggest, make_shared<Entry>(L"foo the bar", 11),
      make_shared<Entry>(L"the foo bar", 10), make_shared<Entry>(L"the fo", 9),
      make_shared<Entry>(L"foo bar", 8), make_shared<Entry>(L"foobar", 7));
  // with middle stopword
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"foo the bar")); // becomes "foo bar*"
  suggest = indexSearcher->suggest(query, 9, false);
  assertSuggestions(suggest, make_shared<Entry>(L"foo the bar", 11),
                    make_shared<Entry>(L"the foo bar", 10),
                    make_shared<Entry>(L"foo bar", 8)); // no foobar
  // no space
  query = make_shared<PrefixCompletionQuery>(completionAnalyzer,
                                             make_shared<Term>(field, L"foob"));
  suggest = indexSearcher->suggest(
      query, 4, false); // separators, thus only match "foobar"
  assertSuggestions(suggest, make_shared<Entry>(L"foobar", 7));
  // surrounding stopwords
  query = make_shared<PrefixCompletionQuery>(
      completionAnalyzer,
      make_shared<Term>(field, L"the baz the")); // becomes "baz*"
  suggest = indexSearcher->suggest(
      query, 4, false); // stopwords in query get removed so we match
  assertSuggestions(suggest, make_shared<Entry>(L"baz the", 12));
  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testGhostField() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, iwcWithSuggestField(analyzer, L"suggest_field", L"suggest_field2",
                               L"suggest_field3"));

  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"apples", 3));
  iw->addDocument(document);
  // need another document so whole segment isn't deleted
  iw->addDocument(make_shared<Document>());
  iw->commit();

  document = make_shared<Document>();
  document->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field2", L"apples", 3));
  iw->addDocument(document);
  iw->commit();

  iw->deleteDocuments({make_shared<Term>(L"id", L"0")});
  // first force merge is OK
  iw->forceMerge(1);

  // second force merge causes MultiFields to include "suggest_field" in its
  // iteration, yet a null Terms is returned (no documents have this field
  // anymore)
  iw->addDocument(make_shared<Document>());
  iw->forceMerge(1);

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"app"));
  assertEquals(0, indexSearcher->suggest(query, 3, false)->totalHits);

  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field2", L"app"));
  assertSuggestions(indexSearcher->suggest(query, 3, false),
                    make_shared<Entry>(L"apples", 3));

  reader->close();
  delete iw;
}

void TestPrefixCompletionQuery::testEmptyPrefixContextQuery() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"")));
  query->addContext(L"type", 1);

  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertEquals(0, suggest->scoreDocs.size());

  reader->close();
  delete iw;
}
} // namespace org::apache::lucene::search::suggest::document