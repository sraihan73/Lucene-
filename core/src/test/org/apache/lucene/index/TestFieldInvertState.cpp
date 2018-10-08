using namespace std;

#include "TestFieldInvertState.h"

namespace org::apache::lucene::index
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Token = org::apache::lucene::analysis::Token;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
const shared_ptr<NeverForgetsSimilarity>
    TestFieldInvertState::NeverForgetsSimilarity::INSTANCE =
        make_shared<NeverForgetsSimilarity>();

TestFieldInvertState::NeverForgetsSimilarity::NeverForgetsSimilarity()
{
  // no
}

int64_t TestFieldInvertState::NeverForgetsSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  this->lastState = state;
  return 1;
}

shared_ptr<Similarity::SimWeight>
TestFieldInvertState::NeverForgetsSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestFieldInvertState::NeverForgetsSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

void TestFieldInvertState::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setSimilarity(NeverForgetsSimilarity::INSTANCE);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTokenStream>(make_shared<Token>(L"a", 0, 1),
                                     make_shared<Token>(L"b", 2, 3),
                                     make_shared<Token>(L"c", 4, 5)),
      TextField::TYPE_NOT_STORED);
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<FieldInvertState> fis =
      NeverForgetsSimilarity::INSTANCE::lastState;
  TestUtil::assertEquals(1, fis->getMaxTermFrequency());
  TestUtil::assertEquals(3, fis->getUniqueTermCount());
  TestUtil::assertEquals(0, fis->getNumOverlap());
  TestUtil::assertEquals(3, fis->getLength());
  IOUtils::close({w, dir});
}

void TestFieldInvertState::testRandom() 
{
  int numUniqueTokens = TestUtil::nextInt(random(), 1, 25);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setSimilarity(NeverForgetsSimilarity::INSTANCE);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();

  int numTokens = atLeast(10000);
  std::deque<std::shared_ptr<Token>> tokens(numTokens);
  unordered_map<wchar_t, int> counts = unordered_map<wchar_t, int>();
  int numStacked = 0;
  int maxTermFreq = 0;
  int pos = -1;
  for (int i = 0; i < numTokens; i++) {
    wchar_t tokenChar =
        static_cast<wchar_t>(L'a' + random()->nextInt(numUniqueTokens));
    optional<int> oldCount = counts[tokenChar];
    int newCount;
    if (!oldCount) {
      newCount = 1;
    } else {
      newCount = 1 + oldCount;
    }
    counts.emplace(tokenChar, newCount);
    maxTermFreq = max(maxTermFreq, newCount);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Token> token =
        make_shared<Token>(Character::toString(tokenChar), 2 * i, 2 * i + 1);

    if (i > 0 && random()->nextInt(7) == 3) {
      token->setPositionIncrement(0);
      numStacked++;
    } else {
      pos++;
    }
    tokens[i] = token;
  }

  shared_ptr<Field> field =
      make_shared<Field>(L"field", make_shared<CannedTokenStream>(tokens),
                         TextField::TYPE_NOT_STORED);
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<FieldInvertState> fis =
      NeverForgetsSimilarity::INSTANCE::lastState;
  TestUtil::assertEquals(maxTermFreq, fis->getMaxTermFrequency());
  TestUtil::assertEquals(counts.size(), fis->getUniqueTermCount());
  TestUtil::assertEquals(numStacked, fis->getNumOverlap());
  TestUtil::assertEquals(numTokens, fis->getLength());
  TestUtil::assertEquals(pos, fis->getPosition());

  IOUtils::close({w, dir});
}
} // namespace org::apache::lucene::index