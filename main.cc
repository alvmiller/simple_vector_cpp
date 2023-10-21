#include <iostream>

#include "simplevector.hpp"

// reset; g++ --std=c++20 -g main.cc -o main.out
// std::cout<<"[TMP] YY XX = "<<XX<<std::endl;

// See:
//    https://en.cppreference.com/w/cpp/container/vector
//    https://github.com/tilir/cpp-graduate/blob/master/14-iterators/wrongit.cc
//    https://github.com/llvm-mirror/libcxx/blob/master/include/vector

//std::vector<int> vect{} - empty vector
//std::vector<int> vect - empty vector
//std::vector<int> vect() - error compl
//std::vector<int> vect(0) - empty vector
//std::vector<int> vect(1) - print 0

void print_sv(const auto& sv)
{
    std::cout << "Print SimpleVector:" << std::endl;
    for (auto&& el : sv) {
        std::cout << el << std::endl;
    }
    return;
}

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char* argv[])
{
#if 1
    try {
        simplecontainer::SimpleVector<int> vb0{};
        print_sv(vb0);

        simplecontainer::SimpleVector<int> v;
        print_sv(v);
    
        simplecontainer::SimpleVector<int> v0(0);
        print_sv(v0);
    
        simplecontainer::SimpleVector<int> v1(1);
        print_sv(v1);

        simplecontainer::SimpleVector<bool> vb{true, false};
        print_sv(vb);

        simplecontainer::SimpleVector<int> vi{1, 2, 3};
        print_sv(vi);

        simplecontainer::SimpleVector<double> vd{1.1, 2.2, 3.3};
        print_sv(vd);

        std::cout << "Distance: " << std::distance(vi.begin(), vi.end()) << std::endl;
    }
    catch (...) {
        std::cout << "Some exception catched!" << std::endl;
    }
#endif

    return 0;
}
