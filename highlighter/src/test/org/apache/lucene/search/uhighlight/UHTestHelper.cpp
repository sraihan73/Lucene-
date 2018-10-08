using namespace std;

#include "UHTestHelper.h"

namespace org::apache::lucene::search::uhighlight
{
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
const shared_ptr<org::apache::lucene::document::FieldType>
    UHTestHelper::postingsType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    UHTestHelper::tvType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    UHTestHelper::postingsWithTvType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    UHTestHelper::reanalysisType =
        org::apache::lucene::document::TextField::TYPE_STORED;

UHTestHelper::StaticConstructor::StaticConstructor()
{
  postingsType->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  postingsType->freeze();

  tvType->setStoreTermVectors(true);
  tvType->setStoreTermVectorPositions(true);
  tvType->setStoreTermVectorOffsets(true);
  tvType->freeze();

  postingsWithTvType->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  postingsWithTvType->setStoreTermVectors(true);
  postingsWithTvType->freeze();

  // re-analysis type needs no further changes.
}

UHTestHelper::StaticConstructor UHTestHelper::staticConstructor;

shared_ptr<FieldType>
UHTestHelper::randomFieldType(shared_ptr<Random> random,
                              deque<FieldType> &typePossibilities)
{
  if (typePossibilities == nullptr || typePossibilities->length == 0) {
    typePossibilities = std::deque<std::shared_ptr<FieldType>>{
        postingsType, tvType, postingsWithTvType, reanalysisType};
  }
  return typePossibilities[random->nextInt(typePossibilities->length)];
}

deque<std::deque<any>> UHTestHelper::parametersFactoryList()
{
  return Arrays::asList(std::deque<std::deque<any>>{
      std::deque<any>{postingsType}, std::deque<any>{tvType},
      std::deque<any>{postingsWithTvType}, std::deque<any>{reanalysisType}});
}
} // namespace org::apache::lucene::search::uhighlight