#if !defined( FractionAdder_INCLUDED )
#define FractionAdder_INCLUDED

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
 inline void FractionAdder<V>::setDenominator( V d ) {
	denominator = d;
}

template< class V > 
 inline int FractionAdder<V>::add( V num ) { 
	numerator += num;
	int wholes = 0;
	while( numerator > denominator ) {
		numerator -= denominator;
		++wholes;
	}
	return wholes;
}

#endif //FractionAdder_INCLUDED
