#ifndef TAJO_COMMON_STATUS_H_
#define TAJO_COMMON_STATUS_H_

#include <vector>
#include "common/logging.h"
#include "util/compiler.h"
#include <errno.h>

namespace tajo {

class Status {
 public:
  static const int CODE_UNKNOWN_ERR = -1;
  static const int CODE_OK = 0;
  static const int CODE_EOS = 1;
  static const int CODE_CONNECTION_REFUSED = ECONNREFUSED; // from errno

  static const Status UNKNOWN_ERROR;
  static const Status OK;
  static const Status EOS;
  static const Status CONNECTION_REFUSED;

  Status(const int code) : code_(code) {}

  Status(const int code, const std::string& message)
    : code_(code),
      message_(message) {}

  bool ok() const {
    return code_ == CODE_OK;
  }

  int code() const {
    return code_ ;
  }

  const std::string& message() const {
    return message_ ;
  }

 private:
  int code_;
  std::string message_;

};

#define RETURN_IF_ERROR(stmt) \
  do { \
    Status __status__ = (stmt); \
    if (UNLIKELY(!__status__.ok())) return __status__; \
  } while (false)

#define FINALIZE_IF_ERROR(stmt, final_func) \
  do { \
    Status __status__ = (stmt); \
    if (UNLIKELY(!__status__.ok())) { final_func(__status__); return __status__; } \
  } while (false)

} // namespace tajo

#endif
