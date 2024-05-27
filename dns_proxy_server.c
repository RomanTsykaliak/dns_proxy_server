// dns_proxy_server.c
/*
Copyright 2024 Roman Tsykaliak
  This program is free software: you can
redistribute it and/or modify it under the terms
of the GNU General Public License as published
by the Free Software Foundation, either version
3 of the License, or (at your option) any later
version.
  This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for
more details.
  You should have received a copy of the GNU
General Public License along with this program.
If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/dns_struct.h>
#include <event2/util.h>
#define DNS_SERVER_IP "8.8.8.8"
#define DNS_SERVER_PORT 53
#define MAX_BUFFER_SIZE 512
//#include <ldns/ldns.h>
#include <arpa/inet.h> // inet_ntop inet_pton
#define UNUSED(x) (void)(x)
// Port number less than 1024 need superuser
// privileges
#define DNS_PORT 1025 // 53
#define BLACKLIST_SIZE 3
// the Google's public DNS server
#define UPSTREAM_DNS "8.8.8.8"
#define IP_ADDRESS "127.0.0.1"
const char *blacklist[BLACKLIST_SIZE]= {
  "bad-domain.com",
  "another-bad-domain.com",
  "yet-another-bad-domain.com"
};
int is_blacklisted(const char *domain) {
  for(int i= 0; i < BLACKLIST_SIZE; ++i)
    if(strcmp(domain, blacklist[i]) == 0)
      return 1;
  return 0;
}
// Check if the requested domain is in the
// blacklist.  The `dns_base` object is passed
// in `main()` to the handle_request function as
// the `data` parameter.
void handle_request(
    struct evdns_server_request*req, void*data){
  struct evdns_base *dns_base=
                       (struct evdns_base*)data;
  for(int i= 0; i < req->nquestions; ++i) {
    struct evdns_server_question
                          *q= req->questions[i];
    if(is_blacklisted(q->name)) {
      // If a domain is in the blacklist,
      // respond with a REFUSED status code.
      //evdns_server_request_respond
      //              (req, LDNS_RCODE_REFUSED);
      return;
    } else {
      // If a domain is not blacklisted,
      // the request is forwarded the request to
      // the upstream DNS server
      evdns_base_resolve_ipv4
             (dns_base, q->name, 0, NULL, NULL);
} } }
void dns_callback(int errcode,
      struct evutil_addrinfo *addr, void *ptr) {
  UNUSED(errcode);
  UNUSED(addr);
  UNUSED(ptr);
  // This function will be called when
  // the DNS lookup is complete
}
static int cpuNum() {
  return (int)sysconf(_SC_NPROCESSORS_ONLN);
}
static void evdns_server_callback(
    struct evdns_server_request*req, void*data){
  UNUSED(req);
  UNUSED(data);
  printf("Get DNS query/n");
  return;
}
int main() {
  int sockfd;
  struct sockaddr_in server_addr, client_addr,
                                dns_server_addr;
  socklen_t addr_len=sizeof(struct sockaddr_in);
  char buffer[MAX_BUFFER_SIZE];
  // Create socket
  if((sockfd= socket(AF_INET,SOCK_DGRAM,0)) <0){
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  // Bind the socket to port 53
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family= AF_INET;
  server_addr.sin_addr.s_addr= INADDR_ANY;
  server_addr.sin_port= htons(DNS_SERVER_PORT);
  if(bind(sockfd,
          (const struct sockaddr*)&server_addr,
          sizeof(server_addr)           ) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  // Set up the DNS server address
  memset(&dns_server_addr, 0,
                       sizeof(dns_server_addr));
  dns_server_addr.sin_family= AF_INET;
  dns_server_addr.sin_port=
                         htons(DNS_SERVER_PORT);
  dns_server_addr.sin_addr.s_addr=
                       inet_addr(DNS_SERVER_IP);
  while(1) {
    int len= recvfrom(sockfd, buffer,
     MAX_BUFFER_SIZE, 0,
     (struct sockaddr*)&client_addr, &addr_len);
    if(len > 0) {
      sendto(sockfd, buffer, len, 0,
             (struct sockaddr*)&dns_server_addr,
             sizeof(dns_server_addr));
      len= recvfrom(sockfd, buffer,
                MAX_BUFFER_SIZE, 0, NULL, NULL);
      sendto(sockfd, buffer, len, 0,
                 (struct sockaddr*)&client_addr,
                 addr_len);
  } }
  return 0;
  printf("DNS proxy server starting...\n");
  printf("Number of CPU %d\n", cpuNum());
  struct event_base *event_base=
                               event_base_new();
  //struct evdns_base *evdns_base= NULL;
  //evdns_base= evdns_base_new(event_base, 0);
  // Create a server socket and
  // obtain a valid file descriptor.  `AF_INET`
  // for IPv4 addresses.  `SOCK_DGRAM` for
  // the UDP protocol.  `0` to chose UDP.
  evutil_socket_t sock=
                 socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    perror("Failed to create a socket.\n");
    return 1;
  }
  // Calls to send/recv will return immediatelly
  // with whatever they can do and not wait
  // until they have done everything asked for
  evutil_make_socket_nonblocking(sock);
  // Define the address structure for the socket
  struct sockaddr_in my_addr;
  my_addr.sin_family= AF_INET; // IPv4
  my_addr.sin_port=
                htons((unsigned short)DNS_PORT);
  //my_addr.sin_addr.s_addr= INADDR_ANY;//any IP
  // Convert the IP address from text
  // to binary form
  if(inet_pton(AF_INET, IP_ADDRESS,
                  &(my_addr.sin_addr)) <= 0) {
       perror("Failed to set IP address.\n");
       return 1;
  }
  // Bind the socket to the address.  This is
  // needed before the socket can receive data
  if(bind(sock, (struct sockaddr*)&my_addr,
                         sizeof(my_addr)) < 0) {
    perror("Failed to bind the socket.\n");
    return 1;
  }
  // Add a DNS server port to the event base.
  // This will handle DNS requests that come in
  // on the socket
  evdns_add_server_port_with_base(event_base,
          sock, 0, evdns_server_callback, NULL);
  // Start the event loop.  This will start
  // processing events and will not return until
  // there are no more events to process
  event_base_dispatch(event_base);
  return 0;
  struct event_base *base= event_base_new();
  struct evdns_base *dns_base=
                        evdns_base_new(base, 1);
  struct evutil_addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family= AF_UNSPEC;
  hints.ai_flags= EVUTIL_AI_CANONNAME;
  hints.ai_socktype= SOCK_STREAM;
  hints.ai_protocol= IPPROTO_TCP;
  const char *hostname= "google.com";
  struct evdns_getaddrinfo_request *req=
    evdns_getaddrinfo(dns_base, hostname,
                 NULL /* no servie name given*/,
                 &hints, dns_callback, NULL);
  if(req == NULL)
   printf("Request for %s failed immediately\n",
                                      hostname);
  event_base_dispatch(base);
  return 0;
  // Create a server socket and
  // obtain a valid file descriptor.  `AF_INET`
  // for IPv4 addresses.  `SOCK_DGRAM` for
  // the UDP protocol.  `0` to chose UDP.
  sockfd= socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    perror("Failed to create socket");
    return 1;
  }
  evdns_base_nameserver_ip_add
                       (dns_base, UPSTREAM_DNS);
  // Create a new DNS server that listens on
  // port 53
  //struct evdns_server_port *port=
  evdns_add_server_port_with_base
    (base, sockfd, 0, handle_request, dns_base);
  printf("DNS proxy server started on port: "
         "%d\n", sockfd);
  event_base_dispatch(base);
  //close(sockfd);
  return 0;
}
