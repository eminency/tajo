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

package org.apache.tajo.util;

public class StopWatch {
  long[] currentMillis;
  long[] currentNano;

  public StopWatch(int size) {
    currentMillis = new long[size];
    currentNano = new long[size];
  }
  public void reset(int index) {
    currentMillis[index] = System.currentTimeMillis();
    currentNano[index] = System.nanoTime();
  }

  public long checkNano(int index) {
    long nano = System.nanoTime();
    long lastValue = currentNano[index];
    currentNano[index] = nano;
    if (lastValue == 0) {
      return 0;
    }
    return nano - lastValue;
  }

  public long checkMillis(int index) {
    long millis = System.currentTimeMillis();
    long lastValue = currentNano[index];
    currentNano[index] = millis;
    if (lastValue == 0) {
      return 0;
    }
    return millis - lastValue;
  }

  public void clearAll() {
    for (int i = 0; i < currentMillis.length; i++) {
      currentMillis[i] = 0;
      currentNano[i] = 0;
    }
  }
}
