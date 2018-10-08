using namespace std;

#include "MultiPassIndexSplitter.h"

namespace org::apache::lucene::index
{
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using Bits = org::apache::lucene::util::Bits;
using org::apache::lucene::util::SuppressForbidden;

void MultiPassIndexSplitter::split(
    shared_ptr<IndexReader> in_,
    std::deque<std::shared_ptr<Directory>> &outputs,
    bool seq) 
{
  if (outputs.empty() || outputs.size() < 2) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Invalid number of outputs.");
  }
  if (in_ == nullptr || in_->numDocs() < 2) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Not enough documents for splitting");
  }
  int numParts = outputs.size();
  // wrap a potentially read-only input
  // this way we don't have to preserve original deletions because neither
  // deleteDocument(int) or undeleteAll() is applied to the wrapped input index.
  shared_ptr<FakeDeleteIndexReader> input =
      make_shared<FakeDeleteIndexReader>(in_);
  int maxDoc = input->maxDoc();
  int partLen = maxDoc / numParts;
  for (int i = 0; i < numParts; i++) {
    input->undeleteAll();
    if (seq) { // sequential range
      int lo = partLen * i;
      int hi = lo + partLen;
      // below range
      for (int j = 0; j < lo; j++) {
        input->deleteDocument(j);
      }
      // above range - last part collects all id-s that remained due to
      // integer rounding errors
      if (i < numParts - 1) {
        for (int j = hi; j < maxDoc; j++) {
          input->deleteDocument(j);
        }
      }
    } else {
      // round-robin
      for (int j = 0; j < maxDoc; j++) {
        if ((j + numParts - i) % numParts != 0) {
          input->deleteDocument(j);
        }
      }
    }
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        outputs[i],
        make_shared<IndexWriterConfig>(nullptr).setOpenMode(OpenMode::CREATE));
    System::err::println(L"Writing part " + to_wstring(i + 1) + L" ...");
    // pass the subreaders directly, as our wrapper's numDocs/hasDeletetions are
    // not up-to-date
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.List<? extends FakeDeleteLeafIndexReader>
    // sr = input.getSequentialSubReaders();
    const deque < ? extends FakeDeleteLeafIndexReader > sr =
                         input->getSequentialSubReaders();
    w->addIndexes({sr.toArray(std::deque<std::shared_ptr<CodecReader>>(
        sr.size()))}); // TODO: maybe take List<IR> here?
    delete w;
  }
  System::err::println(L"Done.");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("deprecation") public static void
// main(std::wstring[] args) throws Exception
void MultiPassIndexSplitter::main(std::deque<wstring> &args) throw(
    runtime_error)
{
  if (args.size() < 5) {
    System::err::println(L"Usage: MultiPassIndexSplitter -out <outputDir> -num "
                         L"<numParts> [-seq] <inputIndex1> [<inputIndex2 ...]");
    System::err::println(
        L"\tinputIndex\tpath to input index, multiple values are ok");
    System::err::println(L"\t-out ouputDir\tpath to output directory to "
                         L"contain partial indexes");
    System::err::println(L"\t-num numParts\tnumber of parts to produce");
    System::err::println(
        L"\t-seq\tsequential docid-range split (default is round-robin)");
    exit(-1);
  }
  deque<std::shared_ptr<IndexReader>> indexes =
      deque<std::shared_ptr<IndexReader>>();
  wstring outDir = L"";
  int numParts = -1;
  bool seq = false;
  for (int i = 0; i < args.size(); i++) {
    if (args[i] == L"-out") {
      outDir = args[++i];
    } else if (args[i] == L"-num") {
      numParts = stoi(args[++i]);
    } else if (args[i] == L"-seq") {
      seq = true;
    } else {
      shared_ptr<Path> file = Paths->get(args[i]);
      if (!Files::isDirectory(file)) {
        System::err::println(L"Invalid input path - skipping: " + file);
        continue;
      }
      shared_ptr<Directory> dir = FSDirectory::open(file);
      try {
        if (!DirectoryReader::indexExists(dir)) {
          System::err::println(L"Invalid input index - skipping: " + file);
          continue;
        }
      } catch (const runtime_error &e) {
        System::err::println(L"Invalid input index - skipping: " + file);
        continue;
      }
      indexes.push_back(DirectoryReader::open(dir));
    }
  }
  if (outDir == L"") {
    throw runtime_error(L"Required argument missing: -out outputDir");
  }
  if (numParts < 2) {
    throw runtime_error(L"Invalid value of required argument: -num numParts");
  }
  if (indexes.empty()) {
    throw runtime_error(L"No input indexes to process");
  }
  shared_ptr<Path> out = Paths->get(outDir);
  Files::createDirectories(out);
  std::deque<std::shared_ptr<Directory>> dirs(numParts);
  for (int i = 0; i < numParts; i++) {
    dirs[i] = FSDirectory::open(out->resolve(L"part-" + to_wstring(i)));
  }
  shared_ptr<MultiPassIndexSplitter> splitter =
      make_shared<MultiPassIndexSplitter>();
  shared_ptr<IndexReader> input;
  if (indexes.size() == 1) {
    input = indexes[0];
  } else {
    input = make_shared<MultiReader>(indexes.toArray(
        std::deque<std::shared_ptr<IndexReader>>(indexes.size())));
  }
  splitter->split(input, dirs, seq);
}

