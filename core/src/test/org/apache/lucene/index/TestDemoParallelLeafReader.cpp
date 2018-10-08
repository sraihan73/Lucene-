using namespace std;

#include "TestDemoParallelLeafReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using Throttling = org::apache::lucene::store::MockDirectoryWrapper::Throttling;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
const wstring TestDemoParallelLeafReader::ReindexingReader::SCHEMA_GEN_KEY =
    L"schema_gen";

void TestDemoParallelLeafReader::ReindexingReader::printRefCounts()
{
  wcout << L"All refCounts:" << endl;
  for (auto ent : parallelReaders) {
    wcout << L"  " << ent.first << L" " << ent.second << L" refCount="
          << ent.second::getRefCount() << endl;
  }
}

TestDemoParallelLeafReader::ReindexingReader::ReindexingReader(
    shared_ptr<Path> root) 
    : w(make_shared<IndexWriter>(indexDir, iwc)),
      mgr(make_shared<ReaderManager>(make_shared<ParallelLeafDirectoryReader>(
          shared_from_this(), DirectoryReader::open(w)))),
      indexDir(openDirectory(root->resolve(L"index"))), root(root),
      segsPath(root->resolve(L"segs"))
{

  // Normal index is stored under "index":

  // Per-segment parallel indices are stored under subdirs "segs":
  Files::createDirectories(segsPath);

  shared_ptr<IndexWriterConfig> iwc = getIndexWriterConfig();
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  iwc->setMergePolicy(make_shared<ReindexingMergePolicy>(
      shared_from_this(), iwc->getMergePolicy()));
  if (DEBUG) {
    wcout << L"TEST: use IWC:\n" << iwc << endl;
  }

  w->getConfig()->setMergedSegmentWarmer([&](reader) {
    if (DEBUG) {
      wcout << Thread::currentThread().getName() << L": TEST: now warm "
            << reader << endl;
    }
    getParallelLeafReader(reader, false, getCurrentSchemaGen());
  });

  // start with empty commit:
  w->commit();
}

void TestDemoParallelLeafReader::ReindexingReader::checkParallelReader(
    shared_ptr<LeafReader> reader, shared_ptr<LeafReader> parallelReader,
    int64_t schemaGen) 
{
}

shared_ptr<Directory>
TestDemoParallelLeafReader::ReindexingReader::openDirectory(
    shared_ptr<Path> path) 
{
  return FSDirectory::open(path);
}

void TestDemoParallelLeafReader::ReindexingReader::commit() 
{
  w->commit();
}

shared_ptr<LeafReader>
TestDemoParallelLeafReader::ReindexingReader::getCurrentReader(
    shared_ptr<LeafReader> reader, int64_t schemaGen) 
{
  shared_ptr<LeafReader> parallelReader =
      getParallelLeafReader(reader, true, schemaGen);
  if (parallelReader != nullptr) {

    // We should not be embedding one ParallelLeafReader inside another:
    assertFalse(std::dynamic_pointer_cast<ParallelLeafReader>(parallelReader) !=
                nullptr);
    assertFalse(std::dynamic_pointer_cast<ParallelLeafReader>(reader) !=
                nullptr);

    // NOTE: important that parallelReader is first, so if there are field name
    // overlaps, because changes to the schema overwrote existing field names,
    // it wins:
    shared_ptr<LeafReader> newReader =
        make_shared<ParallelLeafReaderAnonymousInnerClass>(
            shared_from_this(), parallelReader, reader);

    // Because ParallelLeafReader does its own (extra) incRef:
    parallelReader->decRef();

    return newReader;

  } else {
    // This segment was already current as of currentSchemaGen:
    return reader;
  }
}

TestDemoParallelLeafReader::ReindexingReader::
    ParallelLeafReaderAnonymousInnerClass::
        ParallelLeafReaderAnonymousInnerClass(
            shared_ptr<ReindexingReader> outerInstance,
            shared_ptr<org::apache::lucene::index::LeafReader> parallelReader,
            shared_ptr<org::apache::lucene::index::LeafReader> reader)
    : ParallelLeafReader(false, parallelReader, reader)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Bits> TestDemoParallelLeafReader::ReindexingReader::
    ParallelLeafReaderAnonymousInnerClass::getLiveDocs()
{
  return getParallelReaders()[1].getLiveDocs();
}

int TestDemoParallelLeafReader::ReindexingReader::
    ParallelLeafReaderAnonymousInnerClass::numDocs()
{
  return getParallelReaders()[1].numDocs();
}

TestDemoParallelLeafReader::ReindexingReader::ParallelLeafDirectoryReader::
    ParallelLeafDirectoryReader(
        shared_ptr<TestDemoParallelLeafReader::ReindexingReader> outerInstance,
        shared_ptr<DirectoryReader> in_) 
    : outerInstance(outerInstance)
{
  FilterDirectoryReader(in_,
                        make_shared<SubReaderWrapperAnonymousInnerClass>());
}

TestDemoParallelLeafReader::ReindexingReader::ParallelLeafDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader>
TestDemoParallelLeafReader::ReindexingReader::ParallelLeafDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::wrap(shared_ptr<LeafReader> reader)
{
  try {
    return outerInstance->outerInstance->getCurrentReader(reader,
                                                          currentSchemaGen);
  } catch (const IOException &ioe) {
    // TODO: must close on exc here:
    throw runtime_error(ioe);
  }
}

shared_ptr<DirectoryReader>
TestDemoParallelLeafReader::ReindexingReader::ParallelLeafDirectoryReader::
    doWrapDirectoryReader(shared_ptr<DirectoryReader> in_) 
{
  return make_shared<ParallelLeafDirectoryReader>(outerInstance, in_);
}

void TestDemoParallelLeafReader::ReindexingReader::ParallelLeafDirectoryReader::
    doClose() 
{
  runtime_error firstExc = nullptr;
  for (auto r : getSequentialSubReaders()) {
    if (std::dynamic_pointer_cast<ParallelLeafReader>(r) != nullptr) {
      // try to close each reader, even if an exception is thrown
      try {
        r->decRef();
      } catch (const runtime_error &t) {
        if (firstExc == nullptr) {
          firstExc = t;
        }
      }
    }
  }
  // Also close in, so it decRef's the SegmentInfos
  try {
    in_->doClose();
  } catch (const runtime_error &t) {
    if (firstExc == nullptr) {
      firstExc = t;
    }
  }

  // throw the first exception
  if (firstExc != nullptr) {
    throw IOUtils::rethrowAlways(firstExc);
  }
}

shared_ptr<CacheHelper> TestDemoParallelLeafReader::ReindexingReader::
    ParallelLeafDirectoryReader::getReaderCacheHelper()
{
  return nullptr;
}

