
#include "odrive_main.h"
#include <Drivers/STM32/stm32_system.h>
#include <bitset>

TorqueSensor::TorqueSensor(TIM_HandleTypeDef* timer, Stm32Gpio adc_gpio) : 
        timer_(timer), adc_gpio_(adc_gpio)
{
}

bool TorqueSensor::apply_config(ODriveIntf::MotorIntf::MotorType motor_type) {

    config_.parent = this;
    return true;

}

void TorqueSensor::setup() {

    HAL_TIM_Encoder_Start(timer_, TIM_CHANNEL_ALL);

    // Do GPIO pin setting here
    adc_gpio_ = get_gpio(config_.torquesensor_gpio_pin);
    adc_gpio_.config(GPIO_MODE_INPUT, GPIO_MODE_ANALOG); 

    // adc_gpio.subscribe() > set an ISR for the pin > do we need it? 
    // probably not...

}

void TorqueSensor::set_error(void) {
    torque_estimate_valid_ = false;
}

// bool TorqueSensor::do_checks(){
//     return error_ == ERROR_NONE;
// }


// This function should only sample data when called by a high-priority ISR
void TorqueSensor::sample_now() {

    torque_voltage_meas_ = get_adc_voltage(get_gpio(config_.torquesensor_gpio_pin));

    // I think we don't need this, but let's leave it here for now...
    // Sample all GPIO digital input data registers, used for HALL sensors for example.
    // for (size_t i = 0; i < sizeof(ports_to_sample) / sizeof(ports_to_sample[0]); ++i) {
    //     port_samples_[i] = ports_to_sample[i]->IDR;
    // }

}

// I think we don't need this, but let's leave it here for now...
// bool TorqueSensor::read_sampled_gpio(Stm32Gpio gpio) {
//     for (size_t i = 0; i < sizeof(ports_to_sample) / sizeof(ports_to_sample[0]); ++i) {
//         if (ports_to_sample[i] == gpio.port_) {
//             return port_samples_[i] & gpio.pin_mask_;
//         }
//     }
//     return false;
// }


// Update the torque estimate based on the newest sampled value
bool TorqueSensor::update() {

    // Update measured displacement
    torque_dx_estimate_ = config_.K_VtoX * torque_voltage_meas_;
    // Update measured torque
    torque_nm_estimate_ = config_.K_XtoM * torque_dx_estimate_;

    // Torque filtering
    // TODO

    // Output from TorqueSensor to Controller
    torque_estimate_ = torque_nm_estimate_;

    return true;

}
