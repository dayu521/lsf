#include<iostream>
#include"doctest/doctest.h"
#include"json.h"

TEST_CASE("testing...") {
    lsf::Json js("2.txt");
    auto b=js.run([](auto,const std::string & m){
       std::cout<<m<<std::endl;
    });
    REQUIRE(b==true);
}
