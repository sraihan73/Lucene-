using namespace std;

#include "TestUnifiedHighlighterReanalysis.h"

namespace org::apache::lucene::search::uhighlight
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithoutIndexSearcher() throws
// java.io.IOException
void TestUnifiedHighlighterReanalysis::testWithoutIndexSearcher() throw(
    IOException)
{
  wstring text = L"This is a test. Just a test highlighting without a "
                 L"searcher. Feel free to ignore.";
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"highlighting")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"test")),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(nullptr, indexAnalyzer);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring snippet =
      highlighter->highlightWithoutSearcher(L"body", query, text, 1).toString();

  assertEquals(L"Just a test <b>highlighting</b> without a searcher. ",
               snippet);

  assertEquals(L"test single space", L" ",
               highlighter->highlightWithoutSearcher(L"body", query, L" ", 1));

  assertEquals(L"Hello", highlighter->highlightWithoutSearcher(
                             L"nonexistent", query, L"Hello", 1));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalStateException.class) public void
// testIndexSearcherNullness() throws java.io.IOException
void TestUnifiedHighlighterReanalysis::testIndexSearcherNullness() throw(
    IOException)
{
  wstring text = L"This is a test. Just a test highlighting without a "
                 L"searcher. Feel free to ignore.";
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory directory
  // = newDirectory(); org.apache.lucene.index.RandomIndexWriter indexWriter =
  // new org.apache.lucene.index.RandomIndexWriter(random(), directory);
  // org.apache.lucene.index.IndexReader indexReader = indexWriter.getReader())
  {
    org::apache::lucene::store::Directory directory = newDirectory();
    org::apache::lucene::index::RandomIndexWriter indexWriter =
        org::apache::lucene::index::RandomIndexWriter(random(), directory);
    org::apache::lucene::index::IndexReader indexReader =
        indexWriter->getReader();
    shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);
    shared_ptr<UnifiedHighlighter> highlighter =
        make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
    highlighter->highlightWithoutSearcher(L"body", query, text,
                                          1); // should throw
  }
}
} // namespace org::apache::lucene::search::uhighlight