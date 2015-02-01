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

package org.apache.tajo.engine.utils;

import org.apache.tajo.ExecutionBlockId;
import org.apache.tajo.QueryId;
import org.apache.tajo.conf.TajoConf;
import org.apache.tajo.conf.TajoConf.ConfVars;
import org.apache.tajo.engine.query.QueryContext;
import org.apache.tajo.ipc.TajoWorkerProtocol.QueryProfileDataListProto.QueryProfileMetricsProto;
import org.apache.tajo.rpc.protocolrecords.PrimitiveProtos.KeyValueProto;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

public class QueryProfiler {
  static List<QueryId> queryIdList = new ArrayList<QueryId>();
  static Map<QueryId, Map<ExecutionBlockId, List<QueryProfileMetrics>>> profileValues =
      new HashMap<QueryId, Map<ExecutionBlockId, List<QueryProfileMetrics>>>();

  public static boolean isEnabledProfile(TajoConf tajoConf) {
    return tajoConf.getBoolVar(ConfVars.ENABLE_QUERY_PROFILE);
  }

  public static void addProfileMetrics(ExecutionBlockId ebId, QueryProfileMetrics metrics) {
    if (metrics == null) {
      return;
    }
    synchronized(queryIdList) {
      removeLatestValue();

      Map<ExecutionBlockId, List<QueryProfileMetrics>> ebProfileValues = profileValues.get(ebId.getQueryId());
      if (ebProfileValues == null) {
        ebProfileValues = new HashMap<ExecutionBlockId, List<QueryProfileMetrics>>();
        profileValues.put(ebId.getQueryId(), ebProfileValues);
        queryIdList.add(ebId.getQueryId());
      }

      List<QueryProfileMetrics> metricsList = ebProfileValues.get(ebId);
      if (metricsList == null) {
        metricsList = new ArrayList<QueryProfileMetrics>();
        ebProfileValues.put(ebId, metricsList);
      }
      boolean alreadyExists = false;
      for (QueryProfileMetrics eachMetrics: metricsList) {
        if (eachMetrics.operatorName.equals(metrics.operatorName)) {
          for (Map.Entry<String, AtomicLong> entry: metrics.metricsValues.entrySet()) {
            eachMetrics.addValue(entry.getKey(), entry.getValue().longValue());
          }
          alreadyExists = true;
        }
      }
      if (!alreadyExists) {
        metricsList.add(metrics);
      }
    }
  }

  public static Map<ExecutionBlockId, List<QueryProfileMetrics>> getProfileMetrics(QueryId queryId) {
    synchronized(queryIdList) {
      return profileValues.get(queryId);
    }
  }

  public static void removeProfileMetrics(QueryId queryId) {
    synchronized(queryIdList) {
      queryIdList.remove(queryId);
      profileValues.remove(queryId);
    }
  }

  private static void removeLatestValue() {
    if (queryIdList.size() > 10) {
      for (int i = 10; i < queryIdList.size(); i++) {
        QueryId removedQueryId = queryIdList.remove(0);
        profileValues.remove(removedQueryId);
      }
    }
  }

  public static class QueryProfileMetrics {
    private String operatorName;
    private Map<String, AtomicLong> metricsValues = new HashMap<String, AtomicLong>();
    private Map<String, Long> minValues = new HashMap<String, Long>();
    private Map<String, Long> maxValues = new HashMap<String, Long>();

    public QueryProfileMetrics(String operatorName) {
      this.operatorName = operatorName;
    }
    public void addValue(String metricsName, long metricsValue) {
      AtomicLong value = metricsValues.get(metricsName);
      if (value == null) {
        metricsValues.put(metricsName, new AtomicLong(metricsValue));
        minValues.put(metricsName, metricsValue);
        maxValues.put(metricsName, metricsValue);
      } else {
        value.getAndAdd(metricsValue);
        if (metricsValue < minValues.get(metricsName)) {
          minValues.put(metricsName, metricsValue);
        }

        if (metricsValue > maxValues.get(metricsName)) {
          maxValues.put(metricsName, metricsValue);
        }
      }
    }

    public String toInfoString() {
      StringBuilder sb = new StringBuilder();
      for (String eachKey: metricsValues.keySet()) {
        sb.append("\t").append(eachKey).append(": ").append(metricsValues.get(eachKey));
        sb.append("[").append(minValues.get(eachKey)).append(" ~ ").append(maxValues.get(eachKey)).append("]\n");
      }

      return sb.toString();
    }

    public String toJSONString() {
        StringBuilder sb = new StringBuilder();

        sb.append('[');

        for (String key: metricsValues.keySet()) {
            sb.append('[');
            sb.append('"').append(key).append('"');
            sb.append(',');
            sb.append(metricsValues.get(key)).append(',').
                    append(minValues.get(key)).append(',').
                    append(maxValues.get(key)).append("],");
        }
        sb.deleteCharAt(sb.length()-1);

        sb.append(']');

        return sb.toString();
    }

    public QueryProfileMetricsProto getProto() {
      try {
        List<KeyValueProto> values = new ArrayList<KeyValueProto>();
        for (Map.Entry<String, AtomicLong> entry : metricsValues.entrySet()) {
          values.add(KeyValueProto.newBuilder().setKey(entry.getKey()).setValue("" + entry.getValue().get()).build());
        }

        List<KeyValueProto> mins = new ArrayList<KeyValueProto>();
        for (Map.Entry<String, Long> entry : minValues.entrySet()) {
          mins.add(KeyValueProto.newBuilder().setKey(entry.getKey()).setValue("" + entry.getValue()).build());
        }

        List<KeyValueProto> maxs = new ArrayList<KeyValueProto>();
        for (Map.Entry<String, Long> entry : maxValues.entrySet()) {
          maxs.add(KeyValueProto.newBuilder().setKey(entry.getKey()).setValue("" + entry.getValue()).build());
        }

        return QueryProfileMetricsProto.newBuilder()
            .addAllMetricsValues(values)
            .addAllMinValues(mins)
            .addAllMaxValues(maxs)
            .build();
      } catch (Exception e) {
        e.printStackTrace();
        return null;
      }
    }

    public void setMinValue(String key, long value) {
      if (minValues.containsKey(key)) {
        if (value < minValues.get(key)) {
          minValues.put(key, value);
        }
      } else {
        minValues.put(key, value);
      }
    }

    public void setMaxValue(String key, long value) {
      if (maxValues.containsKey(key)) {
        if (value > maxValues.get(key)) {
          maxValues.put(key, value);
        }
      } else {
        maxValues.put(key, value);
      }
    }
  }
}
