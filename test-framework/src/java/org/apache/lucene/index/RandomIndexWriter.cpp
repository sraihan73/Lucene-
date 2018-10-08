using namespace std;

#include "RandomIndexWriter.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NullInfoStream = org::apache::lucene::util::NullInfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<IndexWriter>
RandomIndexWriter::mockIndexWriter(shared_ptr<Directory> dir,
                                   shared_ptr<IndexWriterConfig> conf,
                                   shared_ptr<Random> r) 
{
  // Randomly calls Thread.yield so we mixup thread scheduling
  shared_ptr<Random> *const random = make_shared<Random>(r->nextLong());
  return mockIndexWriter(r, dir, conf,
                         make_shared<TestPointAnonymousInnerClass>(random));
}

RandomIndexWriter::TestPointAnonymousInnerClass::TestPointAnonymousInnerClass(
    shared_ptr<Random> random)
{
  this->random = random;
}

void RandomIndexWriter::TestPointAnonymousInnerClass::apply(
    const wstring &message)
{
  if (random->nextInt(4) == 2) {
    Thread::yield();
  }
}

shared_ptr<IndexWriter> RandomIndexWriter::mockIndexWriter(
    shared_ptr<Random> r, shared_ptr<Directory> dir,
    shared_ptr<IndexWriterConfig> conf,
    shared_ptr<TestPoint> testPoint) 
{
  conf->setInfoStream(
      make_shared<TestPointInfoStream>(conf->getInfoStream(), testPoint));
  shared_ptr<DirectoryReader> reader = nullptr;
  if (r->nextBoolean() && DirectoryReader::indexExists(dir) &&
      conf->getOpenMode() != IndexWriterConfig::OpenMode::CREATE) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"RIW: open writer from reader" << endl;
    }
    reader = DirectoryReader::open(dir);
    conf->setIndexCommit(reader->getIndexCommit());
  }

  shared_ptr<IndexWriter> iw;
  bool success = false;
  try {
    iw = make_shared<IndexWriterAnonymousInnerClass>(dir, conf);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      if (success) {
        IOUtils::close({reader});
      } else {
        IOUtils::closeWhileHandlingException({reader});
      }
    }
  }
  return iw;
}

RandomIndexWriter::IndexWriterAnonymousInnerClass::
    IndexWriterAnonymousInnerClass(
        shared_ptr<Directory> dir,
        shared_ptr<org::apache::lucene::index::IndexWriterConfig> conf)
    : IndexWriter(dir, conf)
{
}

bool RandomIndexWriter::IndexWriterAnonymousInnerClass::isEnableTestPoints()
{
  return true;
}

RandomIndexWriter::RandomIndexWriter(
    shared_ptr<Random> r, shared_ptr<Directory> dir) 
    : RandomIndexWriter(
          r, dir, LuceneTestCase::newIndexWriterConfig(r, new MockAnalyzer(r)),
          true, r->nextBoolean())
{
}

RandomIndexWriter::RandomIndexWriter(shared_ptr<Random> r,
                                     shared_ptr<Directory> dir,
                                     shared_ptr<Analyzer> a) 
    : RandomIndexWriter(r, dir, LuceneTestCase::newIndexWriterConfig(r, a))
{
}

RandomIndexWriter::RandomIndexWriter(
    shared_ptr<Random> r, shared_ptr<Directory> dir,
    shared_ptr<IndexWriterConfig> c) 
    : RandomIndexWriter(r, dir, c, false, r->nextBoolean())
{
}

RandomIndexWriter::RandomIndexWriter(shared_ptr<Random> r,
                                     shared_ptr<Directory> dir,
                                     shared_ptr<IndexWriterConfig> c,
                                     bool useSoftDeletes) 
    : RandomIndexWriter(r, dir, c, false, useSoftDeletes)
{
}

