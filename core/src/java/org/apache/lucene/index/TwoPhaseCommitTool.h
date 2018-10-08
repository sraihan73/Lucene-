#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TwoPhaseCommit;
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
namespace org::apache::lucene::index
{

/**
 * A utility for executing 2-phase commit on several objects.
 *
 * @see TwoPhaseCommit
 * @lucene.experimental
 */
class TwoPhaseCommitTool final
    : public std::enable_shared_from_this<TwoPhaseCommitTool>
{
  GET_CLASS_NAME(TwoPhaseCommitTool)

  /** No instance */
private:
  TwoPhaseCommitTool();

  /**
   * Thrown by {@link TwoPhaseCommitTool#execute(TwoPhaseCommit...)} when an
   * object fails to prepareCommit().
   */
public:
  class PrepareCommitFailException : public IOException
  {
    GET_CLASS_NAME(PrepareCommitFailException)

    /** Sole constructor. */
  public:
    PrepareCommitFailException(std::runtime_error cause,
                               std::shared_ptr<TwoPhaseCommit> obj);

  protected:
    std::shared_ptr<PrepareCommitFailException> shared_from_this()
    {
      return std::static_pointer_cast<PrepareCommitFailException>(
          java.io.IOException::shared_from_this());
    }
  };

  /**
   * Thrown by {@link TwoPhaseCommitTool#execute(TwoPhaseCommit...)} when an
   * object fails to commit().
   */
public:
  class CommitFailException : public IOException
  {
    GET_CLASS_NAME(CommitFailException)

    /** Sole constructor. */
  public:
    CommitFailException(std::runtime_error cause,
                        std::shared_ptr<TwoPhaseCommit> obj);

  protected:
    std::shared_ptr<CommitFailException> shared_from_this()
    {
      return std::static_pointer_cast<CommitFailException>(
          java.io.IOException::shared_from_this());
    }
  };

  /** rollback all objects, discarding any exceptions that occur. */
private:
  static void rollback(std::deque<TwoPhaseCommit> &objects);

  /**
   * Executes a 2-phase commit algorithm by first
   * {@link TwoPhaseCommit#prepareCommit()} all objects and only if all succeed,
   * it proceeds with {@link TwoPhaseCommit#commit()}. If any of the objects
   * fail on either the preparation or actual commit, it terminates and
   * {@link TwoPhaseCommit#rollback()} all of them.
   * <p>
   * <b>NOTE:</b> it may happen that an object fails to commit, after few have
   * already successfully committed. This tool will still issue a rollback
   * instruction on them as well, but depending on the implementation, it may
   * not have any effect.
   * <p>
   * <b>NOTE:</b> if any of the objects are {@code null}, this method simply
   * skips over them.
   *
   * @throws PrepareCommitFailException
   *           if any of the objects fail to
   *           {@link TwoPhaseCommit#prepareCommit()}
   * @throws CommitFailException
   *           if any of the objects fail to {@link TwoPhaseCommit#commit()}
   */
public:
  static void execute(std::deque<TwoPhaseCommit> &objects) throw(
      PrepareCommitFailException, CommitFailException);
};

} // namespace org::apache::lucene::index
