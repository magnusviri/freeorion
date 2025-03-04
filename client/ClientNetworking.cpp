#define WIN32_LEAN_AND_MEAN

#include "ClientNetworking.h"

#include "../network/Message.h"
#include "../network/MessageQueue.h"

// boost::asio pulls in windows.h which in turn defines the macros GetMessage,
// SendMessage, min and max. Disabling the generation of the min and max macros
// and undefining those should avoid name collisions with std c++ library and
// FreeOrion function names.
#define NOMINMAX
#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#ifdef FREEORION_WIN32
#   undef GetMessage
#   undef SendMessage
#endif

#include "../network/Networking.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/optional/optional.hpp>

#include <thread>

using boost::asio::ip::tcp;
using namespace Networking;

namespace {
    DeclareThreadSafeLogger(network);

    /** A simple client that broadcasts UDP datagrams on the local network for
        FreeOrion servers, and reports any it finds. */
    class ServerDiscoverer {
    public:
        using ServerList = std::vector<std::pair<boost::asio::ip::address, std::string>>;

        ServerDiscoverer(boost::asio::io_context& io_context) :
            m_io_context(&io_context),
            m_timer(io_context),
            m_socket(io_context),
            m_recv_buf(),
            m_receive_successful(false),
            m_server_name()
        {}

        const ServerList& Servers() const
        { return m_servers; }