RandomIndexWriter::RandomIndexWriter(shared_ptr<Random> r,
                                     shared_ptr<Directory> dir,
                                     shared_ptr<IndexWriterConfig> c,
                                     bool closeAnalyzer,
                                     bool useSoftDeletes) 
    : w(mockIndexWriter(dir, c, r)), r(make_shared<Random>(r->nextLong()))
{
  // TODO: this should be solved in a different way; Random should not be shared
  // (!).
  if (useSoftDeletes) {
    c->setSoftDeletesField(L"___soft_deletes");
    softDeletesRatio = 1.d / static_cast<double>(1) + r->nextInt(10);
  } else {
    softDeletesRatio = 0;
  }
  flushAt = TestUtil::nextInt(r, 10, 1000);
  if (closeAnalyzer) {
    analyzer = w->getAnalyzer();
  } else {
    analyzer.reset();
  }
  if (LuceneTestCase::VERBOSE) {
    wcout << L"RIW dir=" << dir << endl;
  }

  // Make sure we sometimes test indices that don't get
  // any forced merges:
  doRandomForceMerge_ = !(std::dynamic_pointer_cast<NoMergePolicy>(
                              c->getMergePolicy()) != nullptr) &&
                        r->nextBoolean();
}

template <typename T>
int64_t RandomIndexWriter::addDocument(deque<T> &doc) 
{
  static_assert(is_base_of<IndexableField, T>::value,
                L"T must inherit from IndexableField");

  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  int64_t seqNo;
  if (r->nextInt(5) == 3) {
    // TODO: maybe, we should simply buffer up added docs
    // (but we need to clone them), and only when
    // getReader, commit, etc. are called, we do an
    // addDocuments?  Would be better testing.
    seqNo = w->addDocuments(
        make_shared<IterableAnonymousInnerClass>(shared_from_this(), doc));
  } else {
    seqNo = w->addDocument(doc);
  }

  maybeFlushOrCommit();

  return seqNo;
}

RandomIndexWriter::IterableAnonymousInnerClass::IterableAnonymousInnerClass(
    shared_ptr<RandomIndexWriter> outerInstance,
    deque<std::shared_ptr<T>> &doc)
{
  this->outerInstance = outerInstance;
  this->doc = doc;
}

shared_ptr<Iterator<deque<std::shared_ptr<T>>>>
RandomIndexWriter::IterableAnonymousInnerClass::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

RandomIndexWriter::IterableAnonymousInnerClass::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool RandomIndexWriter::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  return !done;
}

void RandomIndexWriter::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

deque<std::shared_ptr<T>> RandomIndexWriter::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  if (done) {
    throw make_shared<IllegalStateException>();
  }
  done = true;
  return outerInstance->doc;
}

