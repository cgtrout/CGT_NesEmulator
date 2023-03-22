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
         // Calculate RC (resistor-capacitor) value using the cutoff frequency
        double RC = 1.0 / (2.0 * M_PI * cutoff_frequency);
        
        // Calculate the time step (dt) using the sample rate
        double dt = 1.0 / sample_rate;
        
        // Calculate the alpha (filter coefficient) using RC and dt
        alpha_ = RC / (RC + dt);
    }

    double process(double input) {
        // Calculate the output using the filter coefficient (alpha), previous input, and previous output
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
Class HighPassFilter
================================================================
================================================================
*/
class HighPassFilter {
public:
    HighPassFilter( double cutoff_frequency, double sample_rate ) : 
        cutoff_frequency_( cutoff_frequency ), sample_rate_( sample_rate ),
        RC_( 1.0 / ( cutoff_frequency_ * 2 * 3.14159265 ) ), 
        dt_( 1.0 / sample_rate_ ),
        alpha_( RC_ / ( RC_ + dt_ ) ), 
        x_previous_( 0 ), y_previous_( 0 ) {}

    double process( double x_current ) {
        double y_current = alpha_ * y_previous_ + alpha_ * ( x_current - x_previous_ );
        x_previous_ = x_current;
        y_previous_ = y_current;
        return y_current;
    }

private:
    double cutoff_frequency_;
    double sample_rate_;
    double RC_;
    double dt_;
    double alpha_;
    double x_previous_;
    double y_previous_;
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