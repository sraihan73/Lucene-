using namespace std;

#include "DrillSidewaysScorer.h"

namespace org::apache::lucene::facet
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using Collector = org::apache::lucene::search::Collector;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

DrillSidewaysScorer::DrillSidewaysScorer(
    shared_ptr<LeafReaderContext> context, shared_ptr<Scorer> baseScorer,
    shared_ptr<Collector> drillDownCollector,
    std::deque<std::shared_ptr<DocsAndCost>> &dims, bool scoreSubDocsAtOnce)
    : drillDownCollector(drillDownCollector), dims(dims),
      baseScorer(baseScorer), baseIterator(baseScorer->begin()),
      context(context), scoreSubDocsAtOnce(scoreSubDocsAtOnce)
{
}

int64_t DrillSidewaysScorer::cost() { return baseIterator->cost(); }

int DrillSidewaysScorer::score(shared_ptr<LeafCollector> collector,
                               shared_ptr<Bits> acceptDocs, int min,
                               int maxDoc) 
{
  if (min != 0) {
    throw invalid_argument(L"min must be 0, got " + to_wstring(min));
  }
  if (maxDoc != numeric_limits<int>::max()) {
    throw invalid_argument(L"maxDoc must be Integer.MAX_VALUE");
  }
  // if (DEBUG) {
  //  System.out.println("\nscore: reader=" + context.reader());
  //}
  // System.out.println("score r=" + context.reader());
  shared_ptr<FakeScorer> scorer = make_shared<FakeScorer>(shared_from_this());
  collector->setScorer(scorer);
  if (drillDownCollector != nullptr) {
    drillDownLeafCollector = drillDownCollector->getLeafCollector(context);
    drillDownLeafCollector->setScorer(scorer);
  } else {
    drillDownLeafCollector.reset();
  }
  for (auto dim : dims) {
    dim->sidewaysLeafCollector =
        dim->sidewaysCollector->getLeafCollector(context);
    dim->sidewaysLeafCollector->setScorer(scorer);
  }

  // some scorers, eg ReqExlScorer, can hit NPE if cost is called after nextDoc
  int64_t baseQueryCost = baseIterator->cost();

  constexpr int numDims = dims.size();

  int64_t drillDownCost = 0;
  for (int dim = 0; dim < numDims; dim++) {
    drillDownCost += dims[dim]->approximation.cost();
  }

  int64_t drillDownAdvancedCost = 0;
  if (numDims > 1) {
    drillDownAdvancedCost = dims[1]->approximation.cost();
  }

  // Position all scorers to their first matching doc:
  baseIterator->nextDoc();
  for (auto dim : dims) {
    dim->approximation->nextDoc();
  }

  /*
  System.out.println("\nbaseDocID=" + baseScorer.docID() + " est=" +
  estBaseHitCount); System.out.println("  maxDoc=" + context.reader().maxDoc());
  System.out.println("  maxCost=" + maxCost);
  System.out.println("  dims[0].freq=" + dims[0].freq);
  if (numDims > 1) {
    System.out.println("  dims[1].freq=" + dims[1].freq);
  }
  */

  if (scoreSubDocsAtOnce || baseQueryCost < drillDownCost / 10) {
    // System.out.println("queryFirst: baseScorer=" + baseScorer + "
    // disis.length=" + disis.length + " bits.length=" + bits.length);
    doQueryFirstScoring(acceptDocs, collector, dims);
  } else if (numDims > 1 && drillDownAdvancedCost < baseQueryCost / 10) {
    // System.out.println("drillDownAdvance");
    doDrillDownAdvanceScoring(acceptDocs, collector, dims);
  } else {
    // System.out.println("union");
    doUnionScoring(acceptDocs, collector, dims);
  }

  return numeric_limits<int>::max();
}