void RandomIndexWriter::maybeFlushOrCommit() 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  if (docCount++ == flushAt) {
    if (r->nextBoolean()) {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW.add/updateDocument: now flushing the largest writer at "
                 L"docCount="
              << docCount << endl;
      }
      int activeThreadStateCount =
          w->docWriter->perThreadPool->getActiveThreadStateCount();
      int numFlushes = min(1, r->nextInt(activeThreadStateCount + 1));
      for (int i = 0; i < numFlushes; i++) {
        if (w->flushNextBuffer() == false) {
          break; // stop once we didn't flush anything
        }
      }
    } else if (r->nextBoolean()) {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW.add/updateDocument: now doing a flush at docCount="
              << docCount << endl;
      }
      w->flush();
    } else {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW.add/updateDocument: now doing a commit at docCount="
              << docCount << endl;
      }
      w->commit();
    }
    flushAt += TestUtil::nextInt(r, static_cast<int>(flushAtFactor * 10),
                                 static_cast<int>(flushAtFactor * 1000));
    if (flushAtFactor < 2e6) {
      // gradually but exponentially increase time b/w flushes
      flushAtFactor *= 1.05;
    }
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long addDocuments(Iterable<? extends
// Iterable<? extends IndexableField>> docs) throws java.io.IOException
int64_t RandomIndexWriter::addDocuments(deque<T1> docs) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  int64_t seqNo = w->addDocuments(docs);
  maybeFlushOrCommit();
  return seqNo;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public long updateDocuments(Term delTerm,
// Iterable<? extends Iterable<? extends IndexableField>> docs) throws
// java.io.IOException
int64_t RandomIndexWriter::updateDocuments(shared_ptr<Term> delTerm,
                                             deque<T1> docs) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  int64_t seqNo;
  if (useSoftDeletes()) {
    seqNo =
        w->softUpdateDocuments(delTerm, docs,
                               {make_shared<NumericDocValuesField>(
                                   w->getConfig()->getSoftDeletesField(), 1)});
  } else {
    seqNo = w->updateDocuments(delTerm, docs);
  }
  maybeFlushOrCommit();
  return seqNo;
}

bool RandomIndexWriter::useSoftDeletes()
{
  return r->nextDouble() < softDeletesRatio;
}

template <typename T>
int64_t RandomIndexWriter::updateDocument(shared_ptr<Term> t,
                                            deque<T> &doc) 
{
  static_assert(is_base_of<IndexableField, T>::value,
                L"T must inherit from IndexableField");

  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  constexpr int64_t seqNo;
  if (useSoftDeletes()) {
    if (r->nextInt(5) == 3) {
      seqNo = w->softUpdateDocuments(
          t, Arrays::asList(doc),
          {make_shared<NumericDocValuesField>(
              w->getConfig()->getSoftDeletesField(), 1)});
    } else {
      seqNo =
          w->softUpdateDocument(t, doc,
                                {make_shared<NumericDocValuesField>(
                                    w->getConfig()->getSoftDeletesField(), 1)});
    }
  } else {
    if (r->nextInt(5) == 3) {
      seqNo = w->updateDocuments(t, Arrays::asList(doc));
    } else {
      seqNo = w->updateDocument(t, doc);
    }
  }
  maybeFlushOrCommit();

  return seqNo;
}

int64_t
RandomIndexWriter::addIndexes(deque<Directory> &dirs) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->addIndexes({dirs});
}

int64_t
RandomIndexWriter::addIndexes(deque<CodecReader> &readers) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->addIndexes({readers});
}

int64_t RandomIndexWriter::updateNumericDocValue(
    shared_ptr<Term> term, const wstring &field,
    optional<int64_t> &value) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->updateNumericDocValue(term, field, value);
}

int64_t RandomIndexWriter::updateBinaryDocValue(
    shared_ptr<Term> term, const wstring &field,
    shared_ptr<BytesRef> value) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->updateBinaryDocValue(term, field, value);
}

int64_t
RandomIndexWriter::updateDocValues(shared_ptr<Term> term,
                                   deque<Field> &updates) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->updateDocValues(term, {updates});
}

int64_t
RandomIndexWriter::deleteDocuments(shared_ptr<Term> term) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->deleteDocuments({term});
}

int64_t
RandomIndexWriter::deleteDocuments(shared_ptr<Query> q) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->deleteDocuments({q});
}

int64_t RandomIndexWriter::commit() 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return w->commit();
}

int RandomIndexWriter::numDocs() { return w->numDocs(); }

int RandomIndexWriter::maxDoc() { return w->maxDoc(); }

int64_t RandomIndexWriter::deleteAll() 
{
  return w->deleteAll();
}

shared_ptr<DirectoryReader> RandomIndexWriter::getReader() 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  return getReader(true, false);
}

void RandomIndexWriter::forceMergeDeletes(bool doWait) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  w->forceMergeDeletes(doWait);
}

void RandomIndexWriter::forceMergeDeletes() 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  w->forceMergeDeletes();
}

void RandomIndexWriter::setDoRandomForceMerge(bool v)
{
  doRandomForceMerge_ = v;
}

void RandomIndexWriter::setDoRandomForceMergeAssert(bool v)
{
  doRandomForceMergeAssert = v;
}

