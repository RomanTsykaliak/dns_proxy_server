<h1 align="center">DNS proxy server</h1>

<h4 align="center">The DNS proxy server with a domains blacklist feature to filter unwanted host names resolving.</h4>

## Video

## Pictures

## Project Overview

#### Functional requirements ####

1. The proxy server reads its parameters during startup from the configuration file.
2. The configuration file contains the following parameters: 
   * IP address of upstream DNS server
   * List of domain names to filter resolving ("blacklist")
   * Type of DNS proxy server's response for blacklisted domains (not found, refused, resolve to a pre-configured IP address)
3. DNS proxy server uses UDP protocol for processing client's requests and for interaction with upstream DNS server.
4. If a domain name from a client's request is not found in the blacklist, then the proxy server forwards the request to the upstream DNS server, waits for a response, and sends it back to the client.
5. If a domain name from a client's request is found in the blacklist, then the proxy server responds with a response defined in the configuration file.

#### Non-functional requirements ####

1. Programming language: C
2. It is allowed to use third-party libraries or code.  If you use one then you must comply with its license terms and conditions.
3. All other requirements and restrictions are up to your discretion.

#### Expectations ####

1. Source code of DNS proxy server written according to the requirements.  Source code should be in a plain ASCII text (it is not a joke—sometimes we receive source code in MS Word document).
2. Instruction how to build, configure and run the proxy server (including necessary files, like Makefile, etc.).
3. Description how you tested the proxy server.
4. Document, if your solution has additional (known to you) limitations or restrictions, or it has more features that it was requested.

## Usage Guide

Install necessary libraries: `libevent` library to handle network events; `ldns` library to handle DNS messages.

``` bash
sudo apt-get install libevent-dev libldsn-dev
```

Then, compile (as user):

``` bash
make
```

Get the `dig` command for testing:

``` bash
sudo apt-get install bind9-dnsutils
```

In a terminal window, start the DNS proxy server:

``` bash
./dns_proxy_server
```

## Verification

Open a new terminal window and use the `dig` command to send a DNS query to `localhost` (the DNS proxy server) on port 53.

``` bash
host -p 1025 google.com 127.0.0.1
;; communications error to 127.0.0.1#1025: timed out
;; communications error to 127.0.0.1#1025: timed out
;; no servers could be reached
```

``` bash
dig -p 1025 @127.0.0.1 google.com
;; communications error to 127.0.0.1#1025: timed out
;; communications error to 127.0.0.1#1025: timed out
;; communications error to 127.0.0.1#1025: timed out

; <<>> DiG 9.18.19-1~deb12u1-Debian <<>> -p 1025 @127.0.0.1 google.com
; (1 server found)
;; global options: +cmd
;; no servers could be reached
```

``` bash
nslookup
> set port=1025
> server 127.0.0.1
Default server: 127.0.0.1
Address: 127.0.0.1#1025
> host google.com
;; communications error to 127.0.0.1#1025: timed out
;; communications error to 127.0.0.1#1025: timed out
;; communications error to 127.0.0.1#1025: timed out
;; no servers could be reached
```

If google.com is not in the blacklist, the response will include the IP address for google.com; otherwise, the response will indicate that the query was refused.

## License

GPL-3.0-or-later
