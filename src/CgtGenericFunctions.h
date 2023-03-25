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

    template<typename T, typename ReturnType = T>
    ReturnType calculateAverage( const std::vector<T>& buffer_data ) {
        using R = std::common_type_t<T, decltype( buffer_data.size( ) )>;
        R total = std::accumulate( buffer_data.begin( ), buffer_data.end( ), R{} );
        return static_cast<ReturnType>( total / static_cast<R>( buffer_data.size( ) ) );
    }

    template<typename T>
    float calculateAverageFloat( const std::vector<T>& buffer_data ) {
        return calculateAverage<T, float>( buffer_data );
    }

    template<typename T>
    double calculateAverageDouble( const std::vector<T>& buffer_data ) {
        return calculateAverage<T, double>( buffer_data );
    }

}
