<!--- \defgroup parser Protocol parser/generator -->

[TOC]

[STUN]: https://tools.ietf.org/html/rfc5389
[TURN]: https://tools.ietf.org/html/rfc5766
[MS-TURN]: https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx
[STUN traits]: @ref turner::stun::protocol_traits_t
[TURN traits]: @ref turner::turn::protocol_traits_t
[MS-TURN traits]: @ref turner::msturn::protocol_traits_t


Overview {#overview}
========

Turner library provides functionality to generate or parse [STUN], [TURN] and
[MS-TURN] protocol messages. All those protocols are based on [STUN],
extending it with new messages and attributes. Due similarity, most of code is
shared and specific protocol just provides traits structure ([STUN traits],
[TURN traits], [MS-TURN traits]) describing it's details used during
parsing/generating.

Common API is designed to be type-safe and user extensible i.e. business logic
scope can add new messages or attributes to concrete protocol and Turner
library will be able to use them while preventing simpler type errors during
compile time (like handling [TURN] message as [MS-TURN]'s, extracting string
from integer attribute, etc). Applications can even provide their own protocol
traits to create new [STUN]-like protocols.

Parsing incoming network message is divided into three phases:
1. check if bytes represent protocol message (bytes ->
   turner::protocol_t::parse -> turner::any_message_t)
2. check what kind of request/response message is
   (turner::any_message_t::try_as -> turner::message_reader_t)
3. extract attributes from request/response (turner::message_reader_t::read ->
   attribute value)

> **Important** thing to remember is that during this process all instantiated
> classes are constructed on same memory region as incoming raw network data (as
> const, i.e. underlying data is not changed but changing it externally is
> undefined behaviour). This allows to employ zero-copy approach.

Generating new messages are equally simple:
1. create new message into given region (turner::message_type_t::make ->
   turner::message_writer_t)
2. add attributes (turner::message_writer_t::write)
3. finalize message (turner::message_writer_t::finish)

Or to generate response for existing request:
1. turn existing message into response
   (turner::message_reader_t::to_success_response or
   turner::message_reader_t::to_error_response)

Following sections will provide simple sample code.


Generate request {#generate-request}
----------------

Create new STUN Binding request into given buffer:
  ~~~{.cpp}
  char buf[1024];

  // begin -> &buf[0], end -> end of STUN Binding message
  auto [begin, end] = turner::stun::binding.make(buf).finish();
  ~~~

Or slightly more complex sample with adding new field:
  ~~~{.cpp}
  char buf[1024];
  auto [begin, end] = turner::turn::allocation.make(buf)
    .write(turner::turn::software, "turner_client/1.0")
    .write(turner::turn::lifetime, 600)
    .finish();
  ~~~

The most complex of all, adding also message integrity:
  ~~~{.cpp}
  char buf[1024];
  auto integrity_calculator = turner::msturn::make_integrity_calculator(
    "realm", "username", "password"
  );
  auto [begin, end] = turner::msturn::allocate.make(buf)
    .write(turner::turn::software, "turner_client/1.0")
    .write(turner::turn::lifetime, 600)
    .finish(integrity_calculator);
  ~~~


Parse request {#parse-request}
-------------

After receiving MS-TURN Allocate message generated above, detect protocol and
then parse message
  ~~~{.cpp}
  std::vector<uint8_t> raw_data; // assume it is already filled

  // detect protocol from raw data
  if (auto message = turner::msturn::parse(raw_data))
  {
    // check if message has valid integrity
    auto integrity_calculator = turner::msturn::make_integrity_calculator(
      "realm", "username", "password"
    );
    if (!message->has_valid_integrity(integrity_calculator))
    {
      return;
    }

    // detect and handle request (note different ways of detecting)
    if (auto request = turner::msturn::allocate(message))
    {
      auto lifetime = request.read(turner::msturn::lifetime);
      // ...
    }
    else if (auto request = message->try_as(turner::msturn::set_active_destination))
    {
      // ...
    }
  }
  else if (auto message = turner::turn::parse(raw_data))
  {
    // ...
  }
  ~~~


Generate response for request {#generate-response}
-----------------------------

Assuming we already received and detected incoming MS-TURN Allocate message,
here's how we can generate response for it:
  ~~~{.cpp}
  // note how we reuse same raw_data as occupied by request itself
  // after this call, using request is undefined behaviour
  // to preserve request, simply use different destination when calling
  // turner::message_reader_t::to_error_response
  auto [begin, end] = request.to_error_response(raw_data)
    .write(turner::msturn::software, "turner_server/1.0")
    .write(turner::msturn::error_code, turner::msturn::try_alternate)
    .write(turner::msturn::alternate_server, {address, port});
    .finish();
  ~~~


Extending library {#extending}
-----------------

As it often happens, official protocol or parser library evolves slower than
business logic needs. To handle such situations, [STUN] protocol defines
message and attribute ranges that can be extended by "designated expert".

Business logic side can add easily new attribute types in application scope,
after which those can be used just like any other attribute type:
Similar approach works for adding new attributes as well:
  ~~~{.cpp}
  // Add new string attribute, alternate_fqdn
  // see example below how to use it
  constexpr const turner::turn::protocol_t::attribute_type_t<0x8001,
    turner::string_attribute_processor_t
  > alternate_fqdn;
  ~~~

New message types can be added similar way:
  ~~~{.cpp}
  // Add new message to TURN protocol (GO-AWAY to signal connected clients
  // that server will be going down and presenting new FQDN where they can
  // connect to)
  constexpr const turner::turn::protocol_t::message_type_t<0x0800> go_away;

  char buf[1024];
  auto [begin, end] = go_away.make(buf)
    .write(alternate_fqdn, "turn.domain")
    .finish();
  ~~~

> **Important**: Turner library does not support "comprehension required" attribute types
> discovery yet due lack of finding satisfactory API for it. (work in
> progress)
