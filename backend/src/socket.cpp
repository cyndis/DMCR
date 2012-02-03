#include "socket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "dmcr_protocol.pb.h"
#include <cstdio>

dmcr::Socket::Socket(const std::string &hostname, in_port_t port)
    : m_hostname(hostname), m_port(port), m_listener(NULL),
      m_fd(0), m_seq(0)
{
#ifdef DMCR_THREADED
    pthread_spin_init(&m_spinlock, 0);
#endif
}

dmcr::Socket::~Socket()
{
    close(m_fd);
}

void dmcr::Socket::setListener(IBackendSocketListener *listener)
{
    m_listener = listener;
}

void dmcr::Socket::connect()
{
    // Resolve host
    struct addrinfo *addr;
    struct addrinfo hints;
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL; hints.ai_addr = NULL; hints.ai_next = NULL;

    char port_str[10];
    snprintf(port_str, 10, "%d", m_port);

    int r = getaddrinfo(m_hostname.c_str(), port_str, &hints, &addr);
    if (r != 0)
        throw SocketException("Could not resolve hostname");

    int fd = 0;
    for (; addr != NULL; addr = addr->ai_next) {
        fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (!fd)
            continue;

        if (::connect(fd, addr->ai_addr, addr->ai_addrlen) == -1) {
            close(fd);
            continue;
        }

        break;
    }

    if (addr == NULL) {
        freeaddrinfo(addr);
        throw SocketException("Could not connect to host");
    }

    freeaddrinfo(addr);

    m_fd = fd;

    sendHandshakePacket();
}

void dmcr::Socket::sendHandshakePacket()
{
    DmcrBackendHandshake packet;
    packet.set_protocol_version(0);
    packet.set_description("DMCR/1.0");
    sendPacket(Packet_BackendHandshake, packet);
}

void dmcr::Socket::sendPacket(PacketId id,
                              const ::google::protobuf::Message &msg)
{
#ifdef DMCR_THREADED
    pthread_spin_lock(&m_spinlock);
#endif
    DmcrPacketHeader header;
    header.set_length(msg.ByteSize());
    header.set_id((uint8_t)id);
    header.set_seq(m_seq++);

    header.SerializeToFileDescriptor(m_fd);
    msg.SerializeToFileDescriptor(m_fd);
#ifdef DMCR_THREADED
    pthread_spin_unlock(&m_spinlock);
#endif
}
