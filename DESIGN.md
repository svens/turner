# DESIGN

C++ STUN/TURN/MS-TURN/MTURN library and server. Combines two predecessors:

1. `v0.turner`: typed C++ relationships between protocols, messages and
   attributes; zero-copy reads.
2. `rustun`: layering codec <- no-I/O server <- application; in-place
   request-to-response transformation. Design reference only; no code or
   documentation is reused.

## 1. Priorities

1. Public API simplicity.
2. Implementation scalability and performance over internal simplicity.
3. When on the fence, prefer the simpler option.

## 2. Scope

1. Protocols: STUN, TURN, MS-TURN, MTURN (details pending discussion).
2. RFC baseline: [RFC 8489](https://datatracker.ietf.org/doc/html/rfc8489)
   (STUN) and [RFC 8656](https://datatracker.ietf.org/doc/html/rfc8656)
   (TURN).
3. IPv6 supported from the start.

## 3. Layers

| Target | Depends on | Responsibility |
|---|---|---|
| `turner::codec` | pal | protocols, messages, attributes, reader/writer, ChannelData, algorithms |
| `turner::server` | codec, pal | protocol logic, no I/O, driven by explicit time, returns actions |
| `turner::simple_client` | codec | functional-test API: build requests, parse responses; no timers or retransmission |
| `turner::client` | codec, pal | deferred: retransmission, timers, allocation lifecycle |
| test harness | server, simple_client | in-memory transport and fake clock |
| `turnerd` | server, pal | deferred: sockets, configuration, signals |

Dependencies are unidirectional, top of table first.

## 4. Codec

### 4.1 Declarations

1. Each protocol is a struct holding `constexpr` method and attribute objects.
   Protocol extension is struct inheritance (`turn: stun`).
2. An attribute binds protocol, value type and type code at compile time.
   Applications define their own attributes the same way, without modifying
   the library.
3. A method declares its supported classes (request/response, indication).
   Creating a message of an unsupported class does not compile.
4. Mixing protocols (e.g. reading an MS-TURN attribute from a STUN message)
   does not compile.
5. Each protocol provides a registry of known attributes; the server uses it to
   produce 420 Unknown Attribute responses.

### 4.2 Reading

1. Parsing validates the whole message once: header, cookie, TLV structure,
   attribute count limit, FINGERPRINT. Later operations trust it and skip
   re-validation.
2. A parsed message is narrowed with `as(method)`, returning an optional typed
   request or indication. A visitor-style dispatch may be added later on top.
3. A message is a `std::ranges` range of attribute views; standard algorithms
   apply. Library algorithms cover typed reads, multi-attribute reads into
   structured bindings, and collecting unknown comprehension-required
   attributes.
4. Strings and byte blobs are returned as views into the message buffer.

### 4.3 Writing

1. New messages: writer over a caller-provided buffer, append typed attributes,
   finish with or without MESSAGE-INTEGRITY.
2. Responses: a request transforms in place into a success or error response
   writer over the same buffer. Only request types offer the transformation;
   it consumes the request (`&&`-qualified).
3. Views read from a request are invalid once its response writes attributes.
   Callers finish reading before transforming.

### 4.4 Buffer model

1. Buffers are caller-owned, mutable and fixed-size; the codec never allocates.
2. Parse input is the full buffer span plus received size.
3. Each protocol exports `max_headroom`: bytes the application reserves before
   received data so relay headers (Data indication, ChannelData) are prepended
   in place.
4. Client-to-peer relay forwards a subspan of the received buffer without
   copying.
5. Message positions and lengths are 16-bit, matching the wire format.
6. MESSAGE-INTEGRITY verification patches the header length in place,
   computes, and restores it.

### 4.5 Cryptography

1. HMAC and hashing come from pal/crypto directly; no pluggable provider layer.

## 5. Server

1. No system calls: no sockets, clock, random source or filesystem access.
2. Application provides current time, random bytes, credential lookup and
   logging hooks.
3. Input is one datagram plus its ingress and source; output is an action
   (send a span of the input buffer to an endpoint, or drop).
4. Hot-path lookups (relay port, client 5-tuple, channel/peer within an
   allocation) are O(1).
5. Threading model: open, see [Open questions](#7-open-questions).

## 6. Build and dependencies

1. Conventions follow pal: C++23, `.hpp` headers, its STYLE.md and AGENTS.md,
   `ninja` targets over CMake presets.
2. pal is fetched with CMake `FetchContent` pinned to a commit. When a sibling
   pal checkout exists, `FETCHCONTENT_SOURCE_DIR_PAL` defaults to it so both
   projects can be edited together. Switching to a git submodule remains an
   option.

## 7. Open questions

1. Server threading model: per-thread instances with partitioned state, or a
   shared instance.
2. MS-TURN and MTURN details.