        void DiscoverServers() {
            using namespace boost::asio::ip;
            udp::resolver resolver(*m_io_context);
            udp::resolver::query query(udp::v4(), "255.255.255.255",
                                       std::to_string(Networking::DiscoveryPort()),
                                       resolver_query_base::address_configured |
                                       resolver_query_base::numeric_service);
            udp::resolver::iterator end_it;
            for (udp::resolver::iterator it = resolver.resolve(query); it != end_it; ++it) {
                udp::endpoint receiver_endpoint = *it;

                m_socket.close();
                m_socket.open(udp::v4());
                m_socket.set_option(boost::asio::socket_base::broadcast(true));

                m_socket.send_to(boost::asio::buffer(DISCOVERY_QUESTION),
                                 receiver_endpoint);

                m_socket.async_receive_from(
                    boost::asio::buffer(m_recv_buf),
                    m_sender_endpoint,
                    boost::bind(&ServerDiscoverer::HandleReceive,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

                m_timer.expires_after(std::chrono::seconds(2));
                m_timer.async_wait(boost::bind(&ServerDiscoverer::CloseSocket, this));
                m_io_context->run();
                m_io_context->reset();
                if (m_receive_successful) {
                    boost::asio::ip::address address = m_server_name == "localhost" ?
                        boost::asio::ip::address::from_string("127.0.0.1") :
                        m_sender_endpoint.address();
                    m_servers.push_back({address, m_server_name});
                }
                m_receive_successful = false;
                m_server_name.clear();
            }
        }

    private:
        void HandleReceive(const boost::system::error_code& error, std::size_t length) {
            if (error == boost::asio::error::message_size) {
                m_socket.async_receive_from(
                    boost::asio::buffer(m_recv_buf),
                    m_sender_endpoint,
                    boost::bind(&ServerDiscoverer::HandleReceive,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

            } else if (!error) {
                std::string buffer_string(m_recv_buf.begin(), m_recv_buf.begin() + length);
                if (boost::algorithm::starts_with(buffer_string, DISCOVERY_ANSWER)) {
                    m_receive_successful = true;
                    m_server_name =
                        boost::algorithm::erase_first_copy(buffer_string, DISCOVERY_ANSWER);
                    if (m_server_name == boost::asio::ip::host_name())
                        m_server_name = "localhost";
                    m_timer.cancel();
                    m_socket.close();
                }
            }
        }

        void CloseSocket()
        { m_socket.close(); }

        boost::asio::io_context*            m_io_context;
        boost::asio::high_resolution_timer  m_timer;
        boost::asio::ip::udp::socket        m_socket;

        std::array<char, 1024>              m_recv_buf;

        boost::asio::ip::udp::endpoint      m_sender_endpoint;
        bool                                m_receive_successful;
        std::string                         m_server_name;
        ServerList                          m_servers;
    };
}



class ClientNetworking::Impl {
public:
    /** The type of list returned by a call to DiscoverLANServers(). */
    using ServerList = std::vector<std::pair<boost::asio::ip::address, std::string>>;

    Impl();

    /** Returns true iff the client is full duplex connected to the server. */
    bool IsConnected() const;

    /** Returns true iff the client is connected to receive from the server. */
    bool IsRxConnected() const;

    /** Returns true iff the client is connected to send to the server. */
    bool IsTxConnected() const;

    /** Returns the ID of the player on this client. */
    int PlayerID() const;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    int HostPlayerID() const;

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const;

    /** Checks if the client has some authorization \a role. */
    bool HasAuthRole(Networking::RoleType role) const;

    /** Returns destination address of server. */
    const std::string& Destination() const;

    /** Returns a list of the addresses and names of all servers on the Local
        Area Network. */
    ClientNetworking::ServerNames DiscoverLANServerNames();

    /** Connects to the server at \a ip_address.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. If \p
        expect_timeout is true, timeout is not reported as an error. */
    bool ConnectToServer(const ClientNetworking* const self,
                         const std::string& ip_address,
                         const std::chrono::milliseconds& timeout = std::chrono::seconds(10),
                         bool expect_timeout = false);

    /** Connects to the server on the client's host.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. If \p
        expect_timeout is true, timeout is not reported as an error.*/
    bool ConnectToLocalHostServer(
        const ClientNetworking* const self,
        const std::chrono::milliseconds& timeout = std::chrono::seconds(10),
        bool expect_timeout = false);

    /** Sends \a message to the server. This function actually just enqueues
        the message for sending and returns immediately. */
    void SendMessage(Message&& message);

    /** Adds a message to this client's incoming messages queue. */
    void SendSelfMessage(Message&& message);

    /** Return the next incoming message from the server if available or boost::none.
        Remove the message from the incoming message queue. */
    boost::optional<Message> GetMessage();

    /** Disconnects the client from the server. */
    void DisconnectFromServer();

    /** Sets player ID for this client. */
    void SetPlayerID(int player_id);

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id);

    /** Get authorization roles access. */
    Networking::AuthRoles& AuthorizationRoles();

private:
    void HandleException(const boost::system::system_error& error);
    void HandleConnection(const boost::system::error_code& error,
                          tcp::resolver::iterator endpoint_it);
    void HandleResolve(const boost::system::error_code& error, tcp::resolver::iterator results);
    void HandleDeadlineTimeout(const boost::system::error_code& error);

    void NetworkingThread(const std::shared_ptr<const ClientNetworking> self);
    void HandleMessageBodyRead(const std::shared_ptr<const ClientNetworking>& keep_alive,
                               boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(const std::shared_ptr<const ClientNetworking>& keep_alive,
                                 boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage(const std::shared_ptr<const ClientNetworking>& keep_alive);
    void HandleMessageWrite(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncWriteMessage();
    void SendMessageImpl(Message message);
    void DisconnectFromServerImpl();
    bool _IsConnected() const;  // Non-thread-safe: Return true iff the client is full duplex connected to the server.
    bool CloseSocketIfNotConnected();  // Close the socket iff the client is not fully duplex connected to the server.
    void LaunchNetworkThread(const ClientNetworking* const self);

    int                             m_player_id = Networking::INVALID_PLAYER_ID;
    int                             m_host_player_id = Networking::INVALID_PLAYER_ID;
    Networking::AuthRoles           m_roles;

    boost::asio::io_context            m_io_context;
    boost::asio::ip::tcp::socket       m_socket;
    boost::asio::high_resolution_timer m_deadline_timer;
    boost::asio::high_resolution_timer m_reconnect_timer;
    tcp::resolver::iterator            m_resolver_results;
    bool                               m_deadline_has_expired{ false };

    // m_mutex guards m_incoming_message, m_rx_connected and m_tx_connected which are written by
    // the networking thread and read by the main thread to check incoming messages and connection
    // status. As those read and write operations are not atomic, shared access has to be
    // protected to prevent unpredictable results.
    mutable std::mutex              m_mutex;

    bool                            m_rx_connected = false; // accessed from multiple threads
    bool                            m_tx_connected = false; // accessed from multiple threads

    MessageQueue                    m_incoming_messages;    // accessed from multiple threads, but its interface is threadsafe
    std::list<Message>              m_outgoing_messages;

    Message::HeaderBuffer           m_incoming_header = {};
    Message                         m_incoming_message;
    Message::HeaderBuffer           m_outgoing_header = {};

    std::string                     m_destination;
};


////////////////////////////////////////////////
// ClientNetworking Impl
////////////////////////////////////////////////
ClientNetworking::Impl::Impl() :
    m_socket(m_io_context),
    m_deadline_timer(m_io_context),
    m_reconnect_timer(m_io_context),
    m_incoming_messages(m_mutex)
{}

bool ClientNetworking::Impl::IsConnected() const {
    std::scoped_lock lock(m_mutex);
    return _IsConnected();
}

bool ClientNetworking::Impl::_IsConnected() const {
    return m_rx_connected && m_tx_connected;
}

bool ClientNetworking::Impl::CloseSocketIfNotConnected() {
    std::scoped_lock lock(m_mutex);
    bool do_close = !_IsConnected();
    if (do_close)
        m_socket.close();
    
    return do_close;
}

bool ClientNetworking::Impl::IsRxConnected() const {
    std::scoped_lock lock(m_mutex);
    return m_rx_connected;
}

bool ClientNetworking::Impl::IsTxConnected() const {
    std::scoped_lock lock(m_mutex);
    return m_tx_connected;
}

int ClientNetworking::Impl::PlayerID() const
{ return m_player_id; }

int ClientNetworking::Impl::HostPlayerID() const
{ return m_host_player_id; }

bool ClientNetworking::Impl::PlayerIsHost(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return false;
    return player_id == m_host_player_id;
}

bool ClientNetworking::Impl::HasAuthRole(Networking::RoleType role) const
{ return m_roles.HasRole(role); }

ClientNetworking::ServerNames ClientNetworking::Impl::DiscoverLANServerNames() {
    if (!IsConnected())
        return ServerNames();
    ServerDiscoverer discoverer(m_io_context);
    discoverer.DiscoverServers();
    ServerNames names;
    for (const auto& server : discoverer.Servers()) {
        names.push_back(server.second);
    }
    return names;
}

const std::string& ClientNetworking::Impl::Destination() const
{ return m_destination; }


void ClientNetworking::Impl::LaunchNetworkThread(const ClientNetworking* const self)
{
    // Prepare the socket
    
    // linger option has different meanings on different platforms.  It affects the
    // behavior of the socket.close().  It can do the following:
    // - close both send and receive immediately,
    // - finish sending any pending send packets and wait up to SOCKET_LINGER_TIME for
    // ACKs,
    // - finish sending pending sent packets and wait up to SOCKET_LINGER_TIME for ACKs
    // and for the other side of the connection to close,
    // linger may/may not cause close() to block until the linger time has elapsed.
    TraceLogger(network) << "ClientNetworking::Impl::LaunchNetworkThread(" << self << ")";
    m_socket.set_option(boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));

    // keep alive is an OS dependent option that will keep the TCP connection alive and
    // then deliver an OS dependent error when/if the other side of the connection
    // times out or closes.
    m_socket.set_option(boost::asio::socket_base::keep_alive(true));

    DebugLogger(network) << "ConnectToServer() : starting networking thread";
    boost::thread(boost::bind(&ClientNetworking::Impl::NetworkingThread, this, self->shared_from_this()));
}


bool ClientNetworking::Impl::ConnectToServer(
    const ClientNetworking* const self,
    const std::string& ip_address,
    const std::chrono::milliseconds& timeout,
    bool expect_timeout)
{
    TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer(" << self << ", " << ip_address << ", " << timeout.count() << ", " << expect_timeout << ")";
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start_time = Clock::now();

    using namespace boost::asio::ip;
    tcp::resolver resolver(m_io_context);
    tcp::resolver::query query(ip_address,
                               std::to_string(Networking::MessagePort()),
                               resolver_query_base::numeric_service);
    
    // Resolve the query - will try to connect on success.
    resolver.async_resolve(query, [this](const auto& err, const auto& results) {
        HandleResolve(err, results); 
    });

    TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer() - Resolving...";
    m_io_context.run_one();
    TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer() - Resolved.";
    // configure the deadline timer to close socket and cancel connection attempts at timeout
    m_deadline_has_expired = false;
    m_deadline_timer.expires_from_now(timeout);
    m_deadline_timer.async_wait([this](const auto& err) { HandleDeadlineTimeout(err); });
    
    try {
        TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer() - Starting asio event loop";
        m_io_context.run(); // blocks until connection or timeout
        m_io_context.reset();

        if (IsConnected()) {
            auto connection_time = Clock::now() - start_time;
            DebugLogger(network) << "Connecting to server took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(connection_time).count() << " ms.";
            LaunchNetworkThread(self);
        }
        else {
            TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer() - Could not connect";
            if(!expect_timeout)
                InfoLogger(network) << "ConnectToServer() : failed to connect to server.";
        };

    } catch (const std::exception& e) {
        ErrorLogger(network) << "ConnectToServer() : unable to connect to server at "
                             << ip_address << " due to exception: " << e.what();
    }
    if (IsConnected())
        m_destination = ip_address;
    TraceLogger(network) << "ClientNetworking::Impl::ConnectToServer() - Returning.";
    return IsConnected();
}

bool ClientNetworking::Impl::ConnectToLocalHostServer(
    const ClientNetworking* const self,
    const std::chrono::milliseconds& timeout,
    bool expect_timeout)
{
    TraceLogger(network) << "ClientNetworking::Impl::ConnectToLocalHostServer(" << self << ", " << timeout.count() << ", " << expect_timeout << ")";
    bool retval = false;
#if FREEORION_WIN32
    try {
#endif
        retval = ConnectToServer(self, "127.0.0.1", timeout, expect_timeout);
#if FREEORION_WIN32
    } catch (const boost::system::system_error& e) {
        if (e.code().value() != WSAEADDRNOTAVAIL)
            throw;
    }
#endif
    TraceLogger(network) << "Return from ClientNetworking::Impl::ConnectToLocalHostServer()";
    return retval;
}

void ClientNetworking::Impl::DisconnectFromServer() {
    bool is_open(false);

    { // Create a scope for the mutex
        std::scoped_lock lock(m_mutex);
        is_open = m_rx_connected || m_tx_connected;
    }

    if (is_open)
        m_io_context.post(boost::bind(&ClientNetworking::Impl::DisconnectFromServerImpl, this));
}

void ClientNetworking::Impl::SetPlayerID(int player_id) {
    DebugLogger(network) << "ClientNetworking::SetPlayerID: player id set to: " << player_id;
    m_player_id = player_id;
}

void ClientNetworking::Impl::SetHostPlayerID(int host_player_id)
{ m_host_player_id = host_player_id; }

Networking::AuthRoles& ClientNetworking::Impl::AuthorizationRoles()
{ return m_roles; }

void ClientNetworking::Impl::SendMessage(Message&& message) {
    if (!IsTxConnected()) {
        ErrorLogger(network) << "ClientNetworking::SendMessage can't send message when not transmit connected";
        return;
    }
    TraceLogger(network) << "ClientNetworking::SendMessage() : sending message " << message;
    m_io_context.post(boost::bind(&ClientNetworking::Impl::SendMessageImpl, this, std::move(message)));
}

void ClientNetworking::Impl::SendSelfMessage(Message&& message) {
    TraceLogger(network) << "ClientNetworking::SendSelfMessage() : sending self message " << message;
    m_incoming_messages.PushBack(std::move(message));
}

boost::optional<Message> ClientNetworking::Impl::GetMessage() {
    auto message = m_incoming_messages.PopFront();
    if (message)
        TraceLogger(network) << "ClientNetworking::GetMessage() : received message " << *message;
    return message;
}

void ClientNetworking::Impl::HandleConnection(const boost::system::error_code& error,
                                              tcp::resolver::iterator endpoint_it)
{
    DebugLogger(network) << "ClientNetworking::HandleConnection : " << endpoint_it->host_name();
    if (error == boost::asio::error::operation_aborted) {
        DebugLogger(network) << "ClientNetworking::HandleConnection : Operation aborted.";
        return;
    }
    else if (error) {
        DebugLogger(network) << "ClientNetworking::HandleConnection : connection error #"
                             << error.value() <<" \"" << error.message() << "\""
                             << "... retrying";
        m_socket.close();
        endpoint_it++;
        if (endpoint_it == tcp::resolver::iterator())
        {
            endpoint_it = m_resolver_results;
            m_reconnect_timer.expires_from_now(std::chrono::milliseconds(100));
            m_reconnect_timer.async_wait([this, endpoint_it](const auto& err) {
                // If the m_deadline_timer has expired, it will try to cancel
                // this timer and set the m_deadline_has_expired flag.
                // If expiry of both timers is sufficiently close together
                // this callback may have already been scheduled and this timer
                // can no longer be canceled - so need to check the flag here.
                if (err == boost::asio::error::operation_aborted
                    || m_deadline_has_expired)
                {
                    TraceLogger(network) << "ClientNetworking::Impl::m_reconnect_timer::async_wait - Canceling reconnect attempts due to deadline timeout";
                    return;

                }
                TraceLogger(network) << "ClientNetworking::Impl::m_reconnect_timer::async_wait - Scheduling another connection attempt";
                m_socket.async_connect(*endpoint_it, [this, endpoint_it](const auto& error) {
                    HandleConnection(error, endpoint_it);
                });
            });
        } else {
            m_socket.async_connect(*endpoint_it, [this, endpoint_it](const auto& error) {
                HandleConnection(error, endpoint_it);
            });
        }
    } else {
        m_deadline_timer.cancel();
        const auto& endpoint = endpoint_it->endpoint();
        InfoLogger(network) << "Connected to server at " << endpoint.address() << ":" << endpoint.port();
        std::scoped_lock lock(m_mutex);
        m_rx_connected = true;
        m_tx_connected = true;
    }
}

void ClientNetworking::Impl::HandleResolve(const boost::system::error_code& error, 
                                           tcp::resolver::iterator results)
{
    TraceLogger(network) << "ClientNetworking::Impl::HandleResolve(" << error << ")";
    if (error) {
        ErrorLogger(network) << "Failed to resolve query.";
        m_deadline_timer.cancel();
        return;
    }

    m_resolver_results = results;

    DebugLogger(network) << "Attempt to connect to server at one of these addresses:";
    tcp::resolver::iterator end_it;
    for (tcp::resolver::iterator it = results; it != end_it; ++it) {
        DebugLogger(network) << "host_name: " << it->host_name()
            << "  address: " << it->endpoint().address()
            << "  port: " << it->endpoint().port();
    }

    m_socket.close();
    m_socket.async_connect(*results, [this, results](const auto& error) {
        HandleConnection(error, results); 
    });
    TraceLogger(network) << "Return from ClientNetworking::Impl::HandleResolve()";
}

void ClientNetworking::Impl::HandleDeadlineTimeout(const boost::system::error_code& error)
{
    TraceLogger(network) << "ClientNetworking::Impl::HandleDeadlineTimeout(" << error << ")";
    if (error == boost::asio::error::operation_aborted)
    {
        // Canceled e.g. due to successfull connection
        DebugLogger(network) << "ConnectToServer() : Deadline timer cancelled.";
        return;
    }

    m_deadline_has_expired = true;
    m_reconnect_timer.cancel();
    bool did_close_socket = CloseSocketIfNotConnected();
    if (did_close_socket)
    {
        DebugLogger(network) << "ConnectToServer() : Timeout.";
    }
    TraceLogger(network) << "Return from ClientNetworking::Impl::HandleDeadlineTimeout()";
}

void ClientNetworking::Impl::HandleException(const boost::system::system_error& error) {
    if (error.code() == boost::asio::error::eof) {
        DebugLogger(network) << "Client connection disconnected by EOF from server.";
        m_socket.close();
    }
    else if (error.code() == boost::asio::error::connection_reset)
        DebugLogger(network) << "Client connection disconnected, due to connection reset from server.";
    else if (error.code() == boost::asio::error::operation_aborted)
        DebugLogger(network) << "Client connection closed by client.";
    else {
        ErrorLogger(network) << "ClientNetworking::NetworkingThread() : Networking thread will be terminated "
                             << "due to unhandled exception error #" << error.code().value() << " \""
                             << error.code().message() << "\"";
    }
}

void ClientNetworking::Impl::NetworkingThread(const std::shared_ptr<const ClientNetworking> self) {
    auto protect_from_destruction_in_other_thread = self;
    try {
        if (!m_outgoing_messages.empty())
            AsyncWriteMessage();
        AsyncReadMessage(protect_from_destruction_in_other_thread);
        m_io_context.run();
    } catch (const boost::system::system_error& error) {
        HandleException(error);
    }
    m_outgoing_messages.clear();
    m_io_context.reset();
    { // Mutex scope
        std::scoped_lock lock(m_mutex);
        m_rx_connected = false;
        m_tx_connected = false;
    }
    TraceLogger(network) << "ClientNetworking::NetworkingThread() : Networking thread terminated.";
}

void ClientNetworking::Impl::HandleMessageBodyRead(const std::shared_ptr<const ClientNetworking>& keep_alive,
                                                   boost::system::error_code error, std::size_t bytes_transferred)
{
    if (error)
        throw boost::system::system_error(error);

    assert(static_cast<int>(bytes_transferred) <= m_incoming_header[Message::Parts::SIZE]);
    if (static_cast<int>(bytes_transferred) == m_incoming_header[Message::Parts::SIZE]) {
        m_incoming_messages.PushBack(m_incoming_message);
        AsyncReadMessage(keep_alive);
    }
}

void ClientNetworking::Impl::HandleMessageHeaderRead(const std::shared_ptr<const ClientNetworking>& keep_alive,
                                                     boost::system::error_code error, std::size_t bytes_transferred)
{
    if (error)
        throw boost::system::system_error(error);
    assert(bytes_transferred <= Message::HeaderBufferSize);
    if (bytes_transferred != Message::HeaderBufferSize)
        return;

    BufferToHeader(m_incoming_header, m_incoming_message);
    m_incoming_message.Resize(m_incoming_header[Message::Parts::SIZE]);
    // Intentionally not checked for open.  We expect (header, body) pairs.
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
        boost::bind(&ClientNetworking::Impl::HandleMessageBodyRead,
                    this, keep_alive,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::Impl::AsyncReadMessage(const std::shared_ptr<const ClientNetworking>& keep_alive) {
    // If keep_alive's count < 2 the networking thread is orphaned so shut down
    if (keep_alive.use_count() < 2)
        DisconnectFromServerImpl();

    if (m_socket.is_open())
        boost::asio::async_read(
            m_socket, boost::asio::buffer(m_incoming_header),
            boost::bind(&ClientNetworking::Impl::HandleMessageHeaderRead, this, keep_alive,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::Impl::HandleMessageWrite(boost::system::error_code error, std::size_t bytes_transferred) {
    if (error) {
        throw boost::system::system_error(error);
        return;
    }

    assert(static_cast<int>(bytes_transferred) <= static_cast<int>(Message::HeaderBufferSize) + m_outgoing_header[Message::Parts::SIZE]);
    if (static_cast<int>(bytes_transferred) != static_cast<int>(Message::HeaderBufferSize) + m_outgoing_header[Message::Parts::SIZE])
        return;

    m_outgoing_messages.pop_front();
    if (!m_outgoing_messages.empty())
        AsyncWriteMessage();

    // Check if finished sending last pending write while shutting down.
    else {
        bool should_shutdown(false);
        { // Scope for the mutex
            std::scoped_lock lock(m_mutex);
            should_shutdown = !m_tx_connected;
        }
        if (should_shutdown) {
            DisconnectFromServerImpl();
        }
    }
}

void ClientNetworking::Impl::AsyncWriteMessage() {
    if (!m_socket.is_open()) {
        ErrorLogger(network) << "Socket is closed. Dropping message.";
        return;
    }

    HeaderToBuffer(m_outgoing_messages.front(), m_outgoing_header);
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(m_outgoing_header));
    buffers.push_back(boost::asio::buffer(m_outgoing_messages.front().Data(),
                                          m_outgoing_messages.front().Size()));
    boost::asio::async_write(m_socket, buffers,
                             boost::bind(&ClientNetworking::Impl::HandleMessageWrite, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::Impl::SendMessageImpl(Message message) {
    bool start_write = m_outgoing_messages.empty();
    m_outgoing_messages.push_back(std::move(message));
    if (start_write)
        AsyncWriteMessage();
}

void ClientNetworking::Impl::DisconnectFromServerImpl() {
    DebugLogger(network) << "ClientNetworking::Impl::DisconnectFromServerImpl";
    // Depending behavior of linger on OS's of the sending and receiving machines this call to close could
    // - immediately disconnect both send and receive channels
    // - immediately disconnect send, but continue receiving until all pending sent packets are
    //   received and acknowledged.
    // - send pending packets and wait for the receive side to terminate the connection.

    // The shutdown steps:
    // 1. Finish sending pending tx messages.  These may include final panic messages
    // 2. After the final tx call DisconnectFromServerImpl again from the tx handler.
    // 3. shutdown the connection
    // 4. server end acknowledges with a 0 length packet
    // 5. close the connection in the rx handler

    // Stop sending new packets
    { // Scope for the mutex
        std::scoped_lock lock(m_mutex);
        m_tx_connected = false;
        m_rx_connected = m_socket.is_open();
    }

    if (!m_outgoing_messages.empty())
        return;

    // Note: m_socket.is_open() may be independently true/false on each of these checks.
    if (m_socket.is_open())
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
}



////////////////////////////////////////////////
// ClientNetworking
////////////////////////////////////////////////
ClientNetworking::ClientNetworking() :
    m_impl(std::make_unique<ClientNetworking::Impl>())
{}

ClientNetworking::~ClientNetworking() = default;

bool ClientNetworking::IsConnected() const
{ return m_impl->IsConnected(); }

bool ClientNetworking::IsRxConnected() const
{ return m_impl->IsRxConnected(); }

bool ClientNetworking::IsTxConnected() const
{ return m_impl->IsTxConnected(); }

int ClientNetworking::PlayerID() const
{ return m_impl->PlayerID(); }

int ClientNetworking::HostPlayerID() const
{ return m_impl->HostPlayerID(); }

bool ClientNetworking::PlayerIsHost(int player_id) const
{ return m_impl->PlayerIsHost(player_id); }

bool ClientNetworking::HasAuthRole(Networking::RoleType role) const
{ return m_impl->HasAuthRole(role); }

const std::string& ClientNetworking::Destination() const
{ return m_impl->Destination(); }

ClientNetworking::ServerNames ClientNetworking::DiscoverLANServerNames()
{ return m_impl->DiscoverLANServerNames(); }

bool ClientNetworking::ConnectToServer(
    const std::string& ip_address,
    const std::chrono::milliseconds& timeout)
{ return m_impl->ConnectToServer(this, ip_address, timeout); }

bool ClientNetworking::ConnectToLocalHostServer(
    const std::chrono::milliseconds& timeout)
{ return m_impl->ConnectToLocalHostServer(this, timeout); }

bool ClientNetworking::PingServer(
    const std::string& ip_address,
    const std::chrono::milliseconds& timeout)
{ return m_impl->ConnectToServer(this, ip_address, timeout, true /*expect_timeout*/); }

bool ClientNetworking::PingLocalHostServer(
    const std::chrono::milliseconds& timeout)
{ return m_impl->ConnectToLocalHostServer(this, timeout, true /*expect_timeout*/); }

void ClientNetworking::DisconnectFromServer()
{ return m_impl->DisconnectFromServer(); }

void ClientNetworking::SetPlayerID(int player_id)
{ return m_impl->SetPlayerID(player_id); }

void ClientNetworking::SetHostPlayerID(int host_player_id)
{ return m_impl->SetHostPlayerID(host_player_id); }

Networking::AuthRoles& ClientNetworking::AuthorizationRoles()
{ return m_impl->AuthorizationRoles(); }

void ClientNetworking::SendMessage(Message message)
{ m_impl->SendMessage(std::move(message)); }

void ClientNetworking::SendSelfMessage(Message message)
{ m_impl->SendSelfMessage(std::move(message)); }

boost::optional<Message> ClientNetworking::GetMessage()
{ return m_impl->GetMessage(); }
