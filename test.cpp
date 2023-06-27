#include <vector>
#include <memory>
#include <iostream>
using namespace std;

struct tt{
    public:
    string b = "123d";
    int* a = new int[10]{1,2,3,4,5,6,7,8,9,10};
};


unique_ptr<tt> f(){
    unique_ptr<tt> a1{new tt{}};
    return a1;
}


int main(){
    tt a{};
    tt b = move(a);
    a.b[0]='3';
    cout<<a.b<<endl;
    return 0;
}