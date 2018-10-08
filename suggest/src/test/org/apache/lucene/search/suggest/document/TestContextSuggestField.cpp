using namespace std;

#include "TestContextSuggestField.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
//    import static
//    org.apache.lucene.analysis.BaseTokenStreamTestCase.assertTokenStreamContents;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.Entry; import
//    static
//    org.apache.lucene.search.suggest.document.TestSuggestField.assertSuggestions;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.iwcWithSuggestField;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before() throws Exception
void TestContextSuggestField::before() 
{
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestContextSuggestField::after()  { delete dir; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptySuggestion() throws Exception
void TestContextSuggestField::testEmptySuggestion() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ContextSuggestField>(L"suggest_field", L"", 1, L"type1");
  });
  assertTrue(expected.what()->contains(L"value"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReservedChars() throws Exception
void TestContextSuggestField::testReservedChars() 
{
  shared_ptr<CharsRefBuilder> charsRefBuilder = make_shared<CharsRefBuilder>();
  charsRefBuilder->append(L"sugg");
  charsRefBuilder->setCharAt(
      2, static_cast<wchar_t>(ContextSuggestField::CONTEXT_SEPARATOR));

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Document> document = make_shared<Document>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter iw
  // = new org.apache.lucene.index.RandomIndexWriter(random(), dir,
  // iwcWithSuggestField(analyzer, "name")))
  {
    org::apache::lucene::index::RandomIndexWriter iw =
        org::apache::lucene::index::RandomIndexWriter(
            random(), dir, iwcWithSuggestField(analyzer, L"name"));
    // exception should be thrown for context value containing CONTEXT_SEPARATOR
    invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
      document->add(make_shared<ContextSuggestField>(
          L"name", L"sugg", 1, charsRefBuilder->toString()));
      iw->addDocument(document);
      iw->commit();
    });
    assertTrue(expected.what()->contains(L"[0x1d]"));
  }
  document->clear();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter iw
  // = new org.apache.lucene.index.RandomIndexWriter(random(), dir,
  // iwcWithSuggestField(analyzer, "name")))
  {
    org::apache::lucene::index::RandomIndexWriter iw =
        org::apache::lucene::index::RandomIndexWriter(
            random(), dir, iwcWithSuggestField(analyzer, L"name"));
    // exception should be thrown for context value containing CONTEXT_SEPARATOR
    invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
      document->add(make_shared<ContextSuggestField>(
          L"name", charsRefBuilder->toString(), 1, L"sugg"));
      iw->addDocument(document);
      iw->commit();
    });
    assertTrue(expected.what()->contains(L"[0x1d]"));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream() throws Exception
void TestContextSuggestField::testTokenStream() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<ContextSuggestField> field = make_shared<ContextSuggestField>(
      L"field", L"input", 1, L"context1", L"context2");
  shared_ptr<BytesRef> surfaceForm = make_shared<BytesRef>(L"input");
  shared_ptr<ByteArrayOutputStream> byteArrayOutputStream =
      make_shared<ByteArrayOutputStream>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.store.OutputStreamDataOutput output = new
  // org.apache.lucene.store.OutputStreamDataOutput(byteArrayOutputStream))
  {
    org::apache::lucene::store::OutputStreamDataOutput output =
        org::apache::lucene::store::OutputStreamDataOutput(
            byteArrayOutputStream);
    output->writeVInt(surfaceForm->length);
    output->writeBytes(surfaceForm->bytes, surfaceForm->offset,
                       surfaceForm->length);
    output->writeVInt(1 + 1);
    output->writeByte(ContextSuggestField::TYPE);
  }
  shared_ptr<BytesRef> payload =
      make_shared<BytesRef>(byteArrayOutputStream->toByteArray());
  std::deque<wstring> expectedOutputs(2);
  shared_ptr<CharsRefBuilder> builder = make_shared<CharsRefBuilder>();
  builder->append(L"context1");
  builder->append(
      (static_cast<wchar_t>(ContextSuggestField::CONTEXT_SEPARATOR)));
  builder->append(static_cast<wchar_t>(ConcatenateGraphFilter::SEP_LABEL));
  builder->append(L"input");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  expectedOutputs[0] = builder->toCharsRef()->toString();
  builder->clear();
  builder->append(L"context2");
  builder->append(
      (static_cast<wchar_t>(ContextSuggestField::CONTEXT_SEPARATOR)));
  builder->append(static_cast<wchar_t>(ConcatenateGraphFilter::SEP_LABEL));
  builder->append(L"input");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  expectedOutputs[1] = builder->toCharsRef()->toString();
  shared_ptr<TokenStream> stream =
      make_shared<TestSuggestField::PayloadAttrToTypeAttrFilter>(
          field->tokenStream(analyzer, nullptr));
  assertTokenStreamContents(
      stream, expectedOutputs, nullptr, nullptr,
      std::deque<wstring>{payload->utf8ToString(), payload->utf8ToString()},
      std::deque<int>{1, 0}, nullptr, nullptr);

  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(analyzer);
  stream = make_shared<TestSuggestField::PayloadAttrToTypeAttrFilter>(
      field->tokenStream(completionAnalyzer, nullptr));
  assertTokenStreamContents(
      stream, expectedOutputs, nullptr, nullptr,
      std::deque<wstring>{payload->utf8ToString(), payload->utf8ToString()},
      std::deque<int>{1, 0}, nullptr, nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMixedSuggestFields() throws Exception
void TestContextSuggestField::testMixedSuggestFields() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion1", 4));
  document->push_back(
      make_shared<ContextSuggestField>(L"suggest_field", L"suggestion2", 3));

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter iw
  // = new org.apache.lucene.index.RandomIndexWriter(random(), dir,
  // iwcWithSuggestField(analyzer, "suggest_field")))
  {
    org::apache::lucene::index::RandomIndexWriter iw =
        org::apache::lucene::index::RandomIndexWriter(
            random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
    // mixing suggest field types for same field name should error out
    invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
      iw->addDocument(document);
      iw->commit();
    });
    assertTrue(expected.what()->contains(L"mixed types"));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithSuggestFields() throws Exception
void TestContextSuggestField::testWithSuggestFields() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      iwcWithSuggestField(analyzer, L"suggest_field",
                          L"context_suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion1", 4));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion2", 3));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion3", 2));
  document->push_back(make_shared<ContextSuggestField>(
      L"context_suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"context_suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"context_suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion4", 1));
  document->push_back(make_shared<ContextSuggestField>(
      L"context_suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  shared_ptr<CompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"sugg"));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 10, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", 4),
                    make_shared<Entry>(L"suggestion2", 3),
                    make_shared<Entry>(L"suggestion3", 2),
                    make_shared<Entry>(L"suggestion4", 1));

  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"context_suggest_field", L"sugg"));
  suggest = suggestIndexSearcher->suggest(query, 10, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4),
                    make_shared<Entry>(L"suggestion2", L"type2", 3),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCompletionAnalyzer() throws Exception
void TestContextSuggestField::testCompletionAnalyzer() 
{
  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(make_shared<StandardAnalyzer>(), true,
                                      true);
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(completionAnalyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);
  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          completionAnalyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4),
                    make_shared<Entry>(L"suggestion2", L"type2", 3),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));
  query->addContext(L"type1");
  suggest = suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4));
  reader->close();
  delete iw;
}
} // namespace org::apache::lucene::search::suggest::document