void DrillSidewaysScorer::doQueryFirstScoring(
    shared_ptr<Bits> acceptDocs, shared_ptr<LeafCollector> collector,
    std::deque<std::shared_ptr<DocsAndCost>> &dims) 
{
  // if (DEBUG) {
  //  System.out.println("  doQueryFirstScoring");
  //}
  int docID = baseScorer->docID();

  while (docID != PostingsEnum::NO_MORE_DOCS) {
    if (acceptDocs != nullptr && acceptDocs->get(docID) == false) {
      docID = baseIterator->nextDoc();
      continue;
    }
    shared_ptr<LeafCollector> failedCollector = nullptr;
    for (auto dim : dims) {
      // TODO: should we sort this 2nd dimension of
      // docsEnums from most frequent to least?
      if (dim->approximation->docID() < docID) {
        dim->approximation->advance(docID);
      }

      bool matches = false;
      if (dim->approximation->docID() == docID) {
        if (dim->twoPhase == nullptr) {
          matches = true;
        } else {
          matches = dim->twoPhase->matches();
        }
      }

      if (matches == false) {
        if (failedCollector != nullptr) {
          // More than one dim fails on this document, so
          // it's neither a hit nor a near-miss; move to
          // next doc:
          docID = baseIterator->nextDoc();
          goto nextDocContinue;
        } else {
          failedCollector = dim->sidewaysLeafCollector;
        }
      }
    }

    collectDocID = docID;

    // TODO: we could score on demand instead since we are
    // daat here:
    collectScore = baseScorer->score();

    if (failedCollector == nullptr) {
      // Hit passed all filters, so it's "real":
      collectHit(collector, dims);
    } else {
      // Hit missed exactly one filter:
      collectNearMiss(failedCollector);
    }

    docID = baseIterator->nextDoc();
  nextDocContinue:;
  }
nextDocBreak:;
}

