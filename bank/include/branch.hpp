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

  void start();

 protected:
  /*                           name       , ip         , port    */
  using peer_info = std::tuple<std::string, std::string, uint16_t>;
  using channel_info = std::pair<std::string, std::shared_ptr<Channel>>;

  void message_handler(const BranchMessage &msg, const channel_info &ci);

  void do_send_money();

  void do_accept();

  // void match_peers();

  void init_branch_handler(const InitBranch &msg, const channel_info &ci);

  void init_snapshot_handler(const InitSnapshot &msg, const channel_info &ci);

  void marker_handler(const Marker &msg, const channel_info &ci);

  void retrieve_snapshot_handler(const RetrieveSnapshot &msg, const channel_info &ci);

  void return_snapshot_handler(const ReturnSnapshot &msg, const channel_info &ci);

  void transfer_handler(const Transfer &msg, const channel_info &ci);

  void hello_handler(const Hello &msg, const channel_info &ci);

 private:
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
