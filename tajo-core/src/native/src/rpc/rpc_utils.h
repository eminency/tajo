#include <google/protobuf/stubs/common.h>

#ifndef TAJO_TEST_RPC_UTILS_H_
#define TAJO_TEST_RPC_UTILS_H_

namespace tajo {

namespace rpc {

void nullCallback() {
}

static google::protobuf::Closure *null_callback =
    google::protobuf::NewCallback(nullCallback);

}

} // namespace tajo

#endif // TAJO_TEST_RPC_UTILS_H_