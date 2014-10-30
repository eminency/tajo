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

package org.apache.tajo.engine.planner;

import org.apache.tajo.SessionVars;
import org.apache.tajo.catalog.Schema;
import org.apache.tajo.engine.eval.EvalNode;
import org.apache.tajo.storage.Tuple;
import org.apache.tajo.worker.TaskAttemptContext;
import org.apache.tajo.util.JsonUtil;
import org.codehaus.jackson.node.ArrayNode;
import org.codehaus.jackson.node.JsonNodeFactory;

public class Projector {
  private final Schema inSchema;

  // for projection
  private final EvalNode[] evals;

  public Projector(TaskAttemptContext context, Schema inSchema, Schema outSchema, Target [] targets) {
    int targetNum;

    this.inSchema = inSchema;
    if (targets == null) {
      targets = PlannerUtil.schemaToTargets(outSchema);
    }

    targetNum = targets.length;
    evals = new EvalNode[targetNum];

    if (context.getQueryContext().getBool(SessionVars.CODEGEN)) {
      EvalNode eval;
      for (int i = 0; i < targetNum; i++) {
        eval = targets[i].getEvalTree();
        evals[i] = context.getPrecompiledEval(inSchema, eval);
      }
    } else {
      for (int i = 0; i < targetNum; i++) {
        evals[i] = targets[i].getEvalTree();
      }
    }
  }

  public void eval(Tuple in, Tuple out) {
    for (int i = 0; i < evals.length; i++) {
      out.put(i, evals[i].eval(inSchema, in));
    }
  }

  public ArrayNode toJsonObject() {
    ArrayNode array = JsonNodeFactory.instance.arrayNode();

    for (EvalNode ev:evals) {
      array.add(JsonUtil.stringToJSONNode(ev.toJson()));
    }

    return array;
  }
}
