#pragma once
/*
================================================================
================================================================
Class FractionAdder

- used to add fractional values together (that have same 
  denominator only)

================================================================
================================================================
*/
template< class V >
 class FractionAdder {
  public:	
	FractionAdder( V num, V den );

	void setNumerator( V num );
	void setDenominator( V den );

	V getNumerator() const;
    V getDenominator() const;
	
	//returns number of whole numbers if fraction addition
	//has overflowed to >= 1 whole numbers
	//else returns 0
	int add( V numerator );
  private:
	V numerator;
	V denominator;
};

//
//  implementation
//
template< class V >
 FractionAdder<V>::FractionAdder( V num, V den ) : 
	numerator( num ), 
	denominator( den ) {}

template< class V > 
 inline void FractionAdder<V>::setNumerator( V num ) {
	numerator = num;
}

template< class V > 
inline V FractionAdder<V>::getNumerator() const {
    return numerator;
}

template< class V > 
inline V FractionAdder<V>::getDenominator() const {
    return denominator;
}

template< class V > 
inline void FractionAdder<V>::setDenominator( V d ) {
	if (d == 0) {
        throw std::invalid_argument("Denominator cannot be zero");
    }
	denominator = d;
}

template< class V > 
inline int FractionAdder<V>::add( V num ) { 
    numerator += num;
    int wholes = numerator / denominator;
    numerator %= denominator;
    return wholes;
}
