using namespace std;

#include "ConfusionMatrixGenerator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TermRangeQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/NamedThreadFactory.h"
#include "../ClassificationResult.h"
#include "../Classifier.h"

namespace org::apache::lucene::classification::utils
{
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using Classifier = org::apache::lucene::classification::Classifier;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BytesRef = org::apache::lucene::util::BytesRef;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;

ConfusionMatrixGenerator::ConfusionMatrixGenerator() {}

template <typename T>
shared_ptr<ConfusionMatrix> ConfusionMatrixGenerator::getConfusionMatrix(
    shared_ptr<IndexReader> reader, shared_ptr<Classifier<T>> classifier,
    const wstring &classFieldName, const wstring &textFieldName,
    int64_t timeoutMilliseconds) 
{

  shared_ptr<ExecutorService> executorService = Executors::newFixedThreadPool(
      1, make_shared<NamedThreadFactory>(L"confusion-matrix-gen-"));

  try {

    unordered_map<wstring, unordered_map<wstring, int64_t>> counts =
        unordered_map<wstring, unordered_map<wstring, int64_t>>();
    shared_ptr<IndexSearcher> indexSearcher =
        make_shared<IndexSearcher>(reader);
    shared_ptr<TopDocs> topDocs =
        indexSearcher->search(make_shared<TermRangeQuery>(
                                  classFieldName, nullptr, nullptr, true, true),
                              numeric_limits<int>::max());
    double time = 0;

    int counter = 0;
    for (auto scoreDoc : topDocs->scoreDocs) {

      if (timeoutMilliseconds > 0 && time >= timeoutMilliseconds) {
        break;
      }

      shared_ptr<Document> doc = reader->document(scoreDoc->doc);
      std::deque<wstring> correctAnswers = doc->getValues(classFieldName);

      if (correctAnswers.size() > 0 && correctAnswers.size() > 0) {
        Arrays::sort(correctAnswers);
        shared_ptr<ClassificationResult<T>> result;
        wstring text = doc[textFieldName];
        if (text != L"") {
          try {
            // fail if classification takes more than 5s
            int64_t start = System::currentTimeMillis();
            result = executorService
                         ->submit([&]() { classifier->assignClass(text); })
                         ->get(5, TimeUnit::SECONDS);
            int64_t end = System::currentTimeMillis();
            time += end - start;

            if (result != nullptr) {
              T assignedClass = result->getAssignedClass();
              if (assignedClass != nullptr) {
                counter++;
                // C++ TODO: There is no native C++ equivalent to 'toString':
                wstring classified =
                    std::static_pointer_cast<BytesRef>(assignedClass) != nullptr
                        ? (std::static_pointer_cast<BytesRef>(assignedClass))
                              ->utf8ToString()
                        : assignedClass.toString();

                wstring correctAnswer;
                if (Arrays::binarySearch(correctAnswers, classified) >= 0) {
                  correctAnswer = classified;
                } else {
                  correctAnswer = correctAnswers[0];
                }

                unordered_map<wstring, int64_t> stringLongMap =
                    counts[correctAnswer];
                if (stringLongMap.size() > 0) {
                  optional<int64_t> aLong = stringLongMap[classified];
                  if (aLong) {
                    stringLongMap.emplace(classified, aLong + 1);
                  } else {
                    stringLongMap.emplace(classified, 1LL);
                  }
                } else {
                  stringLongMap = unordered_map<>();
                  stringLongMap.emplace(classified, 1LL);
                  counts.emplace(correctAnswer, stringLongMap);
                }
              }
            }
          } catch (const TimeoutException &timeoutException) {
            // add classification timeout
            time += 5000;
          }
          // C++ TODO: There is no equivalent in C++ to Java 'multi-catch'
          // syntax:
          catch (ExecutionException | InterruptedException executionException) {
            throw runtime_error(executionException);
          }
        }
      }
    }
    return make_shared<ConfusionMatrix>(counts, time / counter, counter);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    executorService->shutdown();
  }
}

ConfusionMatrixGenerator::ConfusionMatrix::ConfusionMatrix(
    unordered_map<wstring, unordered_map<wstring, int64_t>> &linearizedMatrix,
    double avgClassificationTime, int numberOfEvaluatedDocs)
    : linearizedMatrix(linearizedMatrix),
      avgClassificationTime(avgClassificationTime),
      numberOfEvaluatedDocs(numberOfEvaluatedDocs)
{
}

unordered_map<wstring, unordered_map<wstring, int64_t>>
ConfusionMatrixGenerator::ConfusionMatrix::getLinearizedMatrix()
{
  return linearizedMatrix;
}

double
ConfusionMatrixGenerator::ConfusionMatrix::getPrecision(const wstring &klass)
{
  unordered_map<wstring, int64_t> classifications = linearizedMatrix[klass];
  double tp = 0;
  double den = 0; // tp + fp
  if (classifications.size() > 0) {
    for (auto entry : classifications) {
      if (klass == entry.first) {
        tp += entry.second;
      }
    }
    for (auto values : linearizedMatrix) {
      if (values.second::containsKey(klass)) {
        den += values.second->get(klass);
      }
    }
  }
  return tp > 0 ? tp / den : 0;
}

double
ConfusionMatrixGenerator::ConfusionMatrix::getRecall(const wstring &klass)
{
  unordered_map<wstring, int64_t> classifications = linearizedMatrix[klass];
  double tp = 0;
  double fn = 0;
  if (classifications.size() > 0) {
    for (auto entry : classifications) {
      if (klass == entry.first) {
        tp += entry.second;
      } else {
        fn += entry.second;
      }
    }
  }
  return tp + fn > 0 ? tp / (tp + fn) : 0;
}

double
ConfusionMatrixGenerator::ConfusionMatrix::getF1Measure(const wstring &klass)
{
  double recall = getRecall(klass);
  double precision = getPrecision(klass);
  return precision > 0 && recall > 0
             ? 2 * precision * recall / (precision + recall)
             : 0;
}

double ConfusionMatrixGenerator::ConfusionMatrix::getF1Measure()
{
  double recall = getRecall();
  double precision = getPrecision();
  return precision > 0 && recall > 0
             ? 2 * precision * recall / (precision + recall)
             : 0;
}

double ConfusionMatrixGenerator::ConfusionMatrix::getAccuracy()
{
  if (this->accuracy == -1) {
    double tp = 0;
    double tn = 0;
    double tfp = 0; // tp + fp
    double fn = 0;
    for (auto classification : linearizedMatrix) {
      wstring klass = classification.first;
      for (shared_ptr<unordered_map::Entry<wstring, int64_t>> entry :
           classification.second::entrySet()) {
        if (klass == entry.first) {
          tp += entry.second;
        } else {
          fn += entry.second;
        }
      }
      for (auto values : linearizedMatrix) {
        if (values.second::containsKey(klass)) {
          tfp += values.second->get(klass);
        } else {
          tn++;
        }
      }
    }
    this->accuracy = (tp + tn) / (tfp + fn + tn);
  }
  return this->accuracy;
}

double ConfusionMatrixGenerator::ConfusionMatrix::getPrecision()
{
  double p = 0;
  for (auto classification : linearizedMatrix) {
    wstring klass = classification.first;
    p += getPrecision(klass);
  }

  return p / linearizedMatrix.size();
}

double ConfusionMatrixGenerator::ConfusionMatrix::getRecall()
{
  double r = 0;
  for (auto classification : linearizedMatrix) {
    wstring klass = classification.first;
    r += getRecall(klass);
  }

  return r / linearizedMatrix.size();
}

wstring ConfusionMatrixGenerator::ConfusionMatrix::toString()
{
  return wstring(L"ConfusionMatrix{") + L"linearizedMatrix=" +
         linearizedMatrix + L", avgClassificationTime=" +
         to_wstring(avgClassificationTime) + L", numberOfEvaluatedDocs=" +
         to_wstring(numberOfEvaluatedDocs) + L'}';
}

double ConfusionMatrixGenerator::ConfusionMatrix::getAvgClassificationTime()
{
  return avgClassificationTime;
}

int ConfusionMatrixGenerator::ConfusionMatrix::getNumberOfEvaluatedDocs()
{
  return numberOfEvaluatedDocs;
}
} // namespace org::apache::lucene::classification::utils