TestDemoParallelLeafReader::ReindexingReader::~ReindexingReader()
{
  delete w;
  if (DEBUG) {
    wcout << L"TEST: after close writer index="
          << SegmentInfos::readLatestCommit(indexDir) << endl;
  }

  /*
  DirectoryReader r = mgr.acquire();
  try {
    TestUtil.checkReader(r);
  } finally {
    mgr.release(r);
  }
  */
  delete mgr;
  pruneOldSegments(true);
  assertNoExtraSegments();
  delete indexDir;
}

void TestDemoParallelLeafReader::ReindexingReader::
    assertNoExtraSegments() 
{
  shared_ptr<Set<wstring>> liveIDs = unordered_set<wstring>();
  for (auto info : SegmentInfos::readLatestCommit(indexDir)) {
    wstring idString = StringHelper::idToString(info->info->getId());
    liveIDs->add(idString);
  }

  // At this point (closing) the only segments in closedSegments should be the
  // still-live ones:
  for (auto segIDGen : closedSegments) {
    assertTrue(liveIDs->contains(segIDGen->segID));
  }

  bool fail = false;
  for (auto path : segSubDirs(segsPath)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<SegmentIDAndGen> segIDGen =
        make_shared<SegmentIDAndGen>(path->getFileName()->toString());
    if (liveIDs->contains(segIDGen->segID) == false) {
      if (DEBUG) {
        wcout << L"TEST: fail seg=" << path->getFileName()
              << L" is not live but still has a parallel index" << endl;
      }
      fail = true;
    }
  }
  assertFalse(fail);
}

TestDemoParallelLeafReader::ReindexingReader::SegmentIDAndGen::SegmentIDAndGen(
    const wstring &segID, int64_t schemaGen)
    : segID(segID), schemaGen(schemaGen)
{
}

TestDemoParallelLeafReader::ReindexingReader::SegmentIDAndGen::SegmentIDAndGen(
    const wstring &s)
    : segID(parts[0]), schemaGen(static_cast<int64_t>(parts[1]))
{
  std::deque<wstring> parts = s.split(L"_");
  if (parts.size() != 2) {
    throw invalid_argument(L"invalid SegmentIDAndGen \"" + s + L"\"");
  }
  // TODO: better checking of segID?
}

int TestDemoParallelLeafReader::ReindexingReader::SegmentIDAndGen::hashCode()
{
  return static_cast<int>(segID.hashCode() * schemaGen);
}

bool TestDemoParallelLeafReader::ReindexingReader::SegmentIDAndGen::equals(
    any _other)
{
  if (std::dynamic_pointer_cast<SegmentIDAndGen>(_other) != nullptr) {
    shared_ptr<SegmentIDAndGen> other =
        any_cast<std::shared_ptr<SegmentIDAndGen>>(_other);
    return segID == other->segID && schemaGen == other->schemaGen;
  } else {
    return false;
  }
}

wstring
TestDemoParallelLeafReader::ReindexingReader::SegmentIDAndGen::toString()
{
  return segID + L"_" + to_wstring(schemaGen);
}

TestDemoParallelLeafReader::ReindexingReader::ParallelReaderClosed::
    ParallelReaderClosed(
        shared_ptr<TestDemoParallelLeafReader::ReindexingReader> outerInstance,
        shared_ptr<SegmentIDAndGen> segIDGen, shared_ptr<Directory> dir)
    : segIDGen(segIDGen), dir(dir), outerInstance(outerInstance)
{
}

void TestDemoParallelLeafReader::ReindexingReader::ParallelReaderClosed::
    onClose(shared_ptr<IndexReader::CacheKey> ignored)
{
  try {
    {
      // TODO: make this sync finer, i.e. just the segment + schemaGen
      lock_guard<mutex> lock(outerInstance);
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST: now close parallel parLeafReader dir=" << dir
              << L" segIDGen=" << segIDGen << endl;
      }
      outerInstance->parallelReaders.erase(segIDGen);
      delete dir;
      outerInstance->closedSegments->add(segIDGen);
    }
  } catch (const IOException &ioe) {
    wcout << L"TEST: hit IOExc closing dir=" << dir << endl;
    ioe->printStackTrace(System::out);
    throw runtime_error(ioe);
  }
}

shared_ptr<LeafReader>
TestDemoParallelLeafReader::ReindexingReader::getParallelLeafReader(
    shared_ptr<LeafReader> leaf, bool doCache,
    int64_t schemaGen) 
{
  assert(std::dynamic_pointer_cast<SegmentReader>(leaf) != nullptr);
  shared_ptr<SegmentInfo> info =
      (std::static_pointer_cast<SegmentReader>(leaf))->getSegmentInfo()->info;

  int64_t infoSchemaGen = getSchemaGen(info);

  if (DEBUG) {
    wcout << Thread::currentThread().getName()
          << L": TEST: getParallelLeafReader: " << leaf << L" infoSchemaGen="
          << infoSchemaGen << L" vs schemaGen=" << schemaGen << L" doCache="
          << doCache << endl;
  }

  if (infoSchemaGen == schemaGen) {
    if (DEBUG) {
      wcout << Thread::currentThread().getName()
            << L": TEST: segment is already current schemaGen=" << schemaGen
            << L"; skipping" << endl;
    }
    return nullptr;
  }

  if (infoSchemaGen > schemaGen) {
    throw make_shared<IllegalStateException>(
        L"segment infoSchemaGen (" + to_wstring(infoSchemaGen) +
        L") cannot be greater than requested schemaGen (" +
        to_wstring(schemaGen) + L")");
  }

  shared_ptr<SegmentIDAndGen> *const segIDGen = make_shared<SegmentIDAndGen>(
      StringHelper::idToString(info->getId()), schemaGen);

  // While loop because the parallel reader may be closed out from under us, so
  // we must retry:
  while (true) {

    // TODO: make this sync finer, i.e. just the segment + schemaGen
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      shared_ptr<LeafReader> parReader = parallelReaders[segIDGen];

      assert(doCache || parReader == nullptr);

      if (parReader == nullptr) {

        // C++ TODO: There is no native C++ equivalent to 'toString':
        shared_ptr<Path> leafIndex = segsPath->resolve(segIDGen->toString());

        shared_ptr<Directory> *const dir = openDirectory(leafIndex);

        if (LuceneTestCase::slowFileExists(dir, L"done") == false) {
          if (DEBUG) {
            wcout << Thread::currentThread().getName()
                  << L": TEST: build segment index for " << leaf << L" "
                  << segIDGen << L" (source: "
                  << info->getDiagnostics()[L"source"] << L") dir=" << leafIndex
                  << endl;
          }

          if (dir->listAll().size() != 0) {
            // It crashed before finishing last time:
            if (DEBUG) {
              wcout << Thread::currentThread().getName()
                    << L": TEST: remove old incomplete index files: "
                    << leafIndex << endl;
            }
            IOUtils::rm({leafIndex});
          }

          reindex(infoSchemaGen, schemaGen, leaf, dir);

          // Marker file, telling us this index is in fact done.  This way if we
          // crash while doing the reindexing for a given segment, we will later
          // try again:
          delete dir->createOutput(L"done", IOContext::DEFAULT);
        } else {
          if (DEBUG) {
            wcout << Thread::currentThread().getName()
                  << L": TEST: segment index already exists for " << leaf
                  << L" " << segIDGen << L" (source: "
                  << info->getDiagnostics()[L"source"] << L") dir=" << leafIndex
                  << endl;
          }
        }

        if (DEBUG) {
          wcout << Thread::currentThread().getName()
                << L": TEST: now check index " << dir << endl;
        }
        // TestUtil.checkIndex(dir);

        shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
        assert(infos->size() == 1);
        shared_ptr<LeafReader> *const parLeafReader =
            make_shared<SegmentReader>(infos->info(0), Version::LATEST->major,
                                       IOContext::DEFAULT);

        // checkParallelReader(leaf, parLeafReader, schemaGen);

        if (DEBUG) {
          wcout << Thread::currentThread().getName()
                << L": TEST: opened parallel reader: " << parLeafReader << endl;
        }
        if (doCache) {
          parallelReaders.emplace(segIDGen, parLeafReader);

          // Our id+gen could have been previously closed, e.g. if it was a
          // merged segment that was warmed, so we must clear this else the
          // pruning may remove our directory:
          closedSegments->remove(segIDGen);

          parLeafReader->getReaderCacheHelper()->addClosedListener(
              make_shared<ParallelReaderClosed>(shared_from_this(), segIDGen,
                                                dir));

        } else {
          // Used only for merged segment warming:
          // Messy: we close this reader now, instead of leaving open for reuse:
          if (DEBUG) {
            wcout << L"TEST: now decRef non cached refCount="
                  << parLeafReader->getRefCount() << endl;
          }
          parLeafReader->decRef();
          delete dir;

          // Must do this after dir is closed, else another thread could "rm
          // -rf" while we are closing (which makes MDW.close's checkIndex
          // angry):
          closedSegments->add(segIDGen);
          parReader.reset();
        }
        parReader = parLeafReader;

      } else {
        if (parReader->tryIncRef() == false) {
          // We failed: this reader just got closed by another thread, e.g.
          // refresh thread opening a new reader, so this reader is now closed
          // and we must try again.
          if (DEBUG) {
            wcout << Thread::currentThread().getName()
                  << L": TEST: tryIncRef failed for " << parReader << L"; retry"
                  << endl;
          }
          parReader.reset();
          continue;
        }
        if (DEBUG) {
          wcout << Thread::currentThread().getName()
                << L": TEST: use existing already opened parReader="
                << parReader << L" refCount=" << parReader->getRefCount()
                << endl;
        }
        // checkParallelReader(leaf, parReader, schemaGen);
      }

      // We return the new reference to caller
      return parReader;
    }
  }
}

