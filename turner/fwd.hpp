#pragma once

/**
 * \file turner/fwd.hpp
 * Forward declarations
 */

#include <turner/config.hpp>
#include <cstdint>


__turner_begin


//
// turner/protocol.hpp
//

template <typename ProtocolTraits>
class protocol_t;

//
// turner/message.hpp
//

template <typename ProtocolTraits>
class any_message_t;

template <typename ProtocolTraits, uint16_t MessageType>
class message_reader_t;

template <typename ProtocolTraits, uint16_t MessageType>
class message_writer_t;

//
// turner/message_type.hpp
//

template <typename ProtocolTraits, uint16_t MessageType>
class message_type_t;

//
// turner/attribute.hpp
//

class any_attribute_t;

template <
  typename ProtocolTraits,
  uint16_t AttributeType,
  template <typename> typename AttributeProcessor
>
class attribute_type_t;

//
// turner/attribute_processor.hpp
//

template <typename ProtocolTraits> struct address_attribute_processor_t;
template <typename ProtocolTraits> struct array_attribute_processor_t;
template <typename ProtocolTraits> struct error_attribute_processor_t;
template <typename ProtocolTraits> struct string_attribute_processor_t;
template <typename ProtocolTraits> struct uint32_attribute_processor_t;


__turner_end