void DrillSidewaysScorer::doDrillDownAdvanceScoring(
    shared_ptr<Bits> acceptDocs, shared_ptr<LeafCollector> collector,
    std::deque<std::shared_ptr<DocsAndCost>> &dims) 
{
  constexpr int maxDoc = context->reader()->maxDoc();
  constexpr int numDims = dims.size();

  // if (DEBUG) {
  //  System.out.println("  doDrillDownAdvanceScoring");
  //}

  // TODO: maybe a class like BS, instead of parallel arrays
  std::deque<int> filledSlots(CHUNK);
  std::deque<int> docIDs(CHUNK);
  std::deque<float> scores(CHUNK);
  std::deque<int> missingDims(CHUNK);
  std::deque<int> counts(CHUNK);

  docIDs[0] = -1;
  int nextChunkStart = CHUNK;

  shared_ptr<FixedBitSet> *const seen = make_shared<FixedBitSet>(CHUNK);

  while (true) {
    // if (DEBUG) {
    //  System.out.println("\ncycle nextChunkStart=" + nextChunkStart + "
    //  docIds[0]=" + docIDs[0]);
    //}

    // First dim:
    // if (DEBUG) {
    //  System.out.println("  dim0");
    //}
    shared_ptr<DocsAndCost> dc = dims[0];
    int docID = dc->approximation->docID();
    while (docID < nextChunkStart) {
      if (acceptDocs == nullptr || acceptDocs->get(docID)) {
        int slot = docID & MASK;

        if (docIDs[slot] != docID &&
            (dc->twoPhase == nullptr || dc->twoPhase->matches())) {
          seen->set(slot);
          // Mark slot as valid:
          // if (DEBUG) {
          //  System.out.println("    set docID=" + docID + " id=" +
          //  context.reader().document(docID).get("id"));
          //}
          docIDs[slot] = docID;
          missingDims[slot] = 1;
          counts[slot] = 1;
        }
      }

      docID = dc->approximation->nextDoc();
    }

    // Second dim:
    // if (DEBUG) {
    //  System.out.println("  dim1");
    //}
    dc = dims[1];
    docID = dc->approximation->docID();
    while (docID < nextChunkStart) {
      if (acceptDocs == nullptr ||
          acceptDocs->get(docID) &&
              (dc->twoPhase == nullptr || dc->twoPhase->matches())) {
        int slot = docID & MASK;

        if (docIDs[slot] != docID) {
          // Mark slot as valid:
          seen->set(slot);
          // if (DEBUG) {
          //  System.out.println("    set docID=" + docID + " missingDim=0 id="
          //  + context.reader().document(docID).get("id"));
          //}
          docIDs[slot] = docID;
          missingDims[slot] = 0;
          counts[slot] = 1;
        } else {
          // TODO: single-valued dims will always be true
          // below; we could somehow specialize
          if (missingDims[slot] >= 1) {
            missingDims[slot] = 2;
            counts[slot] = 2;
            // if (DEBUG) {
            //  System.out.println("    set docID=" + docID + " missingDim=2
            //  id=" + context.reader().document(docID).get("id"));
            //}
          } else {
            counts[slot] = 1;
            // if (DEBUG) {
            //  System.out.println("    set docID=" + docID + " missingDim=" +
            //  missingDims[slot] + " id=" +
            //  context.reader().document(docID).get("id"));
            //}
          }
        }
      }

      docID = dc->approximation->nextDoc();
    }

    // After this we can "upgrade" to conjunction, because
    // any doc not seen by either dim 0 or dim 1 cannot be
    // a hit or a near miss:

    // if (DEBUG) {
    //  System.out.println("  baseScorer");
    //}

    // Fold in baseScorer, using advance:
    int filledCount = 0;
    int slot0 = 0;
    while (slot0 < CHUNK && (slot0 = seen->nextSetBit(slot0)) !=
                                DocIdSetIterator::NO_MORE_DOCS) {
      int ddDocID = docIDs[slot0];
      assert(ddDocID != -1);

      int baseDocID = baseIterator->docID();
      if (baseDocID < ddDocID) {
        baseDocID = baseIterator->advance(ddDocID);
      }
      if (baseDocID == ddDocID) {
        // if (DEBUG) {
        //  System.out.println("    keep docID=" + ddDocID + " id=" +
        //  context.reader().document(ddDocID).get("id"));
        //}
        scores[slot0] = baseScorer->score();
        filledSlots[filledCount++] = slot0;
        counts[slot0]++;
      } else {
        // if (DEBUG) {
        //  System.out.println("    no docID=" + ddDocID + " id=" +
        //  context.reader().document(ddDocID).get("id"));
        //}
        docIDs[slot0] = -1;

        // TODO: we could jump slot0 forward to the
        // baseDocID ... but we'd need to set docIDs for
        // intervening slots to -1
      }
      slot0++;
    }
    seen->clear(0, CHUNK);

    if (filledCount == 0) {
      if (nextChunkStart >= maxDoc) {
        break;
      }
      nextChunkStart += CHUNK;
      continue;
    }

    // TODO: factor this out & share w/ union scorer,
    // except we start from dim=2 instead:
    for (int dim = 2; dim < numDims; dim++) {
      // if (DEBUG) {
      //  System.out.println("  dim=" + dim + " [" + dims[dim].dim + "]");
      //}
      dc = dims[dim];
      docID = dc->approximation->docID();
      while (docID < nextChunkStart) {
        int slot = docID & MASK;
        if (docIDs[slot] == docID && counts[slot] >= dim &&
            (dc->twoPhase == nullptr || dc->twoPhase->matches())) {
          // TODO: single-valued dims will always be true
          // below; we could somehow specialize
          if (missingDims[slot] >= dim) {
            // if (DEBUG) {
            //  System.out.println("    set docID=" + docID + " count=" +
            //  (dim+2));
            //}
            missingDims[slot] = dim + 1;
            counts[slot] = dim + 2;
          } else {
            // if (DEBUG) {
            //  System.out.println("    set docID=" + docID + " missing count="
            //  + (dim+1));
            //}
            counts[slot] = dim + 1;
          }
        }

        // TODO: sometimes use advance?
        docID = dc->approximation->nextDoc();
      }
    }

    // Collect:
    // if (DEBUG) {
    //  System.out.println("  now collect: " + filledCount + " hits");
    //}
    for (int i = 0; i < filledCount; i++) {
      int slot = filledSlots[i];
      collectDocID = docIDs[slot];
      collectScore = scores[slot];
      // if (DEBUG) {
      //  System.out.println("    docID=" + docIDs[slot] + " count=" +
      //  counts[slot]);
      //}
      if (counts[slot] == 1 + numDims) {
        collectHit(collector, dims);
      } else if (counts[slot] == numDims) {
        collectNearMiss(dims[missingDims[slot]]->sidewaysLeafCollector);
      }
    }

    if (nextChunkStart >= maxDoc) {
      break;
    }

    nextChunkStart += CHUNK;
  }
}

