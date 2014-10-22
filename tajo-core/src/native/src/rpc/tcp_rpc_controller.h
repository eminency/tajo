#ifndef TAJO_RPC_TCP_RPC_CONTROLLER_H_
#define TAJO_RPC_TCP_RPC_CONTROLLER_H_

#include <google/protobuf/service.h>

namespace tajo {

class TcpRpcController : public google::protobuf::RpcController {
 public:
  TcpRpcController(int32_t id = -1) : id_(id), canceled_(false), callback_(NULL) {}
  virtual ~TcpRpcController() {}

  int32_t id() { return id_; }
  virtual void Reset() {
    canceled_ = false;
    callback_ = NULL;
  }
  virtual bool Failed() const { return !error_text_.empty(); }
  virtual std::string ErrorText() const { return error_text_; }
  virtual void StartCancel() { canceled_ = true; }
  virtual void SetFailed(const std::string& reason) { error_text_ = reason; }
  virtual bool IsCanceled() const { return canceled_; }
  virtual void NotifyOnCancel(google::protobuf::Closure* callback) { callback_ = callback; }

 private:
  int32_t id_;
  bool canceled_;
  google::protobuf::Closure* callback_;

  std::string error_text_;
};

}

#endif
