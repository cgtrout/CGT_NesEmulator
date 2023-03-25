#pragma once

#include <vector>

namespace CgtLib {

    template<typename T>
    double calculateSlope( const std::vector<T>& buffer_data ) {
        int n = buffer_data.size( );
        std::vector<double> time( n );

        // Assuming time or index is the independent variable (0, 1, 2, ..., n-1)
        for( int i = 0; i < n; i++ ) {
            time[ i ] = i;
        }

        // Calculate the mean of x (time) and y (buffer_data)
        double mean_x = std::accumulate( time.begin( ), time.end( ), 0.0 ) / n;
        double mean_y = std::accumulate( buffer_data.begin( ), buffer_data.end( ), 0.0 ) / n;

        // Calculate the numerator and denominator of the slope formula
        double numerator = 0.0, denominator = 0.0;
        for( int i = 0; i < n; i++ ) {
            numerator += ( time[ i ] - mean_x ) * ( buffer_data[ i ] - mean_y );
            denominator += ( time[ i ] - mean_x ) * ( time[ i ] - mean_x );
        }

        // Calculate the slope
        double slope = numerator / denominator;

        return slope;
    }

    template<typename T>
    double calculateAverage( const std::vector<T>& buffer_data ) {
        auto total = std::accumulate( buffer_data.begin( ), buffer_data.end( ), 0 );
        return (double)total / (double)buffer_data.size( );
    }
}
