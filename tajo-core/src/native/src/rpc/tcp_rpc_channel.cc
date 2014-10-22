#include <resolv.h>

#include <common/logging.h>
#include <common/status.h>
#include <rpc/tcp_rpc_channel.h>
#include <RpcProtos.pb.h>

using namespace google::protobuf;
using namespace boost;
using boost::asio::ip::tcp;

namespace tajo {

const int TcpRpcChannel::PACKET_HEADER_SIZE = 4;

TcpRpcChannel::TcpRpcChannel(const string& endpoint, bool blocking):
    endpoint_(endpoint),
    blocking_(blocking),
    socket_(io_service_),
    deadline_(io_service_),
    last_status_(Status::OK),
    strand_(io_service_) {

  owned_controller_.reset(new TcpRpcController());
  deadline_.expires_at(posix_time::pos_infin);
  CheckDeadline(boost::system::error_code());
  Connect();
}

TcpRpcChannel::~TcpRpcChannel() {
  std::map<int32_t, ResponseCallback*>::iterator it = running_requests_.begin();
  while (it != running_requests_.end()) {
    it->second->controller()->SetFailed("Channel Terminated");
    it->second->Done();
    delete it->second;
    ++it;
  }
  if (last_status_.ok()) {
    io_service_.post(boost::bind(&TcpRpcChannel::Close, this));
    io_service_.stop();
    ios_thread_->join();
  } else
    Close();
}

void TcpRpcChannel::CallMethod(const MethodDescriptor* method,
                          RpcController* controller,
                          const Message* request,
                          Message* response,
                          Closure* done) {
  if (!controller) {
    controller = owned_controller_.get();
  }

  DCHECK(controller != NULL);

  controller->Reset();
  if (!last_status_.ok()) {
    stringstream msg;
    msg << "Connect to " << endpoint_ << "Error";
    controller->SetFailed(msg.str());
    if (done) done->Run();
    return;
  }

  int32_t id = sequence_++;
  RpcRequest* message = BuildRequest(id, method, request);
  boost::mutex m;
  boost::condition_variable blocking_cv;
  boost::unique_lock<boost::mutex> request_lock(m);

  ResponseCallback* response_callback =
      new ResponseCallback(controller, response, done, blocking_? &blocking_cv : NULL);

  map_lock_.lock();
  running_requests_[id] = response_callback;
  map_lock_.unlock();

  io_service_.post(boost::bind(&TcpRpcChannel::DoWrite, this, message));

  if (blocking_) {
    if(!response_callback->done()) {
      blocking_cv.wait(request_lock);
    }
    if (done) {
      done->Run();
    }
  }
}

void TcpRpcChannel::Connect() {

  LOG(INFO) << "Starting to connect RPC Server: " << endpoint_;

  size_t pos = endpoint_.find(':');

  if (pos == string::npos) {
    last_status_ = Status(Status::CODE_UNKNOWN_ERR,
        "Endpoint must be host:port format");
    return;
  }

  string port = endpoint_.substr(pos + 1);
  string host = endpoint_.substr(0, pos);
  //res_init();
  tcp::resolver::query query(tcp::v4(), host, port);
  tcp::resolver::iterator iter = tcp::resolver(io_service_).resolve(query);

  while(iter != tcp::resolver::iterator()) {
    Close();
    socket_.open(tcp::v4());
    //socket_.set_option(asio::socket_base::receive_buffer_size(8*1024*1024));
    socket_.set_option(tcp::no_delay(true));
    deadline_.expires_from_now(posix_time::seconds(5)); // TODO Configure this value
    error_code_ = asio::error::would_block;
    tcp::endpoint endpoint = *iter;
    socket_.async_connect(endpoint,
               boost::bind(&TcpRpcChannel::HandleConnect, this, asio::placeholders::error));

    do {
      io_service_.run_one();
    } while (error_code_ == boost::asio::error::would_block ||
      error_code_ == boost::asio::error::operation_aborted);

    if (!error_code_) break;
    iter++;
  }
  if (error_code_ || !socket_.is_open()) {
    LOG(ERROR) << "Connect Error: " << error_code_;
    last_status_ = Status(Status::CODE_UNKNOWN_ERR, "Connect Error");
    return;
  }

  deadline_.expires_at(posix_time::pos_infin);

  DoReadHeader();
  ios_thread_.reset(new std::thread(boost::bind(&boost::asio::io_service::run,
      &io_service_)));

}

void TcpRpcChannel::HandleConnect(const boost::system::error_code &error) {

  if (!error) {
    LOG(INFO) << "Succeed to connect RPC Server: " << endpoint_;
  } else {
    LOG(INFO) << "Failed to connect RPC Server: " << endpoint_
        << " code: " << error;
  }

  error_code_ = error;
}

void TcpRpcChannel::DoReadHeader() {

  asio::async_read(
      socket_, asio::buffer(buffer_, PACKET_HEADER_SIZE),
      strand_.wrap(
          boost::bind(&TcpRpcChannel::HandleReadHeader,
          this, asio::placeholders::error,
          asio::placeholders::bytes_transferred())));
}

void TcpRpcChannel::HandleReadHeader(const boost::system::error_code &error,
    std::size_t bytes_len) {
  if (!error) {

    // assert
    CHECK_EQ(bytes_len, PACKET_HEADER_SIZE) <<
          "Invalid Packet Header Length: " << bytes_len;

   msg_size_ = DecodeHeader(buffer_);
    asio::async_read(socket_, asio::buffer(buffer_, msg_size_),
        strand_.wrap(
            boost::bind(&TcpRpcChannel::HandleReadMessage,
            this, asio::placeholders::error,
            asio::placeholders::bytes_transferred())));
  } else {
    this->last_status_ = Status::UNKNOWN_ERROR;
    Close();
  }
}

void TcpRpcChannel::HandleReadMessage(const boost::system::error_code &error,
    std::size_t bytes_len) {
  if (!error) {
    CHECK_EQ(bytes_len, msg_size_) <<
          "Invalid Packet Header Length: " << msg_size_;

    RpcResponse response;
    response.ParseFromArray(buffer_, msg_size_);

    ResponseCallback *callback = NULL;

    int32_t response_id = response.id();
    std::map<int32_t, ResponseCallback*>::iterator it;

    map_lock_.lock();

    it = running_requests_.find(response_id);
    if (it != running_requests_.end()) {
      callback = it->second;
      running_requests_.erase(it);
    }

    map_lock_.unlock();

    if (callback) {
      Message* result = callback->response();

      if (result) {
        result->ParseFromString(response.response_message());
      }

      if (callback) {
        callback->Done();
        delete callback;
      }
    }

    // Chainning the next read
    DoReadHeader();
  } else {
    last_status_ = Status::UNKNOWN_ERROR;
    Close();
  }
}

void TcpRpcChannel::DoWrite(RpcRequest *message) {

  int msg_size = message->ByteSize();
  asio::streambuf buffer;
  buffer.prepare(PACKET_HEADER_SIZE + msg_size);
  ostream request_stream(&buffer);

  EncodeHeader(request_stream, msg_size);
  message->SerializeToOstream(&request_stream);
  delete message; // free allocated message

  asio::async_write(socket_,
      asio::buffer(
          asio::buffer_cast<const char*>(buffer.data()), buffer.size()),
      boost::bind(&TcpRpcChannel::HandleWrite, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred));

}

void TcpRpcChannel::Close() {
  deadline_.expires_at(posix_time::pos_infin);
  boost::system::error_code ignored_ec;
  socket_.close(ignored_ec);
}

RpcRequest* TcpRpcChannel::BuildRequest(int32_t id,
                                    const MethodDescriptor* method,
                                    const Message* request) {
  RpcRequest* rpc = new RpcRequest;
  rpc->set_id(id);
  rpc->set_method_name(method->name());
  if (request) {
    rpc->set_request_message(request->SerializeAsString());
  }

  return rpc;

}

void TcpRpcChannel::EncodeHeader(ostream& stream, int size) {
  uint8_t buf[PACKET_HEADER_SIZE];
  buf[0] = static_cast<uint8_t>((size >> 24) & 0xFF);
  buf[1] = static_cast<uint8_t>((size >> 16) & 0xFF);
  buf[2] = static_cast<uint8_t>((size >> 8) & 0xFF);
  buf[3] = static_cast<uint8_t>(size & 0xFF);
  stream.write(reinterpret_cast<const char*>(buf), PACKET_HEADER_SIZE);
}

int TcpRpcChannel::DecodeHeader(const uint8_t* buf) {
  int size = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];

  return size;
}

} // namespace tajo