void RandomIndexWriter::doRandomForceMerge() 
{
  if (doRandomForceMerge_) {
    constexpr int segCount = w->getSegmentCount();
    if (r->nextBoolean() || segCount == 0) {
      // full forceMerge
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW: doRandomForceMerge(1)" << endl;
      }
      w->forceMerge(1);
    } else if (r->nextBoolean()) {
      // partial forceMerge
      constexpr int limit = TestUtil::nextInt(r, 1, segCount);
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW: doRandomForceMerge(" << limit << L")" << endl;
      }
      w->forceMerge(limit);
      assert((!doRandomForceMergeAssert || w->getSegmentCount() <= limit,
              L"limit=" + to_wstring(limit) + L" actual=" +
                  to_wstring(w->getSegmentCount())));
    } else {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"RIW: do random forceMergeDeletes()" << endl;
      }
      w->forceMergeDeletes();
    }
  }
}

shared_ptr<DirectoryReader>
RandomIndexWriter::getReader(bool applyDeletions,
                             bool writeAllDeletes) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  getReaderCalled = true;
  if (r->nextInt(20) == 2) {
    doRandomForceMerge();
  }
  if (!applyDeletions || r->nextBoolean()) {
    // if we have soft deletes we can't open from a directory
    if (LuceneTestCase::VERBOSE) {
      wcout << L"RIW.getReader: use NRT reader" << endl;
    }
    if (r->nextInt(5) == 1) {
      w->commit();
    }
    return w->getReader(applyDeletions, writeAllDeletes);
  } else {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"RIW.getReader: open new reader" << endl;
    }
    w->commit();
    if (r->nextBoolean()) {
      shared_ptr<DirectoryReader> reader =
          DirectoryReader::open(w->getDirectory());
      if (w->getConfig()->getSoftDeletesField() != L"") {
        return make_shared<SoftDeletesDirectoryReaderWrapper>(
            reader, w->getConfig()->getSoftDeletesField());
      } else {
        return reader;
      }
    } else {
      return w->getReader(applyDeletions, writeAllDeletes);
    }
  }
}

RandomIndexWriter::~RandomIndexWriter()
{
  bool success = false;
  try {
    if (w->isClosed() == false) {
      LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
    }
    // if someone isn't using getReader() API, we want to be sure to
    // forceMerge since presumably they might open a reader on the dir.
    if (getReaderCalled == false && r->nextInt(8) == 2 &&
        w->isClosed() == false) {
      doRandomForceMerge();
      if (w->getConfig()->getCommitOnClose() == false) {
        // index may have changed, must commit the changes, or otherwise they
        // are discarded by the call to close()
        w->commit();
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({w, analyzer});
    } else {
      IOUtils::closeWhileHandlingException({w, analyzer});
    }
  }
}

void RandomIndexWriter::forceMerge(int maxSegmentCount) 
{
  LuceneTestCase::maybeChangeLiveIndexWriterConfig(r, w->getConfig());
  w->forceMerge(maxSegmentCount);
}

RandomIndexWriter::TestPointInfoStream::TestPointInfoStream(
    shared_ptr<InfoStream> delegate_, shared_ptr<TestPoint> testPoint)
    : delegate_(delegate_ == nullptr ? make_shared<NullInfoStream>()
                                     : delegate_),
      testPoint(testPoint)
{
}

RandomIndexWriter::TestPointInfoStream::~TestPointInfoStream()
{
  delete delegate_;
}

void RandomIndexWriter::TestPointInfoStream::message(const wstring &component,
                                                     const wstring &message)
{
  if (L"TP" == component) {
    testPoint->apply(message);
  }
  if (delegate_->isEnabled(component)) {
    delegate_->message(component, message);
  }
}

bool RandomIndexWriter::TestPointInfoStream::isEnabled(const wstring &component)
{
  return L"TP" == component || delegate_->isEnabled(component);
}

void RandomIndexWriter::flush()  { w->flush(); }
} // namespace org::apache::lucene::index