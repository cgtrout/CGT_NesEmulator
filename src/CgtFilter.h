#pragma once

//Filter Classes
//ChatGPT generated audio filters
//
#include <iostream>
#include <vector>
#include <cmath>

namespace CgtLib {
/*
================================================================
================================================================
Class ButterworthHighPassFilter
================================================================
================================================================
*/
class ButterworthHighPassFilter {
public:
    ButterworthHighPassFilter(double cutoff_frequency, double sample_rate)
    : previous_input_(0), previous_output_(0) {
        double RC = 1.0 / (2.0 * M_PI * cutoff_frequency);
        double dt = 1.0 / sample_rate;
        alpha_ = RC / (RC + dt);
    }

    double process(double input) {
        double output = alpha_ * previous_output_ + alpha_ * (input - previous_input_);
        previous_input_ = input;
        previous_output_ = output;
        return output;
    }

private:
    double alpha_;
    double previous_input_;
    double previous_output_;
};


/*
================================================================
================================================================
Class ButterworthLowPassFilter
================================================================
================================================================
*/
class ButterworthLowPassFilter {
public:
    ButterworthLowPassFilter(double cutoff_frequency, double sample_rate)
    : previous_output_(0) {
        double RC = 1.0 / (2.0 * M_PI * cutoff_frequency);
        double dt = 1.0 / sample_rate;
        alpha_ = dt / (RC + dt);
    }

    double process(double input) {
        double output = alpha_ * input + (1.0 - alpha_) * previous_output_;
        previous_output_ = output;
        return output;
    }

private:
    double alpha_;
    double previous_output_;
};

}//namespace