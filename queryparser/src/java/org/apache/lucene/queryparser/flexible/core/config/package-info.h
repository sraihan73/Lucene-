#pragma once
#include "stringhelper.h"
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

/**
 * Base classes used to configure the query processing.
 *
 * <h2>Query Configuration Interfaces</h2>
 * <p>
 * The package <tt>org.apache.lucene.queryparser.flexible.config</tt> contains
 * query configuration handler abstract class that all config handlers should
 * extend. <p> See {@link
 * org.apache.lucene.queryparser.flexible.standard.config.StandardQueryConfigHandler}
 * for a reference implementation. <p> The {@link
 * org.apache.lucene.queryparser.flexible.core.config.QueryConfigHandler} and
 * {@link org.apache.lucene.queryparser.flexible.core.config.FieldConfig} are
 * used in the processors to access config information in a flexible and
 * independent way. See {@link
 * org.apache.lucene.queryparser.flexible.standard.processors.TermRangeQueryNodeProcessor}
 * for a reference implementation.
 */
namespace org::apache::lucene::queryparser::flexible::core::config
{
}
