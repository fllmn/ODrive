
#include "can_simple.hpp"

#include <odrive_main.h>

void CANSimple::handle_can_message(const can_Message_t& msg) {
    //     Frame
    // nodeID | CMD
    // 6 bits | 5 bits
    uint32_t nodeID = get_node_id(msg.id);

    for (auto& axis : axes) {
        if ((axis.config_.can.node_id == nodeID) && (axis.config_.can.is_extended == msg.isExt)) {
            doCommand(axis, msg);
            return;
        }
    }
}

void CANSimple::doCommand(Axis& axis, const can_Message_t& msg) {
    const uint32_t cmd = get_cmd_id(msg.id);
    axis.watchdog_feed();
    switch (cmd) {
        case MSG_CO_NMT_CTRL:
            break;
        case MSG_CO_HEARTBEAT_CMD:
            break;
        case MSG_ODRIVE_HEARTBEAT:
            // We don't currently do anything to respond to ODrive heartbeat messages
            break;
        case MSG_ODRIVE_ESTOP:
            estop_callback(axis, msg);
            break;
        case MSG_GET_MOTOR_ERROR:
            if (msg.rtr)
                get_motor_error_callback(axis);
            break;
        case MSG_GET_ENCODER_ERROR:
            if (msg.rtr)
                get_encoder_error_callback(axis);
            break;
        case MSG_GET_SENSORLESS_ERROR:
            if (msg.rtr)
                get_sensorless_error_callback(axis);
            break;
        case MSG_SET_AXIS_NODE_ID:
            set_axis_nodeid_callback(axis, msg);
            break;
        case MSG_SET_AXIS_REQUESTED_STATE:
            set_axis_requested_state_callback(axis, msg);
            break;
        case MSG_SET_AXIS_STARTUP_CONFIG:
            set_axis_startup_config_callback(axis, msg);
            break;
        case MSG_GET_ENCODER_ESTIMATES:
            if (msg.rtr)
                get_encoder_estimates_callback(axis);
            break;
        case MSG_GET_ENCODER_COUNT:
            if (msg.rtr)
                get_encoder_count_callback(axis);
            break;
        case MSG_SET_INPUT_POS:
            set_input_pos_callback(axis, msg);
            break;
        case MSG_SET_INPUT_VEL:
            set_input_vel_callback(axis, msg);
            break;
        case MSG_SET_INPUT_TORQUE:
            set_input_torque_callback(axis, msg);
            break;
        case MSG_SET_CONTROLLER_MODES:
            set_controller_modes_callback(axis, msg);
            break;
        case MSG_SET_VEL_LIMIT:
            set_vel_limit_callback(axis, msg);
            break;
        case MSG_START_ANTICOGGING:
            start_anticogging_callback(axis, msg);
            break;
        case MSG_SET_TRAJ_INERTIA:
            set_traj_inertia_callback(axis, msg);
            break;
        case MSG_SET_TRAJ_ACCEL_LIMITS:
            set_traj_accel_limits_callback(axis, msg);
            break;
        case MSG_SET_TRAJ_VEL_LIMIT:
            set_traj_vel_limit_callback(axis, msg);
            break;
        case MSG_GET_IQ:
            if (msg.rtr)
                get_iq_callback(axis);
            break;
        case MSG_GET_SENSORLESS_ESTIMATES:
            if (msg.rtr)
                get_sensorless_estimates_callback(axis);
            break;
        case MSG_RESET_ODRIVE:
            NVIC_SystemReset();
            break;
        case MSG_GET_VBUS_VOLTAGE:
            if (msg.rtr)
                get_vbus_voltage_callback(axis);
            break;
        case MSG_CLEAR_ERRORS:
            clear_errors_callback(axis, msg);
            break;
        case MSG_SET_PERIODIC_UPDATES:
            enable_periodic_update(axis, msg);
        default:
            break;
    }
}

void CANSimple::nmt_callback(const Axis& axis, const can_Message_t& msg) {
    // Not implemented
}

void CANSimple::estop_callback(Axis& axis, const can_Message_t& msg) {
    axis.error_ |= Axis::ERROR_ESTOP_REQUESTED;
}

