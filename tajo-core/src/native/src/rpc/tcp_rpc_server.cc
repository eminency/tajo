#include  <csignal>
#include "common/logging.h"
#include "rpc/tcp_rpc_server.h"
#include "RpcProtos.pb.h"

using namespace std;
using namespace boost;
using namespace google::protobuf;

using boost::asio::ip::tcp;

namespace tajo {

TcpRpcServer::TcpRpcServer(int port) :
    port_(port),
    acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {

}

TcpRpcServer::~TcpRpcServer() {

}

Status TcpRpcServer::Init(Service* service) {
  service_ = service;
  return Status::OK;
}

Status TcpRpcServer::Start() {
  Accept();
  ios_thread_.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));
  LOG(INFO) << "Started RPC Server on port " << port_ ;

  return Status::OK;
}

void TcpRpcServer::Join() {
  ios_thread_->join();
}

Status TcpRpcServer::Stop() {
  LOG(INFO) << "Stopping RPC Server" ;
  io_service_.stop();
  ios_thread_->join();
  LOG(INFO) << "Stopped RPC Server" ;

  return Status::OK;
}

void TcpRpcServer::Accept() {
  RpcSession* session = new RpcSession(io_service_, service_);
  acceptor_.async_accept(session->socket(),
          boost::bind(&TcpRpcServer::HandleAccept, this, session,
            asio::placeholders::error));
}

void TcpRpcServer::HandleSignal(int signum) {
  Stop();
}

void TcpRpcServer::HandleAccept(RpcSession* session, const system::error_code& error) {
  if (!error) {
    session->Start();
    //TODO: Add session to SessionManager
  } else {
    delete session;
  }

  Accept();
}

void RpcSession::Start() {
  asio::async_read(socket_,
          asio::buffer(header_buffer_.get(), TcpRpcChannel::PACKET_HEADER_SIZE),
          boost::bind(&RpcSession::HandleReadHeader, this, asio::placeholders::error));
}

void RpcSession::HandleReadHeader(const system::error_code &error) {
  if (!error) {
    body_size_ = TcpRpcChannel::DecodeHeader(header_buffer_.get());
    body_buffer_.reset(new uint8_t[body_size_]);
    asio::async_read(socket_, asio::buffer(body_buffer_.get(), body_size_),
                    boost::bind(&RpcSession::HandleReadMessage, this, asio::placeholders::error));
  } else {
    Close();
  }
}

void RpcSession::HandleReadMessage(const system::error_code &error) {
  if (!error) {
    RpcRequest request;
    request.ParseFromArray(body_buffer_.get(), body_size_);

    const int32_t id = request.id();
    const string& method_name = request.method_name();
    VLOG(3) << "Method Invoked " << method_name;
    const ServiceDescriptor* svc_desc = service_->GetDescriptor();
    const MethodDescriptor* method_desc =  svc_desc->FindMethodByName(method_name);
    if (method_desc == NULL) {
      LOG(ERROR) << "Cannot Find Method " << method_name;
      Start();
      return;
    }
    Message* message = service_->GetRequestPrototype(method_desc).New();
    message->ParseFromString(request.request_message());
    boost::thread(boost::bind(&RpcSession::RunOnThread, this, id, method_desc, message));

    Start();
  } else {
    Close();
  }
}

void RpcSession::RunOnThread(const int32_t id,
                             const MethodDescriptor* method_desc,
                             Message* message) {
  TcpRpcController* controller = new TcpRpcController(id);
  Message* result = service_->GetResponsePrototype(method_desc).New();
  Closure *callback = NewCallback(this, &RpcSession::HandleCallDone, controller, result);

  service_->CallMethod(method_desc, controller,
                      message,
                      result,
                      callback);

  delete message;
}

void RpcSession::HandleCallDone(TcpRpcController* controller, Message* result) {

  RpcResponse response;
  response.set_id(controller->id());
  response.set_response_message(result->SerializeAsString());

  int msg_size = response.ByteSize();
  asio::streambuf buffer;
  buffer.prepare(TcpRpcChannel::PACKET_HEADER_SIZE + msg_size);
  ostream response_stream(&buffer);

  TcpRpcChannel::EncodeHeader(response_stream, msg_size);
  response.SerializeToOstream(&response_stream);

  delete result;
  delete controller;

  asio::async_write(socket_, asio::buffer(asio::buffer_cast<const char*>(buffer.data()), buffer.size()),
                  boost::bind(&RpcSession::HandleWrite, this, asio::placeholders::error, asio::placeholders::bytes_transferred));

}

void RpcSession::Close() {
  if (socket_.is_open())
    socket_.close();
  //TODO: Remove session from SessionManager
  delete this;
}

} // namespace tajo
