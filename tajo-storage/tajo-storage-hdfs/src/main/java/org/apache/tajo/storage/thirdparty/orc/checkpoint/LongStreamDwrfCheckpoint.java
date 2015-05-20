/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.apache.tajo.storage.thirdparty.orc.checkpoint;

import com.google.common.base.MoreObjects;
import org.apache.tajo.storage.thirdparty.orc.checkpoint.Checkpoints.ColumnPositionsList;
import org.apache.tajo.storage.thirdparty.orc.metadata.CompressionKind;

import static org.apache.tajo.storage.thirdparty.orc.checkpoint.InputStreamCheckpoint.createInputStreamCheckpoint;
import static org.apache.tajo.storage.thirdparty.orc.checkpoint.InputStreamCheckpoint.inputStreamCheckpointToString;

public final class LongStreamDwrfCheckpoint
        implements LongStreamCheckpoint
{
    private final long inputStreamCheckpoint;

    public LongStreamDwrfCheckpoint(long inputStreamCheckpoint)
    {
        this.inputStreamCheckpoint = inputStreamCheckpoint;
    }

    public LongStreamDwrfCheckpoint(CompressionKind compressionKind, ColumnPositionsList positionsList)
    {
        inputStreamCheckpoint = createInputStreamCheckpoint(compressionKind, positionsList);
    }

    public long getInputStreamCheckpoint()
    {
        return inputStreamCheckpoint;
    }

    @Override
    public String toString()
    {
        return MoreObjects.toStringHelper(this)
                .add("inputStreamCheckpoint", inputStreamCheckpointToString(inputStreamCheckpoint))
                .toString();
    }
}