int64_t TestDemoParallelLeafReader::ReindexingReader::getMergingSchemaGen()
{
  return getCurrentSchemaGen();
}

void TestDemoParallelLeafReader::ReindexingReader::pruneOldSegments(
    bool removeOldGens) 
{
  shared_ptr<SegmentInfos> lastCommit =
      SegmentInfos::readLatestCommit(indexDir);
  if (DEBUG) {
    wcout << L"TEST: prune" << endl;
  }

  shared_ptr<Set<wstring>> liveIDs = unordered_set<wstring>();
  for (auto info : lastCommit) {
    wstring idString = StringHelper::idToString(info->info->getId());
    liveIDs->add(idString);
  }

  int64_t currentSchemaGen = getCurrentSchemaGen();

  if (Files::exists(segsPath)) {
    for (auto path : segSubDirs(segsPath)) {
      if (Files::isDirectory(path)) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        shared_ptr<SegmentIDAndGen> segIDGen =
            make_shared<SegmentIDAndGen>(path->getFileName()->toString());
        assert(segIDGen->schemaGen <= currentSchemaGen);
        if (liveIDs->contains(segIDGen->segID) == false &&
            (closedSegments->contains(segIDGen) ||
             (removeOldGens && segIDGen->schemaGen < currentSchemaGen))) {
          if (DEBUG) {
            wcout << L"TEST: remove " << segIDGen << endl;
          }
          try {
            IOUtils::rm({path});
            closedSegments->remove(segIDGen);
          } catch (const IOException &ioe) {
            // OK, we'll retry later
            if (DEBUG) {
              wcout << L"TEST: ignore ioe during delete " << path << L":" << ioe
                    << endl;
            }
          }
        }
      }
    }
  }
}

TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingOneMerge::ReindexingOneMerge(
        shared_ptr<
            TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy>
            outerInstance,
        deque<std::shared_ptr<SegmentCommitInfo>> &segments)
    : OneMerge(segments),
      schemaGen(outerInstance->outerInstance->getMergingSchemaGen()),
      outerInstance(outerInstance)
{
  // Commit up front to which schemaGen we will merge; we don't want a schema
  // change sneaking in for some of our leaf readers but not others:
  int64_t currentSchemaGen =
      outerInstance->outerInstance->getCurrentSchemaGen();

  // Defensive sanity check:
  if (schemaGen > currentSchemaGen) {
    throw make_shared<IllegalStateException>(
        L"currentSchemaGen (" + to_wstring(currentSchemaGen) +
        L") must always be >= mergingSchemaGen (" + to_wstring(schemaGen) +
        L")");
  }
}

shared_ptr<CodecReader> TestDemoParallelLeafReader::ReindexingReader::
    ReindexingMergePolicy::ReindexingOneMerge::wrapForMerge(
        shared_ptr<CodecReader> reader) 
{
  shared_ptr<LeafReader> wrapped =
      outerInstance->outerInstance->getCurrentReader(
          std::static_pointer_cast<SegmentReader>(reader), schemaGen);
  if (std::dynamic_pointer_cast<ParallelLeafReader>(wrapped) != nullptr) {
    parallelReaders.push_back(
        std::static_pointer_cast<ParallelLeafReader>(wrapped));
  }
  return SlowCodecReaderWrapper::wrap(wrapped);
}

void TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingOneMerge::mergeFinished() 
{
  runtime_error th = nullptr;
  for (auto r : parallelReaders) {
    try {
      r->decRef();
    } catch (const runtime_error &t) {
      if (th == nullptr) {
        th = t;
      }
    }
  }

  if (th != nullptr) {
    throw IOUtils::rethrowAlways(th);
  }
}

void TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingOneMerge::setMergeInfo(shared_ptr<SegmentCommitInfo> info)
{
  // Record that this merged segment is current as of this schemaGen:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  info->info->getDiagnostics().emplace(SCHEMA_GEN_KEY,
                                       Long::toString(schemaGen));
  OneMerge::setMergeInfo(info);
}

TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingMergeSpecification::ReindexingMergeSpecification(
        shared_ptr<
            TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy>
            outerInstance)
    : outerInstance(outerInstance)
{
}

void TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingMergeSpecification::add(shared_ptr<OneMerge> merge)
{
  MergeSpecification::add(
      make_shared<ReindexingOneMerge>(outerInstance, merge->segments));
}

wstring TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingMergeSpecification::segString(shared_ptr<Directory> dir)
{
  return L"ReindexingMergeSpec(" + MergeSpecification::segString(dir) + L")";
}

shared_ptr<MergeSpecification>
TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::wrap(
    shared_ptr<MergeSpecification> spec)
{
  shared_ptr<MergeSpecification> wrapped = nullptr;
  if (spec != nullptr) {
    wrapped = make_shared<ReindexingMergeSpecification>(shared_from_this());
    for (auto merge : spec->merges) {
      wrapped->add(merge);
    }
  }
  return wrapped;
}

TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    ReindexingMergePolicy(
        shared_ptr<TestDemoParallelLeafReader::ReindexingReader> outerInstance,
        shared_ptr<MergePolicy> in_)
    : FilterMergePolicy(in_), outerInstance(outerInstance)
{
}

shared_ptr<MergeSpecification>
TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return wrap(in_->findMerges(mergeTrigger, segmentInfos, mergeContext));
}

shared_ptr<MergeSpecification>
TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    findForcedMerges(shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
                     unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
                         &segmentsToMerge,
                     shared_ptr<MergeContext> mergeContext) 
{
  // TODO: do we need to force-force this?  Ie, wrapped MP may think index is
  // already optimized, yet maybe its schemaGen is old?  need test!
  return wrap(in_->findForcedMerges(segmentInfos, maxSegmentCount,
                                    segmentsToMerge, mergeContext));
}

shared_ptr<MergeSpecification> TestDemoParallelLeafReader::ReindexingReader::
    ReindexingMergePolicy::findForcedDeletesMerges(
        shared_ptr<SegmentInfos> segmentInfos,
        shared_ptr<MergeContext> mergeContext) 
{
  return wrap(in_->findForcedDeletesMerges(segmentInfos, mergeContext));
}

bool TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::
    useCompoundFile(shared_ptr<SegmentInfos> segments,
                    shared_ptr<SegmentCommitInfo> newSegment,
                    shared_ptr<MergeContext> mergeContext) 
{
  return in_->useCompoundFile(segments, newSegment, mergeContext);
}

wstring
TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy::toString()
{
  return L"ReindexingMergePolicy(" + in_ + L")";
}

int64_t TestDemoParallelLeafReader::ReindexingReader::getSchemaGen(
    shared_ptr<SegmentInfo> info)
{
  wstring s = info->getDiagnostics()[SCHEMA_GEN_KEY];
  if (s == L"") {
    return -1;
  } else {
    return StringHelper::fromString<int64_t>(s);
  }
}

shared_ptr<ReindexingReader> TestDemoParallelLeafReader::getReindexer(
    shared_ptr<Path> root) 
{
  return make_shared<ReindexingReaderAnonymousInnerClass>(shared_from_this(),
                                                          root);
}

TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass::
    ReindexingReaderAnonymousInnerClass(
        shared_ptr<TestDemoParallelLeafReader> outerInstance,
        shared_ptr<Path> root)
    : ReindexingReader(root)
{
  this->outerInstance = outerInstance;
}

shared_ptr<IndexWriterConfig>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass::
    getIndexWriterConfig() 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  // We write tiny docs, so we need tiny floor to avoid O(N^2) merging:
  tmp->setFloorSegmentMB(.01);
  iwc->setMergePolicy(tmp);
  return iwc;
}

shared_ptr<Directory>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass::openDirectory(
    shared_ptr<Path> path) 
{
  shared_ptr<MockDirectoryWrapper> dir =
      LuceneTestCase::newMockFSDirectory(path);
  dir->setUseSlowOpenClosers(false);
  dir->setThrottling(Throttling::NEVER);
  return dir;
}

void TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass::reindex(
    int64_t oldSchemaGen, int64_t newSchemaGen,
    shared_ptr<LeafReader> reader,
    shared_ptr<Directory> parallelDir) 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();

  // The order of our docIDs must precisely matching incoming reader:
  iwc->setMergePolicy(make_shared<LogByteSizeMergePolicy>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(parallelDir, iwc);
  int maxDoc = reader->maxDoc();

  // Slowly parse the stored field into a new doc values field:
  for (int i = 0; i < maxDoc; i++) {
    // TODO: is this still O(blockSize^2)?
    shared_ptr<Document> oldDoc = reader->document(i);
    shared_ptr<Document> newDoc = make_shared<Document>();
    int64_t value = static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
    newDoc->push_back(make_shared<NumericDocValuesField>(L"number", value));
    newDoc->push_back(make_shared<LongPoint>(L"number", value));
    w->addDocument(newDoc);
  }

  w->forceMerge(1);

  delete w;
}

int64_t TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass::
    getCurrentSchemaGen()
{
  return 0;
}

shared_ptr<ReindexingReader>
TestDemoParallelLeafReader::getReindexerNewDVFields(
    shared_ptr<Path> root,
    shared_ptr<AtomicLong> currentSchemaGen) 
{
  return make_shared<ReindexingReaderAnonymousInnerClass2>(
      shared_from_this(), root, currentSchemaGen);
}

TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::
    ReindexingReaderAnonymousInnerClass2(
        shared_ptr<TestDemoParallelLeafReader> outerInstance,
        shared_ptr<Path> root, shared_ptr<AtomicLong> currentSchemaGen)
    : ReindexingReader(root)
{
  this->outerInstance = outerInstance;
  this->currentSchemaGen = currentSchemaGen;
}

shared_ptr<IndexWriterConfig>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::
    getIndexWriterConfig() 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  // We write tiny docs, so we need tiny floor to avoid O(N^2) merging:
  tmp->setFloorSegmentMB(.01);
  iwc->setMergePolicy(tmp);
  return iwc;
}

shared_ptr<Directory>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::openDirectory(
    shared_ptr<Path> path) 
{
  shared_ptr<MockDirectoryWrapper> dir =
      LuceneTestCase::newMockFSDirectory(path);
  dir->setUseSlowOpenClosers(false);
  dir->setThrottling(Throttling::NEVER);
  return dir;
}

void TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::reindex(
    int64_t oldSchemaGen, int64_t newSchemaGen,
    shared_ptr<LeafReader> reader,
    shared_ptr<Directory> parallelDir) 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();

  // The order of our docIDs must precisely matching incoming reader:
  iwc->setMergePolicy(make_shared<LogByteSizeMergePolicy>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(parallelDir, iwc);
  int maxDoc = reader->maxDoc();

  if (oldSchemaGen <= 0) {
    // Must slowly parse the stored field into a new doc values field:
    for (int i = 0; i < maxDoc; i++) {
      // TODO: is this still O(blockSize^2)?
      shared_ptr<Document> oldDoc = reader->document(i);
      shared_ptr<Document> newDoc = make_shared<Document>();
      int64_t value = static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
      newDoc->push_back(make_shared<NumericDocValuesField>(
          L"number_" + to_wstring(newSchemaGen), value));
      newDoc->push_back(make_shared<LongPoint>(L"number", value));
      w->addDocument(newDoc);
    }
  } else {
    // Just carry over doc values from previous field:
    shared_ptr<NumericDocValues> oldValues =
        reader->getNumericDocValues(L"number_" + to_wstring(oldSchemaGen));
    assertNotNull(L"oldSchemaGen=" + to_wstring(oldSchemaGen), oldValues);
    for (int i = 0; i < maxDoc; i++) {
      // TODO: is this still O(blockSize^2)?
      TestUtil::assertEquals(i, oldValues->nextDoc());
      shared_ptr<Document> oldDoc = reader->document(i);
      shared_ptr<Document> newDoc = make_shared<Document>();
      newDoc->push_back(make_shared<NumericDocValuesField>(
          L"number_" + to_wstring(newSchemaGen), oldValues->longValue()));
      w->addDocument(newDoc);
    }
  }

  w->forceMerge(1);

  delete w;
}

int64_t TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::
    getCurrentSchemaGen()
{
  return currentSchemaGen->get();
}

void TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass2::
    checkParallelReader(shared_ptr<LeafReader> r, shared_ptr<LeafReader> parR,
                        int64_t schemaGen) 
{
  wstring fieldName = L"number_" + to_wstring(schemaGen);
  if (DEBUG) {
    wcout << Thread::currentThread().getName()
          << L": TEST: now check parallel number DVs field=" << fieldName
          << L" r=" << r << L" parR=" << parR << endl;
  }
  shared_ptr<NumericDocValues> numbers = parR->getNumericDocValues(fieldName);
  if (numbers == nullptr) {
    return;
  }
  int maxDoc = r->maxDoc();
  bool failed = false;
  for (int i = 0; i < maxDoc; i++) {
    shared_ptr<Document> oldDoc = r->document(i);
    int64_t value = static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
    TestUtil::assertEquals(i, numbers->nextDoc());
    if (value != numbers->longValue()) {
      if (DEBUG) {
        wcout << L"FAIL: docID=" << i << L" " << oldDoc << L" value=" << value
              << L" number=" << numbers->longValue() << L" numbers=" << numbers
              << endl;
      }
      failed = true;
    } else if (failed) {
      if (DEBUG) {
        wcout << L"OK: docID=" << i << L" " << oldDoc << L" value=" << value
              << L" number=" << numbers->longValue() << endl;
      }
    }
  }
  assertFalse(L"FAILED field=" + fieldName + L" r=" + r, failed);
}

shared_ptr<ReindexingReader>
TestDemoParallelLeafReader::getReindexerSameDVField(
    shared_ptr<Path> root, shared_ptr<AtomicLong> currentSchemaGen,
    shared_ptr<AtomicLong> mergingSchemaGen) 
{
  return make_shared<ReindexingReaderAnonymousInnerClass3>(
      shared_from_this(), root, currentSchemaGen, mergingSchemaGen);
}

TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::
    ReindexingReaderAnonymousInnerClass3(
        shared_ptr<TestDemoParallelLeafReader> outerInstance,
        shared_ptr<Path> root, shared_ptr<AtomicLong> currentSchemaGen,
        shared_ptr<AtomicLong> mergingSchemaGen)
    : ReindexingReader(root)
{
  this->outerInstance = outerInstance;
  this->currentSchemaGen = currentSchemaGen;
  this->mergingSchemaGen = mergingSchemaGen;
}

shared_ptr<IndexWriterConfig>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::
    getIndexWriterConfig() 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  // We write tiny docs, so we need tiny floor to avoid O(N^2) merging:
  tmp->setFloorSegmentMB(.01);
  iwc->setMergePolicy(tmp);
  if (TEST_NIGHTLY) {
    // during nightly tests, we might use too many files if we arent careful
    iwc->setUseCompoundFile(true);
  }
  return iwc;
}

shared_ptr<Directory>
TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::openDirectory(
    shared_ptr<Path> path) 
{
  shared_ptr<MockDirectoryWrapper> dir =
      LuceneTestCase::newMockFSDirectory(path);
  dir->setUseSlowOpenClosers(false);
  dir->setThrottling(Throttling::NEVER);
  return dir;
}

void TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::reindex(
    int64_t oldSchemaGen, int64_t newSchemaGen,
    shared_ptr<LeafReader> reader,
    shared_ptr<Directory> parallelDir) 
{
  shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig();

  // The order of our docIDs must precisely matching incoming reader:
  iwc->setMergePolicy(make_shared<LogByteSizeMergePolicy>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(parallelDir, iwc);
  int maxDoc = reader->maxDoc();

  if (oldSchemaGen <= 0) {
    // Must slowly parse the stored field into a new doc values field:
    for (int i = 0; i < maxDoc; i++) {
      // TODO: is this still O(blockSize^2)?
      shared_ptr<Document> oldDoc = reader->document(i);
      shared_ptr<Document> newDoc = make_shared<Document>();
      int64_t value = static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
      newDoc->push_back(
          make_shared<NumericDocValuesField>(L"number", newSchemaGen * value));
      newDoc->push_back(make_shared<LongPoint>(L"number", value));
      w->addDocument(newDoc);
    }
  } else {
    // Just carry over doc values from previous field:
    shared_ptr<NumericDocValues> oldValues =
        reader->getNumericDocValues(L"number");
    assertNotNull(L"oldSchemaGen=" + to_wstring(oldSchemaGen), oldValues);
    for (int i = 0; i < maxDoc; i++) {
      // TODO: is this still O(blockSize^2)?
      shared_ptr<Document> oldDoc = reader->document(i);
      shared_ptr<Document> newDoc = make_shared<Document>();
      TestUtil::assertEquals(i, oldValues->nextDoc());
      newDoc->push_back(make_shared<NumericDocValuesField>(
          L"number", newSchemaGen * (oldValues->longValue() / oldSchemaGen)));
      w->addDocument(newDoc);
    }
  }

  w->forceMerge(1);

  delete w;
}

int64_t TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::
    getCurrentSchemaGen()
{
  return currentSchemaGen->get();
}

int64_t TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::
    getMergingSchemaGen()
{
  return mergingSchemaGen->get();
}

void TestDemoParallelLeafReader::ReindexingReaderAnonymousInnerClass3::
    checkParallelReader(shared_ptr<LeafReader> r, shared_ptr<LeafReader> parR,
                        int64_t schemaGen) 
{
  if (DEBUG) {
    wcout << Thread::currentThread().getName()
          << L": TEST: now check parallel number DVs r=" << r << L" parR="
          << parR << endl;
  }
  shared_ptr<NumericDocValues> numbers = parR->getNumericDocValues(L"numbers");
  if (numbers == nullptr) {
    return;
  }
  int maxDoc = r->maxDoc();
  bool failed = false;
  for (int i = 0; i < maxDoc; i++) {
    shared_ptr<Document> oldDoc = r->document(i);
    int64_t value = static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
    value *= schemaGen;
    TestUtil::assertEquals(i, numbers->nextDoc());
    if (value != numbers->longValue()) {
      wcout << L"FAIL: docID=" << i << L" " << oldDoc << L" value=" << value
            << L" number=" << numbers->longValue() << L" numbers=" << numbers
            << endl;
      failed = true;
    } else if (failed) {
      wcout << L"OK: docID=" << i << L" " << oldDoc << L" value=" << value
            << L" number=" << numbers->longValue() << endl;
    }
  }
  assertFalse(L"FAILED r=" + r, failed);
}

void TestDemoParallelLeafReader::testBasicMultipleSchemaGens() throw(
    runtime_error)
{

  shared_ptr<AtomicLong> currentSchemaGen = make_shared<AtomicLong>();

  // TODO: separate refresh thread, search threads, indexing threads
  shared_ptr<Path> root = createTempDir();
  shared_ptr<ReindexingReader> reindexer =
      getReindexerNewDVFields(root, currentSchemaGen);
  reindexer->commit();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                              Field::Store::YES));
  reindexer->w->addDocument(doc);

  if (DEBUG) {
    wcout << Thread::currentThread().getName() << L": TEST: refresh @ 1 doc"
          << endl;
  }
  reindexer->mgr->maybeRefresh();
  shared_ptr<DirectoryReader> r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << Thread::currentThread().getName() << L": TEST: got reader=" << r
          << endl;
  }
  try {
    checkAllNumberDVs(r, L"number_" + currentSchemaGen->get(), true, 1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }
  // reindexer.printRefCounts();

  currentSchemaGen->incrementAndGet();

  if (DEBUG) {
    wcout << Thread::currentThread().getName() << L": TEST: increment schemaGen"
          << endl;
  }
  if (DEBUG) {
    wcout << Thread::currentThread().getName() << L": TEST: commit" << endl;
  }
  reindexer->commit();

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                              Field::Store::YES));
  reindexer->w->addDocument(doc);

  if (DEBUG) {
    wcout << L"TEST: refresh @ 2 docs" << endl;
  }
  reindexer->mgr->maybeRefresh();
  // reindexer.printRefCounts();
  r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  try {
    checkAllNumberDVs(r, L"number_" + currentSchemaGen->get(), true, 1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }

  if (DEBUG) {
    wcout << L"TEST: forceMerge" << endl;
  }
  reindexer->w->forceMerge(1);

  currentSchemaGen->incrementAndGet();

  if (DEBUG) {
    wcout << L"TEST: commit" << endl;
  }
  reindexer->commit();

  if (DEBUG) {
    wcout << L"TEST: refresh after forceMerge" << endl;
  }
  reindexer->mgr->maybeRefresh();
  r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  try {
    checkAllNumberDVs(r, L"number_" + currentSchemaGen->get(), true, 1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }

  if (DEBUG) {
    wcout << L"TEST: close writer" << endl;
  }
  delete reindexer;
}

void TestDemoParallelLeafReader::testRandomMultipleSchemaGens() throw(
    runtime_error)
{

  shared_ptr<AtomicLong> currentSchemaGen = make_shared<AtomicLong>();
  shared_ptr<ReindexingReader> reindexer = nullptr;

  // TODO: separate refresh thread, search threads, indexing threads
  int numDocs = atLeast(TEST_NIGHTLY ? 20000 : 1000);
  int maxID = 0;
  shared_ptr<Path> root = createTempDir();
  int refreshEveryNumDocs = 100;
  int commitCloseNumDocs = 1000;
  for (int i = 0; i < numDocs; i++) {
    if (reindexer == nullptr) {
      reindexer = getReindexerNewDVFields(root, currentSchemaGen);
    }

    shared_ptr<Document> doc = make_shared<Document>();
    wstring id;
    wstring updateID;
    if (maxID > 0 && random()->nextInt(10) == 7) {
      // Replace a doc
      id = L"" + random()->nextInt(maxID);
      updateID = id;
    } else {
      id = L"" + to_wstring(maxID++);
      updateID = L"";
    }

    doc->push_back(newStringField(L"id", id, Field::Store::NO));
    doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                                Field::Store::YES));
    if (updateID == L"") {
      reindexer->w->addDocument(doc);
    } else {
      reindexer->w->updateDocument(make_shared<Term>(L"id", updateID), doc);
    }
    if (random()->nextInt(refreshEveryNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: refresh @ "
              << (i << 1) << L" docs" << endl;
      }
      reindexer->mgr->maybeRefresh();

      shared_ptr<DirectoryReader> r = reindexer->mgr->acquire();
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: got reader="
              << r << endl;
      }
      try {
        checkAllNumberDVs(r, L"number_" + currentSchemaGen->get(), true, 1);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reindexer->mgr->release(r);
      }
      if (DEBUG) {
        reindexer->printRefCounts();
      }
      refreshEveryNumDocs = static_cast<int>(1.25 * refreshEveryNumDocs);
    }

    if (random()->nextInt(500) == 17) {
      currentSchemaGen->incrementAndGet();
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST TOP: advance schemaGen to " << currentSchemaGen
              << endl;
      }
    }

    if (i > 0 && random()->nextInt(10) == 7) {
      // Random delete:
      reindexer->w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + random()->nextInt(i))});
    }

    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: commit @ "
              << (i << 1) << L" docs" << endl;
      }
      reindexer->commit();
      // reindexer.printRefCounts();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }

    // Sometimes close & reopen writer/manager, to confirm the parallel segments
    // persist:
    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST TOP: close writer @ " << (i << 1) << L" docs" << endl;
      }
      delete reindexer;
      reindexer.reset();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }
  }

  if (reindexer != nullptr) {
    delete reindexer;
  }
}

