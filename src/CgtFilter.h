#pragma once

//Filter Classes
//ChatGPT generated audio filters
//

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <bitset>
#include <iterator>

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
    ButterworthHighPassFilter(float cutoff_frequency, float sample_rate)
    : previous_input_(0), previous_output_(0) {
         // Calculate RC (resistor-capacitor) value using the cutoff frequency
        float RC = 1.0f / (2.0f * (float)M_PI * cutoff_frequency);
        
        // Calculate the time step (dt) using the sample rate
        float dt = 1.0f / sample_rate;
        
        // Calculate the alpha (filter coefficient) using RC and dt
        alpha_ = RC / (RC + dt);
    }

    float process(float input) {
        // Calculate the output using the filter coefficient (alpha), previous input, and previous output
        float output = alpha_ * previous_output_ + alpha_ * (input - previous_input_);
        
        previous_input_ = input;
        previous_output_ = output;
        
        return output;
    }

private:
    float alpha_;
    float previous_input_;
    float previous_output_;
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
    HighPassFilter( float cutoff_frequency, float sample_rate )
        : cutoff_frequency_( cutoff_frequency ),
        sample_rate_( sample_rate ),
        RC_( 1.0f / ( cutoff_frequency_ * 2 * static_cast<float>( M_PI ) ) ),
        dt_( 1.0f / sample_rate_ ),
        alpha_( RC_ / ( RC_ + dt_ ) ),
        x_previous_( 0 ),
        y_previous_( 0 ) {}

    float process( float x_current ) {
        //y[n] = α * y[n-1] + α * (x[n] - x[n-1])
        //alpha_ = 0.99f;
        float y_current = alpha_ * y_previous_ +  alpha_ * (x_current - x_previous_);
        x_previous_ = x_current;
        y_previous_ = y_current;
        return y_current;
    }

private:
    float cutoff_frequency_;
    float sample_rate_;
    float RC_;
    float dt_;
    float alpha_;
    float x_previous_;
    float y_previous_;
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
    ButterworthLowPassFilter(float cutoff_frequency, float sample_rate)
    : previous_output_(0) {
        float RC = 1.0f / ( 2.0f * (float)M_PI * cutoff_frequency );
        float dt = 1.0f / sample_rate;
        alpha_ = dt / (RC + dt);
    }

    float process(float input) {
        float output = alpha_ * input + (1.0f - alpha_) * previous_output_;
        previous_output_ = output;
        return output;
    }

private:
    float alpha_;
    float previous_output_;
};

}//namespace