void DrillSidewaysScorer::doUnionScoring(
    shared_ptr<Bits> acceptDocs, shared_ptr<LeafCollector> collector,
    std::deque<std::shared_ptr<DocsAndCost>> &dims) 
{
  // if (DEBUG) {
  //  System.out.println("  doUnionScoring");
  //}

  constexpr int maxDoc = context->reader()->maxDoc();
  constexpr int numDims = dims.size();

  // TODO: maybe a class like BS, instead of parallel arrays
  std::deque<int> filledSlots(CHUNK);
  std::deque<int> docIDs(CHUNK);
  std::deque<float> scores(CHUNK);
  std::deque<int> missingDims(CHUNK);
  std::deque<int> counts(CHUNK);

  docIDs[0] = -1;

  // NOTE: this is basically a specialized version of
  // BooleanScorer, to the minShouldMatch=N-1 case, but
  // carefully tracking which dimension failed to match

  int nextChunkStart = CHUNK;

  while (true) {
    // if (DEBUG) {
    //  System.out.println("\ncycle nextChunkStart=" + nextChunkStart + "
    //  docIds[0]=" + docIDs[0]);
    //}
    int filledCount = 0;
    int docID = baseIterator->docID();
    // if (DEBUG) {
    //  System.out.println("  base docID=" + docID);
    //}
    while (docID < nextChunkStart) {
      if (acceptDocs == nullptr || acceptDocs->get(docID)) {
        int slot = docID & MASK;
        // if (DEBUG) {
        //  System.out.println("    docIDs[slot=" + slot + "]=" + docID + " id="
        //  + context.reader().document(docID).get("id"));
        //}

        // Mark slot as valid:
        assert((docIDs[slot] != docID,
                L"slot=" + to_wstring(slot) + L" docID=" + to_wstring(docID)));
        docIDs[slot] = docID;
        scores[slot] = baseScorer->score();
        filledSlots[filledCount++] = slot;
        missingDims[slot] = 0;
        counts[slot] = 1;
      }
      docID = baseIterator->nextDoc();
    }

    if (filledCount == 0) {
      if (nextChunkStart >= maxDoc) {
        break;
      }
      nextChunkStart += CHUNK;
      continue;
    }

    // First drill-down dim, basically adds SHOULD onto
    // the baseQuery:
    // if (DEBUG) {
    //  System.out.println("  dim=0 [" + dims[0].dim + "]");
    //}
    {
      shared_ptr<DocsAndCost> dc = dims[0];
      docID = dc->approximation->docID();
      // if (DEBUG) {
      //  System.out.println("    start docID=" + docID);
      //}
      while (docID < nextChunkStart) {
        int slot = docID & MASK;
        if (docIDs[slot] == docID &&
            (dc->twoPhase == nullptr || dc->twoPhase->matches())) {
          // if (DEBUG) {
          //  System.out.println("      set docID=" + docID + " count=2");
          //}
          missingDims[slot] = 1;
          counts[slot] = 2;
        }
        docID = dc->approximation->nextDoc();
      }
    }

    for (int dim = 1; dim < numDims; dim++) {
      // if (DEBUG) {
      //  System.out.println("  dim=" + dim + " [" + dims[dim].dim + "]");
      //}

      shared_ptr<DocsAndCost> dc = dims[dim];
      docID = dc->approximation->docID();
      // if (DEBUG) {
      //  System.out.println("    start docID=" + docID);
      //}
      while (docID < nextChunkStart) {
        int slot = docID & MASK;
        if (docIDs[slot] == docID && counts[slot] >= dim &&
            (dc->twoPhase == nullptr || dc->twoPhase->matches())) {
          // This doc is still in the running...
          // TODO: single-valued dims will always be true
          // below; we could somehow specialize
          if (missingDims[slot] >= dim) {
            // if (DEBUG) {
            //  System.out.println("      set docID=" + docID + " count=" +
            //  (dim+2));
            //}
            missingDims[slot] = dim + 1;
            counts[slot] = dim + 2;
          } else {
            // if (DEBUG) {
            //  System.out.println("      set docID=" + docID + " missing
            //  count=" + (dim+1));
            //}
            counts[slot] = dim + 1;
          }
        }
        docID = dc->approximation->nextDoc();
      }
    }

    // Collect:
    // System.out.println("  now collect: " + filledCount + " hits");
    for (int i = 0; i < filledCount; i++) {
      // NOTE: This is actually in-order collection,
      // because we only accept docs originally returned by
      // the baseScorer (ie that Scorer is AND'd)
      int slot = filledSlots[i];
      collectDocID = docIDs[slot];
      collectScore = scores[slot];
      // if (DEBUG) {
      //  System.out.println("    docID=" + docIDs[slot] + " count=" +
      //  counts[slot]);
      //}
      // System.out.println("  collect doc=" + collectDocID + " main.freq=" +
      // (counts[slot]-1) + " main.doc=" + collectDocID + " exactCount=" +
      // numDims);
      if (counts[slot] == 1 + numDims) {
        // System.out.println("    hit");
        collectHit(collector, dims);
      } else if (counts[slot] == numDims) {
        // System.out.println("    sw");
        collectNearMiss(dims[missingDims[slot]]->sidewaysLeafCollector);
      }
    }

    if (nextChunkStart >= maxDoc) {
      break;
    }

    nextChunkStart += CHUNK;
  }
}

