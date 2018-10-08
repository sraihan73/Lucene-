using namespace std;

#include "TopDocs.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

float TopDocs::getMaxScore() { return maxScore; }

void TopDocs::setMaxScore(float maxScore) { this->maxScore = maxScore; }

TopDocs::TopDocs(int64_t totalHits,
                 std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs)
    : TopDocs(totalHits, scoreDocs, Float::NaN)
{
}

TopDocs::TopDocs(int64_t totalHits,
                 std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs,
                 float maxScore)
{
  this->totalHits = totalHits;
  this->scoreDocs = scoreDocs;
  this->maxScore = maxScore;
}

TopDocs::ShardRef::ShardRef(int shardIndex, bool useScoreDocIndex)
    : shardIndex(shardIndex), useScoreDocIndex(useScoreDocIndex)
{
}

wstring TopDocs::ShardRef::toString()
{
  return L"ShardRef(shardIndex=" + to_wstring(shardIndex) + L" hitIndex=" +
         to_wstring(hitIndex) + L")";
}

int TopDocs::ShardRef::getShardIndex(shared_ptr<ScoreDoc> scoreDoc)
{
  if (useScoreDocIndex) {
    if (scoreDoc->shardIndex == -1) {
      throw invalid_argument(L"setShardIndex is false but TopDocs[" +
                             to_wstring(shardIndex) + L"].scoreDocs[" +
                             to_wstring(hitIndex) + L"] is not set");
    }
    return scoreDoc->shardIndex;
  } else {
    // NOTE: we don't assert that shardIndex is -1 here, because caller could in
    // fact have set it but asked us to ignore it now
    return shardIndex;
  }
}

bool TopDocs::tieBreakLessThan(shared_ptr<ShardRef> first,
                               shared_ptr<ScoreDoc> firstDoc,
                               shared_ptr<ShardRef> second,
                               shared_ptr<ScoreDoc> secondDoc)
{
  constexpr int firstShardIndex = first->getShardIndex(firstDoc);
  constexpr int secondShardIndex = second->getShardIndex(secondDoc);
  // Tie break: earlier shard wins
  if (firstShardIndex < secondShardIndex) {
    return true;
  } else if (firstShardIndex > secondShardIndex) {
    return false;
  } else {
    // Tie break in same shard: resolve however the
    // shard had resolved it:
    assert(first->hitIndex != second->hitIndex);
    return first->hitIndex < second->hitIndex;
  }
}

TopDocs::ScoreMergeSortQueue::ScoreMergeSortQueue(
    std::deque<std::shared_ptr<TopDocs>> &shardHits)
    : org::apache::lucene::util::PriorityQueue<ShardRef>(shardHits.length),
      shardHits(
          std::deque<std::deque<std::shared_ptr<ScoreDoc>>>(shardHits.size()))
{
  for (int shardIDX = 0; shardIDX < shardHits.size(); shardIDX++) {
    this->shardHits[shardIDX] = shardHits[shardIDX]->scoreDocs;
  }
}

bool TopDocs::ScoreMergeSortQueue::lessThan(shared_ptr<ShardRef> first,
                                            shared_ptr<ShardRef> second)
{
  assert(first != second);
  shared_ptr<ScoreDoc> firstScoreDoc =
      shardHits[first->shardIndex][first->hitIndex];
  shared_ptr<ScoreDoc> secondScoreDoc =
      shardHits[second->shardIndex][second->hitIndex];
  if (firstScoreDoc->score < secondScoreDoc->score) {
    return false;
  } else if (firstScoreDoc->score > secondScoreDoc->score) {
    return true;
  } else {
    return tieBreakLessThan(first, firstScoreDoc, second, secondScoreDoc);
  }
}

