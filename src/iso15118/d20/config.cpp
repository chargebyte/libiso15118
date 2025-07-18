// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 Pionix GmbH and Contributors to EVerest
#include <iso15118/d20/config.hpp>

#include <algorithm>

#include <iso15118/detail/helper.hpp>

namespace iso15118::d20 {

namespace dt = message_20::datatypes;

namespace {

auto get_mobility_needs_mode(const ControlMobilityNeedsModes& mode) {
    using namespace dt;

    if (mode.control_mode == ControlMode::Scheduled and mode.mobility_mode == MobilityNeedsMode::ProvidedBySecc) {
        logf_info("Setting the mobility needs mode to ProvidedByEvcc. In scheduled mode only ProvidedByEvcc is "
                  "supported.");
        return MobilityNeedsMode::ProvidedByEvcc;
    }

    return mode.mobility_mode;
}

auto get_default_dc_parameter_list(const std::vector<ControlMobilityNeedsModes>& control_mobility_modes) {
    using namespace dt;

    // TODO(sl): Add check if a control mode is more than one in that vector

    std::vector<DcParameterList> param_list;

    for (const auto& mode : control_mobility_modes) {
        param_list.push_back({
            DcConnector::Extended,
            mode.control_mode,
            get_mobility_needs_mode(mode),
            Pricing::NoPricing,
        });
    }

    return param_list;
}

auto get_default_dc_bpt_parameter_list(const std::vector<ControlMobilityNeedsModes>& control_mobility_modes) {
    using namespace dt;

    // TODO(sl): Add check if a control mode is more than one in that vector

    std::vector<DcBptParameterList> param_list;

    for (const auto& mode : control_mobility_modes) {
        param_list.push_back({{
                                  DcConnector::Extended,
                                  mode.control_mode,
                                  get_mobility_needs_mode(mode),
                                  Pricing::NoPricing,
                              },
                              BptChannel::Unified,
                              GeneratorMode::GridFollowing});
    }

    return param_list;
}

auto get_default_mcs_parameter_list(const std::vector<ControlMobilityNeedsModes>& control_mobility_modes) {
    using namespace dt;

    // TODO(sl): Add check if a control mode is more than one in that vector
    std::vector<McsParameterList> param_list;

    for (const auto& mode : control_mobility_modes) {
        param_list.push_back({
            McsConnector::Mcs,
            mode.control_mode,
            get_mobility_needs_mode(mode),
            Pricing::NoPricing,
        });
    }

    return param_list;
}

auto get_default_mcs_bpt_parameter_list(const std::vector<ControlMobilityNeedsModes>& control_mobility_modes) {
    using namespace dt;

    // TODO(sl): Add check if a control mode is more than one in that vector
    std::vector<McsBptParameterList> param_list;

    for (const auto& mode : control_mobility_modes) {
        param_list.push_back({{
                                  McsConnector::Mcs,
                                  mode.control_mode,
                                  get_mobility_needs_mode(mode),
                                  Pricing::NoPricing,
                              },
                              BptChannel::Unified,
                              GeneratorMode::GridFollowing});
    }

    return param_list;
}

} // namespace

SessionConfig::SessionConfig(EvseSetupConfig config) :
    evse_id(std::move(config.evse_id)),
    cert_install_service(config.enable_certificate_install_service),
    authorization_services(std::move(config.authorization_services)),
    supported_energy_transfer_services(std::move(config.supported_energy_services)),
    supported_vas_services(std::move(config.supported_vas_services)),
    dc_limits(std::move(config.dc_limits)),
    supported_control_mobility_modes(std::move(config.control_mobility_modes)),
    custom_protocol(std::move(config.custom_protocol)) {

    // TODO(SL): How to handle this probaly
    const auto is_bpt_service = [](dt::ServiceCategory service) {
        return service == dt::ServiceCategory::DC_BPT or service == dt::ServiceCategory::MCS_BPT;
    };
    const auto dc_bpt_found = std::any_of(supported_energy_transfer_services.begin(),
                                          supported_energy_transfer_services.end(), is_bpt_service);

    if (dc_bpt_found and not dc_limits.discharge_limits.has_value()) {
        logf_warning("The supported energy services contain DC_BPT or MCS_BPT, but dc limits does not contain BPT "
                     "limits. This can lead to session shutdowns.");
    }

    if (supported_control_mobility_modes.empty()) {
        logf_warning("No control modes were provided, set to scheduled mode");
        supported_control_mobility_modes = {{dt::ControlMode::Scheduled, dt::MobilityNeedsMode::ProvidedByEvcc}};
    }

    dc_parameter_list = get_default_dc_parameter_list(supported_control_mobility_modes);
    dc_bpt_parameter_list = get_default_dc_bpt_parameter_list(supported_control_mobility_modes);

    mcs_parameter_list = get_default_mcs_parameter_list(supported_control_mobility_modes);
    mcs_bpt_parameter_list = get_default_mcs_bpt_parameter_list(supported_control_mobility_modes);
}

} // namespace iso15118::d20
