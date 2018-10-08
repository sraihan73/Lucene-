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
 * Implementation of the {@linkplain org.apache.lucene.queryparser.classic
 * Lucene classic query parser} using the flexible query parser frameworks
 *
 * <h2>Lucene Flexible Query Parser Implementation</h2>
 * <p>
 * The old Lucene query parser used to have only one class that performed
 * all the parsing operations. In the new query parser structure, the
 * parsing was divided in 3 steps: parsing (syntax), processing (semantic)
 * and building.
 * <p>
 * The classes contained in the package org.apache.lucene.queryParser.standard
 * are used to reproduce the same behavior as the old query parser.
 *
 * <p>
 * Check {@link
 * org.apache.lucene.queryparser.flexible.standard.StandardQueryParser} to quick
 * start using the Lucene query parser.
 */
namespace org::apache::lucene::queryparser::flexible::standard
{
}