TopDocs::MergeSortQueue::MergeSortQueue(
    shared_ptr<Sort> sort, std::deque<std::shared_ptr<TopDocs>> &shardHits)
    : org::apache::lucene::util::PriorityQueue<ShardRef>(shardHits.length),
      shardHits(std::deque<std::deque<std::shared_ptr<ScoreDoc>>>(
          shardHits.size())),
      comparators(
          std::deque<std::shared_ptr<FieldComparator>>(sortFields->length)),
      reverseMul(std::deque<int>(sortFields->length))
{
  for (int shardIDX = 0; shardIDX < shardHits.size(); shardIDX++) {
    std::deque<std::shared_ptr<ScoreDoc>> shard =
        shardHits[shardIDX]->scoreDocs;
    // System.out.println("  init shardIdx=" + shardIDX + " hits=" + shard);
    if (shard.size() > 0) {
      this->shardHits[shardIDX] = shard;
      // Fail gracefully if API is misused:
      for (int hitIDX = 0; hitIDX < shard.size(); hitIDX++) {
        shared_ptr<ScoreDoc> *const sd = shard[hitIDX];
        if (!(std::dynamic_pointer_cast<FieldDoc>(sd) != nullptr)) {
          throw invalid_argument(L"shard " + to_wstring(shardIDX) +
                                 L" was not sorted by the provided Sort "
                                 L"(expected FieldDoc but got ScoreDoc)");
        }
        shared_ptr<FieldDoc> *const fd = std::static_pointer_cast<FieldDoc>(sd);
        if (fd->fields.empty()) {
          throw invalid_argument(
              L"shard " + to_wstring(shardIDX) +
              L" did not set sort field values (FieldDoc.fields is null); you "
              L"must pass fillFields=true to IndexSearcher.search on each "
              L"shard");
        }
      }
    }
  }

  std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
  for (int compIDX = 0; compIDX < sortFields.size(); compIDX++) {
    shared_ptr<SortField> *const sortField = sortFields[compIDX];
    comparators[compIDX] = sortField->getComparator(1, compIDX);
    reverseMul[compIDX] = sortField->getReverse() ? -1 : 1;
  }
}

bool TopDocs::MergeSortQueue::lessThan(shared_ptr<ShardRef> first,
                                       shared_ptr<ShardRef> second)
{
  assert(first != second);
  shared_ptr<FieldDoc> *const firstFD = std::static_pointer_cast<FieldDoc>(
      shardHits[first->shardIndex][first->hitIndex]);
  shared_ptr<FieldDoc> *const secondFD = std::static_pointer_cast<FieldDoc>(
      shardHits[second->shardIndex][second->hitIndex]);
  // System.out.println("  lessThan:\n     first=" + first + " doc=" +
  // firstFD.doc + " score=" + firstFD.score + "\n    second=" + second + " doc="
  // + secondFD.doc + " score=" + secondFD.score);

  for (int compIDX = 0; compIDX < comparators.size(); compIDX++) {
    shared_ptr<FieldComparator> *const comp = comparators[compIDX];
    // System.out.println("    cmp idx=" + compIDX + " cmp1=" +
    // firstFD.fields[compIDX] + " cmp2=" + secondFD.fields[compIDX] + "
    // reverse=" + reverseMul[compIDX]);

    constexpr int cmp =
        reverseMul[compIDX] * comp->compareValues(firstFD->fields[compIDX],
                                                  secondFD->fields[compIDX]);

    if (cmp != 0) {
      // System.out.println("    return " + (cmp < 0));
      return cmp < 0;
    }
  }
  return tieBreakLessThan(first, firstFD, second, secondFD);
}

shared_ptr<TopDocs>
TopDocs::merge(int topN, std::deque<std::shared_ptr<TopDocs>> &shardHits)
{
  return merge(0, topN, shardHits, true);
}

shared_ptr<TopDocs>
TopDocs::merge(int start, int topN,
               std::deque<std::shared_ptr<TopDocs>> &shardHits,
               bool setShardIndex)
{
  return mergeAux(nullptr, start, topN, shardHits, setShardIndex);
}