void DrillSidewaysScorer::collectHit(
    shared_ptr<LeafCollector> collector,
    std::deque<std::shared_ptr<DocsAndCost>> &dims) 
{
  // if (DEBUG) {
  //  System.out.println("      hit");
  //}

  collector->collect(collectDocID);
  if (drillDownCollector != nullptr) {
    drillDownLeafCollector->collect(collectDocID);
  }

  // TODO: we could "fix" faceting of the sideways counts
  // to do this "union" (of the drill down hits) in the
  // end instead:

  // Tally sideways counts:
  for (auto dim : dims) {
    dim->sidewaysLeafCollector->collect(collectDocID);
  }
}

void DrillSidewaysScorer::collectNearMiss(
    shared_ptr<LeafCollector> sidewaysCollector) 
{
  // if (DEBUG) {
  //  System.out.println("      missingDim=" + dim);
  //}
  sidewaysCollector->collect(collectDocID);
}

DrillSidewaysScorer::FakeScorer::FakeScorer(
    shared_ptr<DrillSidewaysScorer> outerInstance)
    : org::apache::lucene::search::Scorer(nullptr), outerInstance(outerInstance)
{
}

int DrillSidewaysScorer::FakeScorer::docID()
{
  return outerInstance->collectDocID;
}

shared_ptr<DocIdSetIterator> DrillSidewaysScorer::FakeScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>(
      L"FakeScorer doesn't support nextDoc()");
}

float DrillSidewaysScorer::FakeScorer::score()
{
  return outerInstance->collectScore;
}

shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>>
DrillSidewaysScorer::FakeScorer::getChildren()
{
  return Collections::singletonList(
      make_shared<Scorer::ChildScorer>(outerInstance->baseScorer, L"MUST"));
}

shared_ptr<Weight> DrillSidewaysScorer::FakeScorer::getWeight()
{
  throw make_shared<UnsupportedOperationException>();
}

DrillSidewaysScorer::DocsAndCost::DocsAndCost(
    shared_ptr<Scorer> scorer, shared_ptr<Collector> sidewaysCollector)
    : sidewaysCollector(sidewaysCollector)
{
  shared_ptr<TwoPhaseIterator> *const twoPhase = scorer->twoPhaseIterator();
  if (twoPhase == nullptr) {
    this->approximation = scorer->begin();
    this->twoPhase.reset();
  } else {
    this->approximation = twoPhase->approximation();
    this->twoPhase = twoPhase;
  }
}
} // namespace org::apache::lucene::facet