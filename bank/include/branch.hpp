#ifndef INCLUDE_BRANCH_HPP_
#define INCLUDE_BRANCH_HPP_
#include "channel.hpp"
#include "snapshot.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <random>
#include <vector>

class Branch : public std::enable_shared_from_this<Branch> {
 public:
  Branch(std::string name, const boost::asio::ip::tcp::endpoint &endpoint,
         boost::asio::io_service &io_service);

  ~Branch() = default;

  Branch(const Branch &) = delete;
  Branch(Branch &&) = delete;
  Branch &operator=(const Branch &) = delete;
  Branch &operator=(Branch &&) = delete;

  void handle_message(const BranchMessage &msg);

  void start();

 protected:
  std::function<void(const BranchMessage &)> d_message_handler = [this](const BranchMessage &msg) {
    handle_message(msg);
  };

  void do_send_money();

  void do_accept();

  void match_peers();

  void init_branch_handler(const InitBranch &msg);

  void init_snapshot_handler(const InitSnapshot &msg);

  void marker_handler(const Marker &msg);

  void retrieve_snapshot_handler(const RetrieveSnapshot &msg);

  void return_snapshot_handler(const ReturnSnapshot &msg);

  void transfer_handler(const Transfer &msg);

 private:
  /*                           name       , ip         , port    */
  using peer_info = std::tuple<std::string, std::string, uint16_t>;
  const std::string d_name;
  std::unordered_map<std::string, std::shared_ptr<Channel>> d_channels;
  std::vector<peer_info> d_known_peers;
  std::atomic_uint64_t d_money = 0;
  std::optional<Snapshot> d_snapshot;
  boost::asio::io_service d_iosvc;
  boost::asio::ip::tcp::acceptor d_acceptor;
  boost::asio::ip::tcp::socket d_socket;
  std::vector<std::shared_ptr<Channel>> d_pending_peers;
  boost::asio::io_service &d_io_svc;
  boost::asio::steady_timer d_timer;
  std::random_device d_rd;
  std::mt19937_64 d_gen;
  boost::asio::ip::tcp::resolver d_resolver;
  boost::asio::io_service::strand d_strand;
};

#endif
