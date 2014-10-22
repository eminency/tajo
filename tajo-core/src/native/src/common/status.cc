#include "common/status.h"

namespace tajo {

const Status Status::UNKNOWN_ERROR(CODE_UNKNOWN_ERR);
const Status Status::OK(CODE_OK);
const Status Status::EOS(CODE_EOS, "End of Stream");
const Status Status::CONNECTION_REFUSED(CODE_CONNECTION_REFUSED, "Connection Refused");

}
