// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <jni.h>

#include "org_apache_tajo_worker_Task.h"

JNIEXPORT void JNICALL 
Java_org_apache_tajo_worker_Task_runJsonPlan
(JNIEnv *env, jobject obj, jstring json_str)
{
  const char *str = env->GetStringUTFChars(json_str, NULL);
  printf("XXXX %s\n", str);
  env->ReleaseStringUTFChars(json_str, str);
}

JNIEXPORT void JNICALL
Java_org_apache_tajo_worker_Task_test
(JNIEnv *env, jobject obj) 
{
  printf("I'm test\n");
}
