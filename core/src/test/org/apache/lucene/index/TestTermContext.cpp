using namespace std;

#include "TestTermContext.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTermContext::testToStringOnNullTermContext() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(make_shared<Document>());
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<TermContext> context =
      TermContext::build(r->getContext(), make_shared<Term>(L"foo", L"bar"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"TermContext\n  state=null\n", context->toString());
  IOUtils::close({r, w, dir});
}
} // namespace org::apache::lucene::index