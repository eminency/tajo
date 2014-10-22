#ifndef TAJO_RPC_TCP_RPC_SERVER_H_
#define TAJO_RPC_TCP_RPC_SERVER_H_

#include "rpc/tcp_rpc_channel.h"
#include "rpc/tcp_rpc_controller.h"

namespace tajo {

class RpcSession {
 public:
  RpcSession(boost::asio::io_service& io_service, google::protobuf::Service* service) :
    socket_(io_service),
    service_(service) {
    header_buffer_.reset(new uint8_t[TcpRpcChannel::PACKET_HEADER_SIZE]);
  }

  ~RpcSession() {
  }

  boost::asio::ip::tcp::socket& socket() { return socket_; }
  void Start();

 private:
  boost::asio::ip::tcp::socket socket_;
  google::protobuf::Service* service_;

  boost::scoped_ptr<uint8_t> header_buffer_;
  boost::scoped_ptr<uint8_t> body_buffer_;
  int body_size_;

  boost::system::error_code error_code_;

  void Close();

  void HandleReadHeader(const boost::system::error_code &e);
  void HandleReadMessage(const boost::system::error_code &e);
  void HandleWrite(const boost::system::error_code &e, size_t bytes_transferred) {
    error_code_ = e;
  }

  void HandleCallDone(TcpRpcController* controller, google::protobuf::Message* result);

  void RunOnThread(const int32_t id,
                   const google::protobuf::MethodDescriptor* method_desc,
                   google::protobuf::Message* message); 
};

class TcpRpcServer {
 public:
  TcpRpcServer(int port);
  ~TcpRpcServer();
  // register state machines and run boost::asio::io_service on a separate thread
  Status Init(google::protobuf::Service* service);
  Status Start();
  void Join();

  // stop io_service thread
  Status Stop();

  google::protobuf::Service* service() { return service_; }

 private:
  int port_;
  google::protobuf::Service* service_;

  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;

  boost::scoped_ptr<boost::thread> ios_thread_;

  void Accept();
  void HandleAccept(RpcSession* session, const boost::system::error_code& e);
  void HandleSignal(int signum);
};


}

#endif
