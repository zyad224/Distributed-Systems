#include <iostream>
#include <netdb.h>
#include "CM.h"

CM::CM(char *_myAddr, uint16_t _myPort) {
    udpSocket.initializeSocket(_myAddr, _myPort);
}

CM::~CM() {

}

int CM::send_with_ack(char *message, char *reply_buffer, size_t reply_buffer_size, int timeout_in_ms, int max_retries,
                      char *receiver_addr, uint16_t receiver_port) {
    // Send and wait for an acknowledgment.
    // A reply to the sent request is implicitly considered an acknowledgment

    ssize_t bytes_read = -1;
    char *local_message = message;

    // Create a sockaddr from the receiver's addr and port
    sockaddr_in receiver_sock_addr = create_sockaddr(receiver_addr, receiver_port);
    sockaddr_in reply_addr;

    // Send message and wait for a reply
    // If no reply is sent within timeout_in_ms, bytes_read will be -1
    // If a reply is received, bytes_read will contain a value > 0 indicating the number of bytes read
    // We repeat this send/wait for reply sequence until a reply is received or max_retries reaches 0
    while (max_retries-- && bytes_read == -1) {
        udpSocket.writeToSocket(local_message,receiver_sock_addr);
        bytes_read = udpSocket.readFromSocketWithTimeout(reply_buffer, reply_buffer_size, reply_addr, timeout_in_ms);
    }
    return (int) bytes_read;
}

int CM::send_no_ack(char *message, char *receiver_addr, uint16_t receiver_port) {
    // Send and don't wait for a reply

    // Create receiver_sock_addr from receiver's addr and port
    sockaddr_in receiver_sock_addr = create_sockaddr(receiver_addr, receiver_port);

    char *local_message = message;
    ssize_t bytes_sent = udpSocket.writeToSocket(local_message, receiver_sock_addr);
    return (int) bytes_sent;
}

int CM::send_no_ack(char *message, sockaddr_in receiver_sock_addr) {
    char *local_message = message;
    ssize_t bytes_sent = udpSocket.writeToSocket(local_message, receiver_sock_addr);
    return (int) bytes_sent;
}

sockaddr_in CM::create_sockaddr(char *addr, uint16_t port) {
    struct hostent *host;
    sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;

    if ((host = gethostbyname(addr)) == NULL) {
        std::cerr << "Error occured when fetching hostname\n";
    }

    sock_addr.sin_addr = *(struct in_addr *) (host->h_addr);

    // Sometimes a segmentation fault might occur here when multiple threads
    // call htons(_peerPort) at the same time
    // This is why we initialize peerAddr only once at the client's initialization
    // and not on every parameters we send
    sock_addr.sin_port = htons(port);
    return sock_addr;
}

int CM::recv_with_block(char *message, size_t message_size, sockaddr_in &receiver_addr) {
    // A blocking receive that fills char* message with the received contents
    ssize_t bytes_read = udpSocket.readFromSocketWithBlock(message, message_size, receiver_addr);
    return (int) bytes_read;
}
