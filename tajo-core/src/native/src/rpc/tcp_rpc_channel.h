#ifndef TAJO_TCP_RPC_CHANNEL_H_
#define TAJO_TCP_RPC_CHANNEL_H_

#include <map>
#include <mutex>
#include <thread>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include "common/status.h"
#include "rpc/tcp_rpc_controller.h"

namespace tajo {

class RpcRequest;

class ResponseCallback {
 public:
  ResponseCallback(google::protobuf::RpcController* controller,
                   google::protobuf::Message* response,
                   google::protobuf::Closure* closure,
                   boost::condition_variable* cv) :
                     controller_(controller),
                     response_(response),
                     closure_(closure),
                     cv_(cv),
  done_(false) {}

  void Done() {
    if (cv_) {
      cv_->notify_one();
      done_.exchange(true);
    } else if (closure_) {
      closure_->Run();
    }
  }
  google::protobuf::RpcController* controller() { return controller_; }
  google::protobuf::Message* response() { return response_; }
  inline std::atomic<bool> done() const {
    return done_;
  }
 private:
  google::protobuf::RpcController* controller_;
  google::protobuf::Message* response_;
  google::protobuf::Closure* closure_;
  volatile std::atomic<bool> done_;
  boost::condition_variable* cv_;
};

class TcpRpcChannel : public google::protobuf::RpcChannel {
 public:
  TcpRpcChannel(const std::string& endpoint, bool blocking = false);
  virtual ~TcpRpcChannel();

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

  static const int PACKET_HEADER_SIZE;
  static const int BUFFER_SIZE = 2 * 1024 * 1024;

  void Close();

  static int DecodeHeader(const uint8_t* buf);
  static void EncodeHeader(std::ostream& stream, int size);

 private:
  const std::string& endpoint_;
  bool blocking_;

  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::deadline_timer deadline_;
  boost::system::error_code error_code_;
  boost::scoped_ptr<std::thread> ios_thread_;
  boost::scoped_ptr<TcpRpcController> owned_controller_;
  boost::asio::strand strand_;

  std::atomic<int32_t> sequence_;
  std::map<int32_t, ResponseCallback*> running_requests_;

  uint8_t buffer_[BUFFER_SIZE];
  int msg_size_;

  Status last_status_;
  std::mutex map_lock_;
  std::mutex lock_;

  RpcRequest* BuildRequest(int32_t id,
                                  const google::protobuf::MethodDescriptor* method,
                                  const google::protobuf::Message* request);

  // Socket Operations
  void Connect();
  void DoReadHeader();
  void DoWrite(RpcRequest *message);

  inline void HandleWrite(const boost::system::error_code &error,
      std::size_t bytes_transferred) {
    last_status_ = !error? Status::OK : Status::UNKNOWN_ERROR;
  }

  void HandleReadHeader(const boost::system::error_code &error,
      std::size_t bytes_len);
  void HandleReadMessage(const boost::system::error_code &error,
      std::size_t bytes_len);
  void HandleConnect(const boost::system::error_code &e);
  void CheckDeadline(const boost::system::error_code &e) {

    if (e != boost::asio::error::operation_aborted) {
      if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
        // The deadline has passed.
        LOG(ERROR) << "Timeout Occurred";
        Close();
        error_code_ = boost::asio::error::timed_out;
      }
    }

    deadline_.async_wait(boost::bind(&TcpRpcChannel::CheckDeadline, this, boost::asio::placeholders::error));
  }
};

}

#endif
