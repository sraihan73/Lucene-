using namespace std;

#include "TaxonomyMergeUtils.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using OrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::OrdinalMap;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using CodecReader = org::apache::lucene::index::CodecReader;
using SlowCodecReaderWrapper =
    org::apache::lucene::index::SlowCodecReaderWrapper;
using Directory = org::apache::lucene::store::Directory;

TaxonomyMergeUtils::TaxonomyMergeUtils() {}

void TaxonomyMergeUtils::merge(
    shared_ptr<Directory> srcIndexDir, shared_ptr<Directory> srcTaxoDir,
    shared_ptr<DirectoryTaxonomyWriter::OrdinalMap> map_obj,
    shared_ptr<IndexWriter> destIndexWriter,
    shared_ptr<DirectoryTaxonomyWriter> destTaxoWriter,
    shared_ptr<FacetsConfig> srcConfig) 
{

  // merge the taxonomies
  destTaxoWriter->addTaxonomy(srcTaxoDir, map_obj);
  std::deque<int> ordinalMap = map_obj->getMap();
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(srcIndexDir);
  try {
    deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    int numReaders = leaves.size();
    std::deque<std::shared_ptr<CodecReader>> wrappedLeaves(numReaders);
    for (int i = 0; i < numReaders; i++) {
      wrappedLeaves[i] =
          SlowCodecReaderWrapper::wrap(make_shared<OrdinalMappingLeafReader>(
              leaves[i]->reader(), ordinalMap, srcConfig));
    }
    destIndexWriter->addIndexes(wrappedLeaves);

    // commit changes to taxonomy and index respectively.
    destTaxoWriter->commit();
    destIndexWriter->commit();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reader->close();
  }
}
} // namespace org::apache::lucene::facet::taxonomy