int32_t CANSimple::get_motor_error_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_MOTOR_ERROR;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal(txmsg, axis.motor_.error_, 0, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::get_encoder_error_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_ENCODER_ERROR;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal(txmsg, axis.encoder_.error_, 0, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::get_sensorless_error_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_SENSORLESS_ERROR;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal(txmsg, axis.sensorless_estimator_.error_, 0, 32, true);

    return odCAN->write(txmsg);
}

void CANSimple::set_axis_nodeid_callback(Axis& axis, const can_Message_t& msg) {
    axis.config_.can.node_id = can_getSignal<uint32_t>(msg, 0, 32, true);
}

void CANSimple::set_axis_requested_state_callback(Axis& axis, const can_Message_t& msg) {
    axis.requested_state_ = static_cast<Axis::AxisState>(can_getSignal<int32_t>(msg, 0, 16, true));
}

void CANSimple::set_axis_startup_config_callback(Axis& axis, const can_Message_t& msg) {
    // Not Implemented
}

int32_t CANSimple::get_encoder_estimates_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_ENCODER_ESTIMATES;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal<float>(txmsg, axis.encoder_.pos_estimate_.any().value_or(0.0f), 0, 32, true);
    can_setSignal<float>(txmsg, axis.encoder_.vel_estimate_.any().value_or(0.0f), 32, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::get_sensorless_estimates_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_SENSORLESS_ESTIMATES;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    static_assert(sizeof(float) == sizeof(axis.sensorless_estimator_.pll_pos_));

    can_setSignal<float>(txmsg, axis.sensorless_estimator_.pll_pos_, 0, 32, true);
    can_setSignal<float>(txmsg, axis.sensorless_estimator_.vel_estimate_.any().value_or(0.0f), 32, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::get_encoder_count_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_ENCODER_COUNT;
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal<int32_t>(txmsg, axis.encoder_.shadow_count_, 0, 32, true);
    can_setSignal<int32_t>(txmsg, axis.encoder_.count_in_cpr_, 32, 32, true);
    return odCAN->write(txmsg);
}

void CANSimple::set_input_pos_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.input_pos_ = can_getSignal<float>(msg, 0, 32, true);
    axis.controller_.input_vel_ = can_getSignal<int16_t>(msg, 32, 16, true, 0.001f, 0);
    axis.controller_.input_torque_ = can_getSignal<int16_t>(msg, 48, 16, true, 0.001f, 0);
    axis.controller_.input_pos_updated();
}

void CANSimple::set_input_vel_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.input_vel_ = can_getSignal<float>(msg, 0, 32, true);
    axis.controller_.input_torque_ = can_getSignal<float>(msg, 32, 32, true);
}

void CANSimple::set_input_torque_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.input_torque_ = can_getSignal<float>(msg, 0, 32, true);
}

void CANSimple::set_controller_modes_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.config_.control_mode = static_cast<Controller::ControlMode>(can_getSignal<int32_t>(msg, 0, 32, true));
    axis.controller_.config_.input_mode = static_cast<Controller::InputMode>(can_getSignal<int32_t>(msg, 32, 32, true));
}

void CANSimple::set_vel_limit_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.config_.vel_limit = can_getSignal<float>(msg, 0, 32, true);
}

void CANSimple::start_anticogging_callback(const Axis& axis, const can_Message_t& msg) {
    axis.controller_.start_anticogging_calibration();
}

void CANSimple::set_traj_vel_limit_callback(Axis& axis, const can_Message_t& msg) {
    axis.trap_traj_.config_.vel_limit = can_getSignal<float>(msg, 0, 32, true);
}

void CANSimple::set_traj_accel_limits_callback(Axis& axis, const can_Message_t& msg) {
    axis.trap_traj_.config_.accel_limit = can_getSignal<float>(msg, 0, 32, true);
    axis.trap_traj_.config_.decel_limit = can_getSignal<float>(msg, 32, 32, true);
}

void CANSimple::set_traj_inertia_callback(Axis& axis, const can_Message_t& msg) {
    axis.controller_.config_.inertia = can_getSignal<float>(msg, 0, 32, true);
}

void CANSimple::set_linear_count_callback(Axis& axis, const can_Message_t& msg){
    axis.encoder_.set_linear_count(can_getSignal<int32_t>(msg, 0, 32, true));
}

