using namespace std;

#include "PKIndexSplitter.h"

namespace org::apache::lucene::index
{
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;

PKIndexSplitter::PKIndexSplitter(shared_ptr<Directory> input,
                                 shared_ptr<Directory> dir1,
                                 shared_ptr<Directory> dir2,
                                 shared_ptr<Query> docsInFirstIndex)
    : PKIndexSplitter(input, dir1, dir2, docsInFirstIndex, newDefaultConfig(),
                      newDefaultConfig())
{
}

shared_ptr<IndexWriterConfig> PKIndexSplitter::newDefaultConfig()
{
  return (make_shared<IndexWriterConfig>(nullptr))
      ->setOpenMode(OpenMode::CREATE);
}

PKIndexSplitter::PKIndexSplitter(shared_ptr<Directory> input,
                                 shared_ptr<Directory> dir1,
                                 shared_ptr<Directory> dir2,
                                 shared_ptr<Query> docsInFirstIndex,
                                 shared_ptr<IndexWriterConfig> config1,
                                 shared_ptr<IndexWriterConfig> config2)
    : docsInFirstIndex(docsInFirstIndex), input(input), dir1(dir1), dir2(dir2),
      config1(config1), config2(config2)
{
}

PKIndexSplitter::PKIndexSplitter(shared_ptr<Directory> input,
                                 shared_ptr<Directory> dir1,
                                 shared_ptr<Directory> dir2,
                                 shared_ptr<Term> midTerm)
    : PKIndexSplitter(input, dir1, dir2,
                      new TermRangeQuery(midTerm->field(), nullptr,
                                         midTerm->bytes(), true, false))
{
}

PKIndexSplitter::PKIndexSplitter(shared_ptr<Directory> input,
                                 shared_ptr<Directory> dir1,
                                 shared_ptr<Directory> dir2,
                                 shared_ptr<Term> midTerm,
                                 shared_ptr<IndexWriterConfig> config1,
                                 shared_ptr<IndexWriterConfig> config2)
    : PKIndexSplitter(input, dir1, dir2,
                      new TermRangeQuery(midTerm->field(), nullptr,
                                         midTerm->bytes(), true, false),
                      config1, config2)
{
}

void PKIndexSplitter::split() 
{
  bool success = false;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(input);
  try {
    // pass an individual config in here since one config can not be reused!
    createIndex(config1, dir1, reader, docsInFirstIndex, false);
    createIndex(config2, dir2, reader, docsInFirstIndex, true);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({reader});
    } else {
      IOUtils::closeWhileHandlingException({reader});
    }
  }
}

void PKIndexSplitter::createIndex(shared_ptr<IndexWriterConfig> config,
                                  shared_ptr<Directory> target,
                                  shared_ptr<DirectoryReader> reader,
                                  shared_ptr<Query> preserveFilter,
                                  bool negateFilter) 
{
  bool success = false;
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(target, config);
  try {
    shared_ptr<IndexSearcher> *const searcher =
        make_shared<IndexSearcher>(reader);
    searcher->setQueryCache(nullptr);
    preserveFilter = searcher->rewrite(preserveFilter);
    shared_ptr<Weight> *const preserveWeight =
        searcher->createWeight(preserveFilter, false, 1);
    const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    std::deque<std::shared_ptr<CodecReader>> subReaders(leaves.size());
    int i = 0;
    for (auto ctx : leaves) {
      subReaders[i++] = make_shared<DocumentFilteredLeafIndexReader>(
          ctx, preserveWeight, negateFilter);
    }
    w->addIndexes(subReaders);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      delete w;
    } else {
      IOUtils::closeWhileHandlingException({w});
    }
  }
}

PKIndexSplitter::DocumentFilteredLeafIndexReader::
    DocumentFilteredLeafIndexReader(shared_ptr<LeafReaderContext> context,
                                    shared_ptr<Weight> preserveWeight,
                                    bool negateFilter) 
    : FilterCodecReader((CodecReader)context->reader()), liveDocs(bits),
      numDocs(bits::cardinality())
{
  // our cast is ok, since we open the Directory.
  constexpr int maxDoc = in_->maxDoc();
  shared_ptr<FixedBitSet> *const bits = make_shared<FixedBitSet>(maxDoc);
  // ignore livedocs here, as we filter them later:
  shared_ptr<Scorer> *const preverveScorer = preserveWeight->scorer(context);
  if (preverveScorer != nullptr) {
    bits->or (preverveScorer->begin());
  }
  if (negateFilter) {
    bits->flip(0, maxDoc);
  }

  if (in_->hasDeletions()) {
    shared_ptr<Bits> *const oldLiveDocs = in_->getLiveDocs();
    assert(oldLiveDocs != nullptr);
    shared_ptr<DocIdSetIterator> *const it =
        make_shared<BitSetIterator>(bits, 0LL); // the cost is not useful here
    for (int i = it->nextDoc(); i != DocIdSetIterator::NO_MORE_DOCS;
         i = it->nextDoc()) {
      if (!oldLiveDocs->get(i)) {
        // we can safely modify the current bit, as the iterator already stepped
        // over it:
        bits->clear(i);
      }
    }
  }
}

int PKIndexSplitter::DocumentFilteredLeafIndexReader::numDocs()
{
  return numDocs_;
}

shared_ptr<Bits> PKIndexSplitter::DocumentFilteredLeafIndexReader::getLiveDocs()
{
  return liveDocs;
}

shared_ptr<CacheHelper>
PKIndexSplitter::DocumentFilteredLeafIndexReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper>
PKIndexSplitter::DocumentFilteredLeafIndexReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index