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

package org.apache.tajo.storage;

public abstract class ProfileContext {
  public static ThreadLocal<ProfileContext> contextThreadLocal = new ThreadLocal<ProfileContext>();

  private boolean enabledProfile;

  public abstract void addProfileMetrics(String operationName, String[] metricsKeys, long[] values);
  public abstract String getId();

  public boolean isEnabledProfile() {
    return enabledProfile;
  }

  public ProfileContext(boolean enabledProfile) {
    this.enabledProfile = enabledProfile;

    contextThreadLocal.set(this);
  }

  public static class DummyProfileContext extends ProfileContext {
    public DummyProfileContext(boolean enabledProfile) {
      super(enabledProfile);
    }

    @Override
    public void addProfileMetrics(String operationName, String[] metricsKeys, long[] values) {
    }

    @Override
    public String getId() {
      return null;
    }
  }
}
