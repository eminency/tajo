/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.tajo.engine.planner.physical;

import org.apache.tajo.catalog.Column;
import org.apache.tajo.catalog.Schema;
import org.apache.tajo.engine.eval.AggregationFunctionCallEval;
import org.apache.tajo.engine.planner.logical.GroupbyNode;
import org.apache.tajo.worker.TaskAttemptContext;
import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonParser;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.ArrayNode;
import org.codehaus.jackson.node.JsonNodeFactory;
import org.codehaus.jackson.node.ObjectNode;

import java.io.IOException;

public abstract class AggregationExec extends UnaryPhysicalExec {
  protected GroupbyNode plan;

  protected final int groupingKeyNum;
  protected int groupingKeyIds[];
  protected final int aggFunctionsNum;
  protected final AggregationFunctionCallEval aggFunctions[];

  protected Schema evalSchema;

  public AggregationExec(final TaskAttemptContext context, GroupbyNode plan,
                         PhysicalExec child) throws IOException {
    super(context, plan.getInSchema(), plan.getOutSchema(), child);
    this.plan = plan;

    evalSchema = plan.getOutSchema();

    final Column [] keyColumns = plan.getGroupingColumns();
    groupingKeyNum = keyColumns.length;
    groupingKeyIds = new int[groupingKeyNum];
    Column col;
    for (int idx = 0; idx < plan.getGroupingColumns().length; idx++) {
      col = keyColumns[idx];
      if (col.hasQualifier()) {
        groupingKeyIds[idx] = inSchema.getColumnId(col.getQualifiedName());
      } else {
        groupingKeyIds[idx] = inSchema.getColumnIdByName(col.getSimpleName());
      }
    }

    if (plan.hasAggFunctions()) {
      aggFunctions = plan.getAggFunctions();
      aggFunctionsNum = aggFunctions.length;
    } else {
      aggFunctions = new AggregationFunctionCallEval[0];
      aggFunctionsNum = 0;
    }
  }

  @Override
  public void close() throws IOException {
    super.close();
    plan = null;
  }

  @Override
  public ObjectNode toJsonObject() {
    ObjectNode obj = JsonNodeFactory.instance.objectNode();
    ArrayNode groupKeyArr = JsonNodeFactory.instance.arrayNode();
    ArrayNode funcArr = JsonNodeFactory.instance.arrayNode();

    obj.put("name", this.getClass().getName());

    for (int groupId: groupingKeyIds)
      groupKeyArr.add(groupId);

    obj.put("groupingKeyIds", groupKeyArr);

    if (plan.hasAggFunctions()) {
      JsonFactory factory = new JsonFactory();
      ObjectMapper mapper = new ObjectMapper();

      for (AggregationFunctionCallEval func: aggFunctions) {
        try {
          JsonParser parser = factory.createJsonParser(func.toJson());
          JsonNode node = mapper.readTree(parser);
          funcArr.add(node);
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }

    obj.put("funcs", funcArr);

    if (child != null)
      obj.put("child", child.toJsonObject());

    return obj;
  }
}