MultiPassIndexSplitter::FakeDeleteIndexReader::FakeDeleteIndexReader(
    shared_ptr<IndexReader> reader) 
    : BaseCompositeReader<FakeDeleteLeafIndexReader>(initSubReaders(reader))
{
}

std::deque<std::shared_ptr<FakeDeleteLeafIndexReader>>
MultiPassIndexSplitter::FakeDeleteIndexReader::initSubReaders(
    shared_ptr<IndexReader> reader) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  std::deque<std::shared_ptr<FakeDeleteLeafIndexReader>> subs(leaves.size());
  int i = 0;
  for (auto ctx : leaves) {
    subs[i++] = make_shared<FakeDeleteLeafIndexReader>(
        SlowCodecReaderWrapper::wrap(ctx->reader()));
  }
  return subs;
}

void MultiPassIndexSplitter::FakeDeleteIndexReader::deleteDocument(int docID)
{
  constexpr int i = readerIndex(docID);
  getSequentialSubReaders()[i]->deleteDocument(docID - readerBase(i));
}

void MultiPassIndexSplitter::FakeDeleteIndexReader::undeleteAll()
{
  for (auto r : getSequentialSubReaders()) {
    r->undeleteAll();
  }
}

void MultiPassIndexSplitter::FakeDeleteIndexReader::doClose() {}

shared_ptr<CacheHelper>
MultiPassIndexSplitter::FakeDeleteIndexReader::getReaderCacheHelper()
{
  return nullptr;
}

MultiPassIndexSplitter::FakeDeleteLeafIndexReader::FakeDeleteLeafIndexReader(
    shared_ptr<CodecReader> reader)
    : FilterCodecReader(reader)
{
  undeleteAll(); // initialize main bitset
}

int MultiPassIndexSplitter::FakeDeleteLeafIndexReader::numDocs()
{
  return liveDocs->cardinality();
}

void MultiPassIndexSplitter::FakeDeleteLeafIndexReader::undeleteAll()
{
  constexpr int maxDoc = in_->maxDoc();
  liveDocs = make_shared<FixedBitSet>(in_->maxDoc());
  if (in_->hasDeletions()) {
    shared_ptr<Bits> *const oldLiveDocs = in_->getLiveDocs();
    assert(oldLiveDocs != nullptr);
    // this loop is a little bit ineffective, as Bits has no nextSetBit():
    for (int i = 0; i < maxDoc; i++) {
      if (oldLiveDocs->get(i)) {
        liveDocs->set(i);
      }
    }
  } else {
    // mark all docs as valid
    liveDocs->set(0, maxDoc);
  }
}

void MultiPassIndexSplitter::FakeDeleteLeafIndexReader::deleteDocument(int n)
{
  liveDocs->clear(n);
}

shared_ptr<Bits>
MultiPassIndexSplitter::FakeDeleteLeafIndexReader::getLiveDocs()
{
  return liveDocs;
}

shared_ptr<CacheHelper>
MultiPassIndexSplitter::FakeDeleteLeafIndexReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper>
MultiPassIndexSplitter::FakeDeleteLeafIndexReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index