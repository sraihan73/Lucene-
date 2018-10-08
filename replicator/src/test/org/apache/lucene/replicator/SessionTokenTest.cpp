using namespace std;

#include "SessionTokenTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSerialization() throws
// java.io.IOException
void SessionTokenTest::testSerialization() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<Revision> rev = make_shared<IndexRevision>(writer);

  shared_ptr<SessionToken> session1 = make_shared<SessionToken>(L"17", rev);
  shared_ptr<ByteArrayOutputStream> baos = make_shared<ByteArrayOutputStream>();
  session1->serialize(make_shared<DataOutputStream>(baos));
  std::deque<char> b = baos->toByteArray();
  shared_ptr<SessionToken> session2 = make_shared<SessionToken>(
      make_shared<DataInputStream>(make_shared<ByteArrayInputStream>(b)));
  assertEquals(session1->id, session2->id);
  assertEquals(session1->version, session2->version);
  assertEquals(1, session2->sourceFiles.size());
  assertEquals(session1->sourceFiles.size(), session2->sourceFiles.size());
  assertEquals(session1->sourceFiles.keySet(), session2->sourceFiles.keySet());
  deque<std::shared_ptr<RevisionFile>> files1 =
      session1->sourceFiles.values().begin()->next();
  deque<std::shared_ptr<RevisionFile>> files2 =
      session2->sourceFiles.values().begin()->next();
  assertEquals(files1, files2);

  delete writer;
  IOUtils::close({dir});
}
} // namespace org::apache::lucene::replicator