#ifndef BVECTOR_HPP
#define BVECTOR_HPP

#include <new>
#include <cstring>

// MEANX : Simplified a lot, the original code had maybe alignment issues
/**
* @author Berenger
* @version 0.5
* @date 24 novembre 2009
* @file BVector.hpp
* @package Utils
* @brief Basic vecctor based on a memory area
*
* This Vector used a memory area, so you have to use it as an array.
* So it is easy and fast to put data at start/end position.
* But it takes a long time to insert or remove value in the center arrea
*
* @must You must give the buffer size if you know it!
*
* @example BVector<int> vec(100);
* @example for(int i = 0 ; i < 10 ;++i) vec.pushBack(i);
* @example
* @example vec[0] = 99;
* @example vec[2] = 100;
* @example
* @example vec.insert(0u,25);
* @example vec.insert(5u,25);
* @example
* @example for(int i = 0 ; i < 5 ;++i) vec.popBack();
*
* @example for(BVector<int>::iterator iter( vec.begin() ); iter != vec.end() ; ++iter){
* @example 	(*iter)+=5;
* @example }
*/

#define DefaultSize 5

template<class T>
class BVector {
protected:
        T* buffer;                                  /**< memory area*/

        int capacity;                               /**< memory capacity*/
        static const int SizeOfT = sizeof(T);	   /**< size of the object*/

        unsigned int currentIndex;                                 /**< start index*/

public:
        /**
        *@brief constructor
        */
        BVector(){
                this->buffer = new T[DefaultSize];// MEANX
                this->capacity = DefaultSize;
                this->currentIndex = 0;
        }

        /**
        *@brief constructor
        *@param inSize the buffer size
        *@param inPointOfStart the point of start [0;1]
        */
        BVector(const int inSize){
                this->buffer = new T[inSize];
                this->capacity = inSize;
                this->currentIndex=0;
        }

        /**
        *@brief destructor
        */
        virtual ~BVector(){

                delete [] this->buffer;
                this->buffer=NULL;
        }

        /**
        *@brief get the buffer capacity
        *@return the buffer capacity
        */
        int getCapacity() const{
                return this->capacity;
        }

        /**
        *@brief set the buffer capacity
        *@param in_capacity to change the capacity
        */
        void setCapacity(int in_capacity);

        /**
        *@brief get a const reference of a given value
        *@param inPosition the query position
        *@return the value
        */
        T& operator[](const int inPosition ){
                return this->buffer[inPosition];
        }

        /**
        *@brief get a const reference of a given value
        *@param inPosition the query position
        *@return the value
        */
        const T& operator[](const int inPosition ) const{
                return this->buffer[inPosition];
        }

        /**
        *@brief Append a value
        *@param inValue the value to append
        */
        virtual void append ( const T & inValue )
        {

                setCapacity( size()+1);
                this->buffer[this->currentIndex++]=inValue;
        }

        /**
        *@brief Append a value
        *@param other the vector to append
        */
        virtual void append ( const BVector<T> & other ){

                setCapacity(size() + other.size());

                for(int index = 0 ; index < other.currentIndex ; ++index )
                {
                        this->buffer[this->currentIndex++]=other.buffer[index];
                }
        }

        /**
        *@brief delete all, then size = 0
        */
        void clear(){
                this->currentIndex=0;
        }

        /**
        *@brief count the value
        *@param inValue the value to test
        *@return the value occured number
        */
        unsigned int size() const{
                return this->currentIndex;
        }

        /**
        *@brief test if the vector is empty
        *@return true if vector is empty
        */
        bool empty() const{
                return !this->currentIndex;
        }

        /**
        *@brief insert a value at a certain position
        *@param inPosition where to insert
        *@param inValue the value to put
        */
        void insert ( int inPosition, T inValue );

        /**
        *@brief pop the last node
        */
        void popBack(){
                if(this->currentIndex)
                {
                        --this->currentIndex;
                }
        }
        /**
        *@brief pop the first node
        */
        void popFront(){
                if(this->currentIndex)
                {
                        memmove(buffer,buffer+1,sizeof(T)*(this->currentIndex-1));
                        --this->currentIndex;
                }
        }

        /**
        *@brief push a new node in back
        *@param inValue the new value
        */
        void pushBack( const T & inValue )
        {
                append(inValue);
        }
        /**
        *@brief push a new node in front
        *@param inValue the new value
        */
        void pushFront( const T & inValue )
        {
                 if(!this->currentIndex) {append(inValue);return;}
                 setCapacity( size()+1);
                 memmove(this->buffer+1,this->buffer,sizeof(T)*(this->currentIndex));
                 this->buffer[0]=inValue;
                 this->currentIndex++;

        }
        /**
        *@brief remove a value at a position
        *@param inPosition the position to delete
        */
        void removeAt ( int inPosition );

        /**
        *@brief set the vector from another
        *@param other the vector to copy
        *@return the current vector
        */
        BVector<T> & operator= ( const BVector<T> & other ){
                clear();
                append(other);
                return *this;
        }

};

template <typename T>
void BVector<T>::setCapacity(int in_capacity){
        const int currentSize = this->currentIndex;

        if( in_capacity < this->capacity)
        {
                return; // no shrink...
        }
        // Grow by at least 50%
        int targetCap=(this->capacity*3)/2;
        if(targetCap>in_capacity) in_capacity=targetCap;

        T* nbuffer = new T[in_capacity];

        memcpy(&nbuffer[0],&this->buffer[0],sizeof(T)*currentSize);

        delete [] this->buffer;
        this->buffer = nbuffer;
        this->capacity = in_capacity;
}
template <typename T>
void BVector<T>::insert ( int inPosition, T inValue )
{
    int last=this->currentIndex;

    if(last==inPosition)
    {
        append(inValue);
        return;
    }
    setCapacity(last+1);
    int tail=last-inPosition;
    memmove(buffer+inPosition+1,buffer+inPosition,sizeof(T)*tail);
    buffer[inPosition]=inValue;
    this->currentIndex++;
}

template <typename T>
void BVector<T>::removeAt( int inPosition )
{
    if(inPosition==currentIndex-1)
    {
            popBack();
            return;
    }
    int tail=this->currentIndex-inPosition-1;
    memmove(buffer+inPosition,buffer+inPosition+1,sizeof(T)*tail);
    this->currentIndex--;
}


// EOF






#endif

