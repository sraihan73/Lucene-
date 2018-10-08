using namespace std;

#include "SegmentDocValuesProducer.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

SegmentDocValuesProducer::SegmentDocValuesProducer(
    shared_ptr<SegmentCommitInfo> si, shared_ptr<Directory> dir,
    shared_ptr<FieldInfos> coreInfos, shared_ptr<FieldInfos> allInfos,
    shared_ptr<SegmentDocValues> segDocValues) 
{
  try {
    shared_ptr<DocValuesProducer> baseProducer = nullptr;
    for (auto fi : allInfos) {
      if (fi->getDocValuesType() == DocValuesType::NONE) {
        continue;
      }
      int64_t docValuesGen = fi->getDocValuesGen();
      if (docValuesGen == -1) {
        if (baseProducer == nullptr) {
          // the base producer gets the original fieldinfos it wrote
          baseProducer = segDocValues->getDocValuesProducer(docValuesGen, si,
                                                            dir, coreInfos);
          dvGens.push_back(docValuesGen);
          dvProducers->add(baseProducer);
        }
        dvProducersByField.emplace(fi->name, baseProducer);
      } else {
        assert((!find(dvGens.begin(), dvGens.end(), docValuesGen) !=
                dvGens.end()));
        // otherwise, producer sees only the one fieldinfo it wrote
        shared_ptr<DocValuesProducer> *const dvp =
            segDocValues->getDocValuesProducer(
                docValuesGen, si, dir,
                make_shared<FieldInfos>(
                    std::deque<std::shared_ptr<FieldInfo>>{fi}));
        dvGens.push_back(docValuesGen);
        dvProducers->add(dvp);
        dvProducersByField.emplace(fi->name, dvp);
      }
    }
  } catch (const runtime_error &t) {
    try {
      segDocValues->decRef(dvGens);
    } catch (const runtime_error &t1) {
      t.addSuppressed(t1);
    }
    throw t;
  }
}

shared_ptr<NumericDocValues> SegmentDocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> dvProducer = dvProducersByField[field->name];
  assert(dvProducer != nullptr);
  return dvProducer->getNumeric(field);
}

shared_ptr<BinaryDocValues> SegmentDocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> dvProducer = dvProducersByField[field->name];
  assert(dvProducer != nullptr);
  return dvProducer->getBinary(field);
}

shared_ptr<SortedDocValues> SegmentDocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> dvProducer = dvProducersByField[field->name];
  assert(dvProducer != nullptr);
  return dvProducer->getSorted(field);
}

shared_ptr<SortedNumericDocValues> SegmentDocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> dvProducer = dvProducersByField[field->name];
  assert(dvProducer != nullptr);
  return dvProducer->getSortedNumeric(field);
}

shared_ptr<SortedSetDocValues> SegmentDocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> dvProducer = dvProducersByField[field->name];
  assert(dvProducer != nullptr);
  return dvProducer->getSortedSet(field);
}

void SegmentDocValuesProducer::checkIntegrity() 
{
  for (auto producer : dvProducers) {
    producer->checkIntegrity();
  }
}

SegmentDocValuesProducer::~SegmentDocValuesProducer()
{
  throw make_shared<UnsupportedOperationException>(); // there is separate ref
                                                      // tracking
}

int64_t SegmentDocValuesProducer::ramBytesUsed()
{
  int64_t ramBytesUsed = BASE_RAM_BYTES_USED;
  ramBytesUsed += dvGens.size() * LONG_RAM_BYTES_USED;
  ramBytesUsed += dvProducers->size() * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  ramBytesUsed +=
      dvProducersByField.size() * 2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  for (auto producer : dvProducers) {
    ramBytesUsed += producer->ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
SegmentDocValuesProducer::getChildResources()
{
  const deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>(dvProducers->size());
  for (auto producer : dvProducers) {
    resources.push_back(Accountables::namedAccountable(L"delegate", producer));
  }
  return Collections::unmodifiableList(resources);
}

wstring SegmentDocValuesProducer::toString()
{
  return getClass().getSimpleName() + L"(producers=" + dvProducers->size() +
         L")";
}
} // namespace org::apache::lucene::index