int32_t CANSimple::get_iq_callback(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_IQ;
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    std::optional<float2D> Idq_setpoint = axis.motor_.current_control_.Idq_setpoint_;
    if (!Idq_setpoint.has_value()) {
        Idq_setpoint = {0.0f, 0.0f};
    }
    
    static_assert(sizeof(float) == sizeof(Idq_setpoint->first));
    static_assert(sizeof(float) == sizeof(Idq_setpoint->second));
    can_setSignal<float>(txmsg, Idq_setpoint->first, 0, 32, true);
    can_setSignal<float>(txmsg, Idq_setpoint->second, 32, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::get_vbus_voltage_callback(const Axis& axis) {
    can_Message_t txmsg;

    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_GET_VBUS_VOLTAGE;
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    uint32_t floatBytes;
    static_assert(sizeof(vbus_voltage) == sizeof(floatBytes));
    can_setSignal<float>(txmsg, vbus_voltage, 0, 32, true);

    return odCAN->write(txmsg);
}

void CANSimple::clear_errors_callback(Axis& axis, const can_Message_t& msg) {
    odrv.clear_errors(); // TODO: might want to clear axis errors only
}
void CANSimple::periodic_handler_update(Axis::CANConfig_t& can_conf, const uint32_t id, const uint32_t command_id, const uint32_t rate_ms, bool construct)
{
    if (construct)
    {
        switch (command_id)
        {
            case MSG_GET_MOTOR_ERROR:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_ENCODER_ERROR:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_SENSORLESS_ERROR:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_ENCODER_ESTIMATES:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_ENCODER_COUNT:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_IQ:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_SENSORLESS_ESTIMATES:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_GET_VBUS_VOLTAGE:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            case MSG_ODRIVE_HEARTBEAT:
                can_conf.periodic_handlers[id] = {command_id, rate_ms, 0};
                break;
            default:
                return;
        }
        can_conf.nbr_of_periodic_handlers++;
    }
    else
    {
        // Clear handler, move last handler to position to clear and clear last element
        can_conf.periodic_handlers[id] = can_conf.periodic_handlers[can_conf.nbr_of_periodic_handlers-1];
        can_conf.periodic_handlers[can_conf.nbr_of_periodic_handlers-1] = {0, 0, 0};
        /*
        memset(&can_conf.periodic_handlers[id], 0, sizeof(struct Axis::CANPeriodic));
        if (id < 5-1)
        {
            memmove(&can_conf.periodic_handlers[id], &can_conf.periodic_handlers[id+1], sizeof(struct CANPeriodic)*(can_conf.nbr_of_periodic_handlers-1));
        }*/

        can_conf.nbr_of_periodic_handlers--;
    }
}
void CANSimple::enable_periodic_update(Axis& axis,  const can_Message_t& msg)
{
    const uint32_t periodic_cmd = can_getSignal<uint32_t>(msg, 0, 32, true);
    const uint32_t update_rate_ms = can_getSignal<uint32_t>(msg, 32, 32, true);

    if (periodic_cmd != MSG_GET_MOTOR_ERROR &&
        periodic_cmd != MSG_GET_ENCODER_ERROR &&
        periodic_cmd != MSG_GET_SENSORLESS_ERROR &&
        periodic_cmd != MSG_GET_ENCODER_ESTIMATES &&
        periodic_cmd != MSG_GET_ENCODER_COUNT &&
        periodic_cmd != MSG_GET_IQ &&
        periodic_cmd != MSG_GET_SENSORLESS_ESTIMATES &&
        periodic_cmd != MSG_GET_VBUS_VOLTAGE &&
        periodic_cmd != MSG_ODRIVE_HEARTBEAT)
    {
        return; //Message not a valid get function
    }
    uint32_t id = 0;
    for (auto& handler: axis.config_.can.periodic_handlers)
    {
        if (handler.can_command == periodic_cmd)
        {
            if (update_rate_ms == 0)
            {
                periodic_handler_update(axis.config_.can, id, periodic_cmd, update_rate_ms, false);
            }
            else
            {
                handler.call_rate_ms = update_rate_ms;
            }
            break;
        }
        else if(handler.call_rate_ms == 0)
        {
            periodic_handler_update(axis.config_.can, id, periodic_cmd, update_rate_ms, true);
            break;
        }
        id++;
    }
}

int32_t CANSimple::send_heartbeat(const Axis& axis) {
    can_Message_t txmsg;
    txmsg.id = axis.config_.can.node_id << NUM_CMD_ID_BITS;
    txmsg.id += MSG_ODRIVE_HEARTBEAT;  // heartbeat ID
    txmsg.isExt = axis.config_.can.is_extended;
    txmsg.len = 8;

    can_setSignal(txmsg, axis.error_, 0, 32, true);
    can_setSignal(txmsg, axis.current_state_, 32, 32, true);

    return odCAN->write(txmsg);
}

int32_t CANSimple::call_periodic_handler(Axis& axis, uint32_t command)
{
        switch (command)
        {
            case MSG_GET_MOTOR_ERROR:
                get_motor_error_callback(axis);
                break;
            case MSG_GET_ENCODER_ERROR:
                get_encoder_error_callback(axis);
                break;
            case MSG_GET_SENSORLESS_ERROR:
                get_sensorless_error_callback(axis);
                break;
            case MSG_GET_ENCODER_ESTIMATES:
                get_encoder_estimates_callback(axis);
                break;
            case MSG_GET_ENCODER_COUNT:
                get_encoder_count_callback(axis);
                break;
            case MSG_GET_IQ:
                get_iq_callback(axis);
                break;
            case MSG_GET_SENSORLESS_ESTIMATES:
                get_sensorless_estimates_callback(axis);
                break;
            case MSG_GET_VBUS_VOLTAGE:
                get_vbus_voltage_callback(axis);
                break;
            case MSG_ODRIVE_HEARTBEAT:
                send_heartbeat(axis);
                break;
            default:
                return 0;
        }
        return 1;
}

void CANSimple::send_cyclic(Axis& axis) {
    const uint32_t now = HAL_GetTick();

    for (auto& handler : axis.config_.can.periodic_handlers)
    {
        if (handler.call_rate_ms == 0)
        {
            break;
        }

        if ((now - handler.last_call) >= handler.call_rate_ms)
        {
            if (call_periodic_handler(axis, handler.can_command) >= 0)
            {
                handler.last_call = now;
            }

        }
    }
}