shared_ptr<TopFieldDocs>
TopDocs::merge(shared_ptr<Sort> sort, int topN,
               std::deque<std::shared_ptr<TopFieldDocs>> &shardHits)
{
  return merge(sort, 0, topN, shardHits, true);
}

shared_ptr<TopFieldDocs>
TopDocs::merge(shared_ptr<Sort> sort, int start, int topN,
               std::deque<std::shared_ptr<TopFieldDocs>> &shardHits,
               bool setShardIndex)
{
  if (sort == nullptr) {
    throw invalid_argument(L"sort must be non-null when merging field-docs");
  }
  return std::static_pointer_cast<TopFieldDocs>(
      mergeAux(sort, start, topN, shardHits, setShardIndex));
}

shared_ptr<TopDocs>
TopDocs::mergeAux(shared_ptr<Sort> sort, int start, int size,
                  std::deque<std::shared_ptr<TopDocs>> &shardHits,
                  bool setShardIndex)
{

  shared_ptr<PriorityQueue<std::shared_ptr<ShardRef>>> *const queue;
  if (sort == nullptr) {
    queue = make_shared<ScoreMergeSortQueue>(shardHits);
  } else {
    queue = make_shared<MergeSortQueue>(sort, shardHits);
  }

  int64_t totalHitCount = 0;
  int availHitCount = 0;
  float maxScore = Float::MIN_VALUE;
  for (int shardIDX = 0; shardIDX < shardHits.size(); shardIDX++) {
    shared_ptr<TopDocs> *const shard = shardHits[shardIDX];
    // totalHits can be non-zero even if no hits were
    // collected, when searchAfter was used:
    totalHitCount += shard->totalHits;
    if (shard->scoreDocs.size() > 0 && shard->scoreDocs.size() > 0) {
      availHitCount += shard->scoreDocs.size();
      queue->push_back(make_shared<ShardRef>(shardIDX, setShardIndex == false));
      maxScore = max(maxScore, shard->getMaxScore());
    }
  }

  if (availHitCount == 0) {
    maxScore = NAN;
  }

  std::deque<std::shared_ptr<ScoreDoc>> hits;
  if (availHitCount <= start) {
    hits = std::deque<std::shared_ptr<ScoreDoc>>(0);
  } else {
    hits = std::deque<std::shared_ptr<ScoreDoc>>(
        min(size, availHitCount - start));
    int requestedResultWindow = start + size;
    int numIterOnHits = min(availHitCount, requestedResultWindow);
    int hitUpto = 0;
    while (hitUpto < numIterOnHits) {
      assert(queue->size() > 0);
      shared_ptr<ShardRef> ref = queue->top();
      shared_ptr<ScoreDoc> *const hit =
          shardHits[ref->shardIndex]->scoreDocs[ref->hitIndex++];
      if (setShardIndex) {
        // caller asked us to record shardIndex (index of the TopDocs array)
        // this hit is coming from:
        hit->shardIndex = ref->shardIndex;
      } else if (hit->shardIndex == -1) {
        throw invalid_argument(L"setShardIndex is false but TopDocs[" +
                               to_wstring(ref->shardIndex) + L"].scoreDocs[" +
                               to_wstring(ref->hitIndex - 1) + L"] is not set");
      }

      if (hitUpto >= start) {
        hits[hitUpto - start] = hit;
      }

      hitUpto++;

      if (ref->hitIndex < shardHits[ref->shardIndex]->scoreDocs->size()) {
        // Not done with this these TopDocs yet:
        queue->updateTop();
      } else {
        queue->pop();
      }
    }
  }

  if (sort == nullptr) {
    return make_shared<TopDocs>(totalHitCount, hits, maxScore);
  } else {
    return make_shared<TopFieldDocs>(totalHitCount, hits, sort->getSort(),
                                     maxScore);
  }
}
} // namespace org::apache::lucene::search