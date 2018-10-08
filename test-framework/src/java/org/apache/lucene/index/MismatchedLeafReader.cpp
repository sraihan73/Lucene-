using namespace std;

#include "MismatchedLeafReader.h"

namespace org::apache::lucene::index
{

MismatchedLeafReader::MismatchedLeafReader(shared_ptr<LeafReader> in_,
                                           shared_ptr<Random> random)
    : FilterLeafReader(in_),
      shuffled(shuffleInfos(in_->getFieldInfos(), random))
{
}

shared_ptr<FieldInfos> MismatchedLeafReader::getFieldInfos()
{
  return shuffled;
}

void MismatchedLeafReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  in_->document(docID,
                make_shared<MismatchedVisitor>(shared_from_this(), visitor));
}

shared_ptr<CacheHelper> MismatchedLeafReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper> MismatchedLeafReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

shared_ptr<FieldInfos>
MismatchedLeafReader::shuffleInfos(shared_ptr<FieldInfos> infos,
                                   shared_ptr<Random> random)
{
  // first, shuffle the order
  deque<std::shared_ptr<FieldInfo>> shuffled =
      deque<std::shared_ptr<FieldInfo>>();
  for (auto info : infos) {
    shuffled.push_back(info);
  }
  Collections::shuffle(shuffled, random);

  // now renumber:
  for (int i = 0; i < shuffled.size(); i++) {
    shared_ptr<FieldInfo> oldInfo = shuffled[i];
    // TODO: should we introduce "gaps" too?
    shared_ptr<FieldInfo> newInfo = make_shared<FieldInfo>(
        oldInfo->name, i, oldInfo->hasVectors(), oldInfo->omitsNorms(),
        oldInfo->hasPayloads(), oldInfo->getIndexOptions(),
        oldInfo->getDocValuesType(), oldInfo->getDocValuesGen(),
        oldInfo->attributes(), oldInfo->getPointDimensionCount(),
        oldInfo->getPointNumBytes(),
        oldInfo->isSoftDeletesField()); // used as soft-deletes field
    shuffled[i] = newInfo;
  }

  return make_shared<FieldInfos>(shuffled.toArray(
      std::deque<std::shared_ptr<FieldInfo>>(shuffled.size())));
}

MismatchedLeafReader::MismatchedVisitor::MismatchedVisitor(
    shared_ptr<MismatchedLeafReader> outerInstance,
    shared_ptr<StoredFieldVisitor> in_)
    : in_(in_), outerInstance(outerInstance)
{
}

void MismatchedLeafReader::MismatchedVisitor::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  in_->binaryField(renumber(fieldInfo), value);
}

void MismatchedLeafReader::MismatchedVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  in_->stringField(renumber(fieldInfo), value);
}

void MismatchedLeafReader::MismatchedVisitor::intField(
    shared_ptr<FieldInfo> fieldInfo, int value) 
{
  in_->intField(renumber(fieldInfo), value);
}

void MismatchedLeafReader::MismatchedVisitor::longField(
    shared_ptr<FieldInfo> fieldInfo, int64_t value) 
{
  in_->longField(renumber(fieldInfo), value);
}

void MismatchedLeafReader::MismatchedVisitor::floatField(
    shared_ptr<FieldInfo> fieldInfo, float value) 
{
  in_->floatField(renumber(fieldInfo), value);
}

void MismatchedLeafReader::MismatchedVisitor::doubleField(
    shared_ptr<FieldInfo> fieldInfo, double value) 
{
  in_->doubleField(renumber(fieldInfo), value);
}

Status MismatchedLeafReader::MismatchedVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  return in_->needsField(renumber(fieldInfo));
}

shared_ptr<FieldInfo> MismatchedLeafReader::MismatchedVisitor::renumber(
    shared_ptr<FieldInfo> original)
{
  shared_ptr<FieldInfo> renumbered =
      outerInstance->shuffled->fieldInfo(original->name);
  if (renumbered == nullptr) {
    throw make_shared<AssertionError>(L"stored fields sending bogus infos!");
  }
  return renumbered;
}
} // namespace org::apache::lucene::index