void TestDemoParallelLeafReader::testRandomMultipleSchemaGensSameField() throw(
    runtime_error)
{

  shared_ptr<AtomicLong> currentSchemaGen = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> mergingSchemaGen = make_shared<AtomicLong>();

  shared_ptr<ReindexingReader> reindexer = nullptr;

  // TODO: separate refresh thread, search threads, indexing threads
  int numDocs = atLeast(TEST_NIGHTLY ? 20000 : 1000);
  int maxID = 0;
  shared_ptr<Path> root = createTempDir();
  int refreshEveryNumDocs = 100;
  int commitCloseNumDocs = 1000;

  for (int i = 0; i < numDocs; i++) {
    if (reindexer == nullptr) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST TOP: open new reader/writer" << endl;
      }
      reindexer =
          getReindexerSameDVField(root, currentSchemaGen, mergingSchemaGen);
    }

    shared_ptr<Document> doc = make_shared<Document>();
    wstring id;
    wstring updateID;
    if (maxID > 0 && random()->nextInt(10) == 7) {
      // Replace a doc
      id = L"" + random()->nextInt(maxID);
      updateID = id;
    } else {
      id = L"" + to_wstring(maxID++);
      updateID = L"";
    }

    doc->push_back(newStringField(L"id", id, Field::Store::NO));
    doc->push_back(newTextField(
        L"text",
        L"number " + to_wstring(TestUtil::nextInt(random(), -10000, 10000)),
        Field::Store::YES));
    if (updateID == L"") {
      reindexer->w->addDocument(doc);
    } else {
      reindexer->w->updateDocument(make_shared<Term>(L"id", updateID), doc);
    }
    if (random()->nextInt(refreshEveryNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: refresh @ "
              << (i << 1) << L" docs" << endl;
      }
      reindexer->mgr->maybeRefresh();
      shared_ptr<DirectoryReader> r = reindexer->mgr->acquire();
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: got reader="
              << r << endl;
      }
      try {
        checkAllNumberDVs(r, L"number", true,
                          static_cast<int>(currentSchemaGen->get()));
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reindexer->mgr->release(r);
      }
      if (DEBUG) {
        reindexer->printRefCounts();
      }
      refreshEveryNumDocs = static_cast<int>(1.25 * refreshEveryNumDocs);
    }

    if (random()->nextInt(500) == 17) {
      currentSchemaGen->incrementAndGet();
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST TOP: advance schemaGen to " << currentSchemaGen
              << endl;
      }
      if (random()->nextBoolean()) {
        mergingSchemaGen->incrementAndGet();
        if (DEBUG) {
          wcout << Thread::currentThread().getName()
                << L": TEST TOP: advance mergingSchemaGen to "
                << mergingSchemaGen << endl;
        }
      }
    }

    if (i > 0 && random()->nextInt(10) == 7) {
      // Random delete:
      reindexer->w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + random()->nextInt(i))});
    }

    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName() << L": TEST TOP: commit @ "
              << (i << 1) << L" docs" << endl;
      }
      reindexer->commit();
      // reindexer.printRefCounts();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }

    // Sometimes close & reopen writer/manager, to confirm the parallel segments
    // persist:
    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << Thread::currentThread().getName()
              << L": TEST TOP: close writer @ " << (i << 1) << L" docs" << endl;
      }
      delete reindexer;
      reindexer.reset();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }
  }

  if (reindexer != nullptr) {
    delete reindexer;
  }

  // Verify main index never reflects schema changes beyond mergingSchemaGen:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newFSDirectory(root.resolve("index")); IndexReader r =
  // DirectoryReader.open(dir))
  {
    org::apache::lucene::store::Directory dir =
        newFSDirectory(root->resolve(L"index"));
    IndexReader r = DirectoryReader::open(dir);
    for (auto ctx : r->leaves()) {
      shared_ptr<LeafReader> leaf = ctx->reader();
      shared_ptr<NumericDocValues> numbers =
          leaf->getNumericDocValues(L"number");
      if (numbers != nullptr) {
        int maxDoc = leaf->maxDoc();
        for (int i = 0; i < maxDoc; i++) {
          shared_ptr<Document> doc = leaf->document(i);
          int64_t value =
              static_cast<int64_t>(doc[L"text"]->split(L" ")[1]);
          TestUtil::assertEquals(i, numbers->nextDoc());
          int64_t dvValue = numbers->longValue();
          if (value == 0) {
            TestUtil::assertEquals(0, dvValue);
          } else {
            assertTrue(dvValue % value == 0);
            assertTrue(dvValue / value <= mergingSchemaGen->get());
          }
        }
      }
    }
  }
}

void TestDemoParallelLeafReader::testBasic() 
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<ReindexingReader> reindexer = getReindexer(tempPath);

  // Start with initial empty commit:
  reindexer->commit();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                              Field::Store::YES));
  reindexer->w->addDocument(doc);

  if (DEBUG) {
    wcout << L"TEST: refresh @ 1 doc" << endl;
  }
  reindexer->mgr->maybeRefresh();
  shared_ptr<DirectoryReader> r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  try {
    checkAllNumberDVs(r);
    shared_ptr<IndexSearcher> s = newSearcher(r);
    testNumericDVSort(s);
    testPointRangeQuery(s);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }
  // reindexer.printRefCounts();

  if (DEBUG) {
    wcout << L"TEST: commit" << endl;
  }
  reindexer->commit();

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                              Field::Store::YES));
  reindexer->w->addDocument(doc);

  if (DEBUG) {
    wcout << L"TEST: refresh @ 2 docs" << endl;
  }
  reindexer->mgr->maybeRefresh();
  // reindexer.printRefCounts();
  r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  try {
    checkAllNumberDVs(r);
    shared_ptr<IndexSearcher> s = newSearcher(r);
    testNumericDVSort(s);
    testPointRangeQuery(s);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }

  if (DEBUG) {
    wcout << L"TEST: forceMerge" << endl;
  }
  reindexer->w->forceMerge(1);

  if (DEBUG) {
    wcout << L"TEST: commit" << endl;
  }
  reindexer->commit();

  if (DEBUG) {
    wcout << L"TEST: refresh after forceMerge" << endl;
  }
  reindexer->mgr->maybeRefresh();
  r = reindexer->mgr->acquire();
  if (DEBUG) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  try {
    checkAllNumberDVs(r);
    shared_ptr<IndexSearcher> s = newSearcher(r);
    testNumericDVSort(s);
    testPointRangeQuery(s);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reindexer->mgr->release(r);
  }

  if (DEBUG) {
    wcout << L"TEST: close writer" << endl;
  }
  delete reindexer;
}

