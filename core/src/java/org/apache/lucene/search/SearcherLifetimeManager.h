#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search
{

/**
 * Keeps track of current plus old IndexSearchers, closing
 * the old ones once they have timed out.
 *
 * Use it like this:
 *
 * <pre class="prettyprint">
 *   SearcherLifetimeManager mgr = new SearcherLifetimeManager();
 * </pre>
 *
 * Per search-request, if it's a "new" search request, then
 * obtain the latest searcher you have (for example, by
 * using {@link SearcherManager}), and then record this
 * searcher:
 *
 * <pre class="prettyprint">
 *   // Record the current searcher, and save the returend
 *   // token into user's search results (eg as a  hidden
 *   // HTML form field):
 *   long token = mgr.record(searcher);
 * </pre>
 *
 * When a follow-up search arrives, for example the user
 * clicks next page, drills down/up, etc., take the token
 * that you saved from the previous search and:
 *
 * <pre class="prettyprint">
 *   // If possible, obtain the same searcher as the last
 *   // search:
 *   IndexSearcher searcher = mgr.acquire(token);
 *   if (searcher != null) {
 *     // Searcher is still here
 *     try {
 *       // do searching...
 *     } finally {
 *       mgr.release(searcher);
 *       // Do not use searcher after this!
 *       searcher = null;
 *     }
 *   } else {
 *     // Searcher was pruned -- notify user session timed
 *     // out, or, pull fresh searcher again
 *   }
 * </pre>
 *
 * Finally, in a separate thread, ideally the same thread
 * that's periodically reopening your searchers, you should
 * periodically prune old searchers:
 *
 * <pre class="prettyprint">
 *   mgr.prune(new PruneByAge(600.0));
 * </pre>
 *
 * <p><b>NOTE</b>: keeping many searchers around means
 * you'll use more resources (open files, RAM) than a single
 * searcher.  However, as long as you are using {@link
 * DirectoryReader#openIfChanged(DirectoryReader)}, the searchers
 * will usually share almost all segments and the added resource usage
 * is contained.  When a large merge has completed, and
 * you reopen, because that is a large change, the new
 * searcher will use higher additional RAM than other
 * searchers; but large merges don't complete very often and
 * it's unlikely you'll hit two of them in your expiration
 * window.  Still you should budget plenty of heap in the
 * JVM to have a good safety margin.
 *
 * @lucene.experimental
 */

class SearcherLifetimeManager
    : public std::enable_shared_from_this<SearcherLifetimeManager>
{
  GET_CLASS_NAME(SearcherLifetimeManager)

public:
  static constexpr double NANOS_PER_SEC = 1000000000.0;

private:
  class SearcherTracker : public std::enable_shared_from_this<SearcherTracker>,
                          public Comparable<std::shared_ptr<SearcherTracker>>
  {
    GET_CLASS_NAME(SearcherTracker)
  public:
    const std::shared_ptr<IndexSearcher> searcher;
    const double recordTimeSec;
    const int64_t version;

    SearcherTracker(std::shared_ptr<IndexSearcher> searcher);

    // Newer searchers are sort before older ones:
    int compareTo(std::shared_ptr<SearcherTracker> other) override;

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual ~SearcherTracker();
  };

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed;
  bool closed = false;

  // TODO: we could get by w/ just a "set"; need to have
  // Tracker hash by its version and have compareTo(Long)
  // compare to its version
  const std::shared_ptr<
      ConcurrentHashMap<int64_t, std::shared_ptr<SearcherTracker>>>
      searchers = std::make_shared<
          ConcurrentHashMap<int64_t, std::shared_ptr<SearcherTracker>>>();

  void ensureOpen();

  /** Records that you are now using this IndexSearcher.
   *  Always call this when you've obtained a possibly new
   *  {@link IndexSearcher}, for example from {@link
   *  SearcherManager}.  It's fine if you already passed the
   *  same searcher to this method before.
   *
   *  <p>This returns the long token that you can later pass
   *  to {@link #acquire} to retrieve the same IndexSearcher.
   *  You should record this long token in the search results
   *  sent to your user, such that if the user performs a
   *  follow-on action (clicks next page, drills down, etc.)
   *  the token is returned. */
public:
  virtual int64_t
  record(std::shared_ptr<IndexSearcher> searcher) ;

  /** Retrieve a previously recorded {@link IndexSearcher}, if it
   *  has not yet been closed
   *
   *  <p><b>NOTE</b>: this may return null when the
   *  requested searcher has already timed out.  When this
   *  happens you should notify your user that their session
   *  timed out and that they'll have to restart their
   *  search.
   *
   *  <p>If this returns a non-null result, you must match
   *  later call {@link #release} on this searcher, best
   *  from a finally clause. */
  virtual std::shared_ptr<IndexSearcher> acquire(int64_t version);

  /** Release a searcher previously obtained from {@link
   *  #acquire}.
   *
   * <p><b>NOTE</b>: it's fine to call this after close. */
  virtual void release(std::shared_ptr<IndexSearcher> s) ;

  /** See {@link #prune}. */
public:
  class Pruner
  {
    GET_CLASS_NAME(Pruner)
    /** Return true if this searcher should be removed.
     *  @param ageSec how much time has passed since this
     *         searcher was the current (live) searcher
     *  @param searcher Searcher
     **/
  public:
    virtual bool doPrune(double ageSec,
                         std::shared_ptr<IndexSearcher> searcher) = 0;
  };

  /** Simple pruner that drops any searcher older by
   *  more than the specified seconds, than the newest
   *  searcher. */
public:
  class PruneByAge final : public std::enable_shared_from_this<PruneByAge>,
                           public Pruner
  {
    GET_CLASS_NAME(PruneByAge)
  private:
    const double maxAgeSec;

  public:
    PruneByAge(double maxAgeSec);

    bool doPrune(double ageSec,
                 std::shared_ptr<IndexSearcher> searcher) override;
  };

  /** Calls provided {@link Pruner} to prune entries.  The
   *  entries are passed to the Pruner in sorted (newest to
   *  oldest IndexSearcher) order.
   *
   *  <p><b>NOTE</b>: you must peridiocally call this, ideally
   *  from the same background thread that opens new
   *  searchers. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void prune(std::shared_ptr<Pruner> pruner) ;

  /** Close this to future searching; any searches still in
   *  process in other threads won't be affected, and they
   *  should still call {@link #release} after they are
   *  done.
   *
   *  <p><b>NOTE</b>: you must ensure no other threads are
   *  calling {@link #record} while you call close();
   *  otherwise it's possible not all searcher references
   *  will be freed. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~SearcherLifetimeManager();
};

} // namespace org::apache::lucene::search
