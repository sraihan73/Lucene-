using namespace std;

#include "SegmentDocValues.h"

namespace org::apache::lucene::index
{
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IOUtils = org::apache::lucene::util::IOUtils;
using RefCount = org::apache::lucene::util::RefCount;

shared_ptr<RefCount<std::shared_ptr<DocValuesProducer>>>
SegmentDocValues::newDocValuesProducer(
    shared_ptr<SegmentCommitInfo> si, shared_ptr<Directory> dir,
    optional<int64_t> &gen, shared_ptr<FieldInfos> infos) 
{
  shared_ptr<Directory> dvDir = dir;
  wstring segmentSuffix = L"";
  if (gen.value() != -1) {
    dvDir = si->info->dir; // gen'd files are written outside CFS, so use
                           // SegInfo directory
    // C++ TODO: There is no native C++ equivalent to 'toString':
    segmentSuffix = Long::toString(gen.value(), Character::MAX_RADIX);
  }

  // set SegmentReadState to deque only the fields that are relevant to that gen
  shared_ptr<SegmentReadState> srs = make_shared<SegmentReadState>(
      dvDir, si->info, infos, IOContext::READ, segmentSuffix);
  shared_ptr<DocValuesFormat> dvFormat =
      si->info->getCodec()->docValuesFormat();
  return make_shared<RefCountAnonymousInnerClass>(
      shared_from_this(), dvFormat->fieldsProducer(srs), gen);
}

SegmentDocValues::RefCountAnonymousInnerClass::RefCountAnonymousInnerClass(
    shared_ptr<SegmentDocValues> outerInstance,
    shared_ptr<DocValuesProducer> fieldsProducer, optional<int64_t> &gen)
    : org::apache::lucene::util::RefCount<
          org::apache::lucene::codecs::DocValuesProducer>(fieldsProducer)
{
  this->outerInstance = outerInstance;
  this->gen = gen;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override protected void
// release() throws java.io.IOException
void SegmentDocValues::RefCountAnonymousInnerClass::release() 
{
  object::close();
  {
    lock_guard<mutex> lock(outerInstance);
    outerInstance->genDVProducers.erase(gen);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocValuesProducer> SegmentDocValues::getDocValuesProducer(
    int64_t gen, shared_ptr<SegmentCommitInfo> si, shared_ptr<Directory> dir,
    shared_ptr<FieldInfos> infos) 
{
  shared_ptr<RefCount<std::shared_ptr<DocValuesProducer>>> dvp =
      genDVProducers[gen];
  if (dvp == nullptr) {
    dvp = newDocValuesProducer(si, dir, gen, infos);
    assert(dvp != nullptr);
    genDVProducers.emplace(gen, dvp);
  } else {
    dvp->incRef();
  }
  return dvp->get();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SegmentDocValues::decRef(deque<int64_t> &dvProducersGens) throw(
    IOException)
{
  IOUtils::applyToAll(dvProducersGens, [&](any gen) {
    shared_ptr<RefCount<std::shared_ptr<DocValuesProducer>>> dvp =
        genDVProducers[gen];
    assert((dvp != nullptr, L"gen=" + gen));
    dvp->decRef();
  });
}
} // namespace org::apache::lucene::index