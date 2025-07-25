// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 Pionix GmbH and Contributors to EVerest
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <iso15118/io/stream_view.hpp>

namespace iso15118::message_20 {

struct SupportedAppProtocol {
    std::string protocol_namespace;
    uint32_t version_number_major;
    uint32_t version_number_minor;
    uint8_t schema_id;
    uint8_t priority;

    bool operator==(const iso15118::message_20::SupportedAppProtocol& other) const {
        return this->protocol_namespace == other.protocol_namespace and this->priority == other.priority and
               this->schema_id == other.schema_id and this->version_number_major == other.version_number_major and
               this->version_number_minor == other.version_number_minor;
    }
};

struct SupportedAppProtocolRequest {
    std::vector<SupportedAppProtocol> app_protocol;
};

struct SupportedAppProtocolResponse {
    enum class ResponseCode {
        OK_SuccessfulNegotiation = 0,
        OK_SuccessfulNegotiationWithMinorDeviation = 1,
        Failed_NoNegotiation = 2,
    };

    ResponseCode response_code;
    std::optional<uint8_t> schema_id;
};

size_t encode_supported_app_protocol_response(const io::StreamOutputView&, const SupportedAppProtocolResponse&);

} // namespace iso15118::message_20
