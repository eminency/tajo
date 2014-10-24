# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

JARS=`ls $HADOOP_HOME/share/hadoop/hdfs/*.jar`
JARS="$JARS `ls $HADOOP_HOME/share/hadoop/hdfs/lib/*.jar`"
JARS="$JARS `ls $HADOOP_HOME/share/hadoop/common/*.jar`"
JARS="$JARS `ls $HADOOP_HOME/share/hadoop/common/lib/*.jar`"

echo $JARS

CLASSES="org.apache.tajo.worker.Task"

CLASSPATH=.
for JAR in $JARS
do
  CLASSPATH=$CLASSPATH:$JAR 
done

for CLASS in $CLASSES
do
  javah -cp $CLASSPATH:./target/classes:../tajo-common/target/classes:../tajo-catalog/tajo-catalog-common/target/classes -d ./src/native/src/jni $CLASS
done