void TestDemoParallelLeafReader::testRandom() 
{
  shared_ptr<Path> root = createTempDir();
  shared_ptr<ReindexingReader> reindexer = nullptr;

  // TODO: separate refresh thread, search threads, indexing threads
  int numDocs = atLeast(TEST_NIGHTLY ? 20000 : 1000);
  int maxID = 0;
  int refreshEveryNumDocs = 100;
  int commitCloseNumDocs = 1000;
  for (int i = 0; i < numDocs; i++) {
    if (reindexer == nullptr) {
      reindexer = getReindexer(root);
    }

    shared_ptr<Document> doc = make_shared<Document>();
    wstring id;
    wstring updateID;
    if (maxID > 0 && random()->nextInt(10) == 7) {
      // Replace a doc
      id = L"" + random()->nextInt(maxID);
      updateID = id;
    } else {
      id = L"" + to_wstring(maxID++);
      updateID = L"";
    }

    doc->push_back(newStringField(L"id", id, Field::Store::NO));
    doc->push_back(newTextField(L"text", L"number " + random()->nextLong(),
                                Field::Store::YES));
    if (updateID == L"") {
      reindexer->w->addDocument(doc);
    } else {
      reindexer->w->updateDocument(make_shared<Term>(L"id", updateID), doc);
    }

    if (random()->nextInt(refreshEveryNumDocs) == 17) {
      if (DEBUG) {
        wcout << L"TEST: refresh @ " << (i << 1) << L" docs" << endl;
      }
      reindexer->mgr->maybeRefresh();
      shared_ptr<DirectoryReader> r = reindexer->mgr->acquire();
      if (DEBUG) {
        wcout << L"TEST: got reader=" << r << endl;
      }
      try {
        checkAllNumberDVs(r);
        shared_ptr<IndexSearcher> s = newSearcher(r);
        testNumericDVSort(s);
        testPointRangeQuery(s);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reindexer->mgr->release(r);
      }
      refreshEveryNumDocs = static_cast<int>(1.25 * refreshEveryNumDocs);
    }

    if (i > 0 && random()->nextInt(10) == 7) {
      // Random delete:
      reindexer->w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + random()->nextInt(i))});
    }

    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << L"TEST: commit @ " << (i << 1) << L" docs" << endl;
      }
      reindexer->commit();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }

    // Sometimes close & reopen writer/manager, to confirm the parallel segments
    // persist:
    if (random()->nextInt(commitCloseNumDocs) == 17) {
      if (DEBUG) {
        wcout << L"TEST: close writer @ " << (i << 1) << L" docs" << endl;
      }
      delete reindexer;
      reindexer.reset();
      commitCloseNumDocs = static_cast<int>(1.25 * commitCloseNumDocs);
    }
  }
  if (reindexer != nullptr) {
    delete reindexer;
  }
}

void TestDemoParallelLeafReader::checkAllNumberDVs(
    shared_ptr<IndexReader> r) 
{
  checkAllNumberDVs(r, L"number", true, 1);
}

void TestDemoParallelLeafReader::checkAllNumberDVs(
    shared_ptr<IndexReader> r, const wstring &fieldName, bool doThrow,
    int multiplier) 
{
  shared_ptr<NumericDocValues> numbers =
      MultiDocValues::getNumericValues(r, fieldName);
  int maxDoc = r->maxDoc();
  bool failed = false;
  int64_t t0 = System::currentTimeMillis();
  for (int i = 0; i < maxDoc; i++) {
    shared_ptr<Document> oldDoc = r->document(i);
    int64_t value =
        multiplier * static_cast<int64_t>(oldDoc[L"text"]->split(L" ")[1]);
    TestUtil::assertEquals(i, numbers->nextDoc());
    if (value != numbers->longValue()) {
      wcout << L"FAIL: docID=" << i << L" " << oldDoc << L" value=" << value
            << L" number=" << numbers->longValue() << L" numbers=" << numbers
            << endl;
      failed = true;
    } else if (failed) {
      wcout << L"OK: docID=" << i << L" " << oldDoc << L" value=" << value
            << L" number=" << numbers->longValue() << endl;
    }
  }
  if (failed) {
    if (std::dynamic_pointer_cast<LeafReader>(r) != nullptr == false) {
      wcout << L"TEST FAILED; check leaves" << endl;
      for (auto ctx : r->leaves()) {
        wcout << L"CHECK LEAF=" << ctx->reader() << endl;
        checkAllNumberDVs(ctx->reader(), fieldName, false, 1);
      }
    }
    if (doThrow) {
      assertFalse(L"FAILED field=" + fieldName + L" r=" + r, failed);
    } else {
      wcout << L"FAILED field=" << fieldName << L" r=" << r << endl;
    }
  }
}

void TestDemoParallelLeafReader::testNumericDVSort(
    shared_ptr<IndexSearcher> s) 
{
  // Confirm we can sort by the new DV field:
  shared_ptr<TopDocs> hits = s->search(make_shared<MatchAllDocsQuery>(), 100,
                                       make_shared<Sort>(make_shared<SortField>(
                                           L"number", SortField::Type::LONG)));
  int64_t last = numeric_limits<int64_t>::min();
  for (auto scoreDoc : hits->scoreDocs) {
    int64_t value =
        static_cast<int64_t>(s->doc(scoreDoc->doc)[L"text"]->split(L" ")[1]);
    assertTrue(value >= last);
    TestUtil::assertEquals(
        value, (any_cast<optional<int64_t>>(
                    (std::static_pointer_cast<FieldDoc>(scoreDoc))->fields[0]))
                   .value());
    last = value;
  }
}

void TestDemoParallelLeafReader::testPointRangeQuery(
    shared_ptr<IndexSearcher> s) 
{
  for (int i = 0; i < 100; i++) {
    // Confirm we can range search by the new indexed (numeric) field:
    int64_t min = random()->nextLong();
    int64_t max = random()->nextLong();
    if (min > max) {
      int64_t x = min;
      min = max;
      max = x;
    }

    shared_ptr<TopDocs> hits =
        s->search(LongPoint::newRangeQuery(L"number", min, max), 100);
    for (auto scoreDoc : hits->scoreDocs) {
      int64_t value = static_cast<int64_t>(
          s->doc(scoreDoc->doc)[L"text"]->split(L" ")[1]);
      assertTrue(value >= min);
      assertTrue(value <= max);
    }

    Arrays::sort(hits->scoreDocs, make_shared<ComparatorAnonymousInnerClass>());

    shared_ptr<NumericDocValues> numbers =
        MultiDocValues::getNumericValues(s->getIndexReader(), L"number");
    for (auto hit : hits->scoreDocs) {
      if (numbers->docID() < hit->doc) {
        numbers->advance(hit->doc);
      }
      TestUtil::assertEquals(hit->doc, numbers->docID());
      int64_t value =
          static_cast<int64_t>(s->doc(hit->doc)[L"text"]->split(L" ")[1]);
      TestUtil::assertEquals(value, numbers->longValue());
    }
  }
}

TestDemoParallelLeafReader::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int TestDemoParallelLeafReader::ComparatorAnonymousInnerClass::compare(
    shared_ptr<ScoreDoc> a, shared_ptr<ScoreDoc> b)
{
  return a->doc - b->doc;
}

const shared_ptr<java::util::regex::Pattern>
    TestDemoParallelLeafReader::SEG_GEN_SUB_DIR_PATTERN =
        java::util::regex::Pattern::compile(L"^[a-z0-9]+_([0-9]+)$");

deque<std::shared_ptr<Path>> TestDemoParallelLeafReader::segSubDirs(
    shared_ptr<Path> segsPath) 
{
  deque<std::shared_ptr<Path>> result = deque<std::shared_ptr<Path>>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(segsPath))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(segsPath);
    for (auto path : stream) {
      // Must be form <segIDString>_<longGen>
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (Files::isDirectory(path) &&
          SEG_GEN_SUB_DIR_PATTERN->matcher(path->getFileName()->toString())
              .matches()) {
        result.push_back(path);
      }
    }
  }

  return result;
}
} // namespace org::apache::lucene::index