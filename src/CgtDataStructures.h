//CircularBuffer class
//
//For when we want to gather data in a circular buffer, but don't care
//about the order in which we get the data back
template <class T>
class CircularBuffer {
    public:
    
    CircularBuffer(int size) : buffer(size), index(0) {}
    
    //add elem to buffer accounting for wrap around
    void add(T elem);
    
    //simply gets the value at given index
    T operator[](int i) { return buffer[i]; }
    
    //get size of underlying buffer
    size_t size() { return buffer.size(); }
    
    //iterators
    typename std::vector<T>::iterator begin( ) { return buffer.begin( ); }
    typename std::vector<T>::iterator end( ) { return buffer.end( ); }
    
    //get pointer to buffer
    T* getBufferPtr() { return buffer.data(); }
    
    //get underlying vector
    const std::vector<T>& getVector() { return buffer; }
    
    private:
    
    void increaseIndex();
    
    std::vector<T> buffer;
    int index;
};

template <class T>
void CircularBuffer<T>::add(T elem) { 
    buffer[index] = elem;
    increaseIndex();
}

template <class T>
void CircularBuffer<T>::increaseIndex() {
    index++;
    if( index == buffer.size()) {
        index = 0;
    }
}



