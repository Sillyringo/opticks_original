// name=rng ; gcc $name.cc -std=c++11 -lstdc++ -o /tmp/$name && /tmp/$name

// https://stackoverflow.com/questions/31417957/encapsulated-random-number-generator-in-c-11-using-boost

#include <iostream>
#include <iomanip>
#include <random>

template <typename T>
struct RNG
{
    std::mt19937_64 engine ;
    std::uniform_real_distribution<T>  dist ; 

    RNG(unsigned seed_=0u) : dist(0, 1) { engine.seed(seed_); }
    T operator()(){ return dist(engine) ; }
};


template <typename T>
void test_rng( const std::function<T()>& fn )
{
    T a ; 
    T b ; 
    bool done ; 
    unsigned count = 0 ; 

    do {
        a = fn() ; 
        b = fn() ; 
        std::cout 
            << " count " << std::setw(10) <<  count 
            << " a " << std::fixed << std::setw(10) << std::setprecision(4) <<  a 
            << " b " << std::fixed << std::setw(10) << std::setprecision(4) <<  b 
            << std::endl
            ; 

        done = a > 0.99 && b > 0.99 ; 
        count += 1 ;   

    } while( done == false ) ; 


    std::cout 
        << " result " 
        << " count " << count 
        << " a " << a 
        << " b " << b 
        << std::endl
        ; 
}


int main()
{
    unsigned seed = 1u ; 
    //RNG<double> rng0(seed) ; 
    //test_rng<double>(rng0); 

    RNG<float> rng1(seed) ; 
    test_rng<float>(rng1); 

    return 0;
}


