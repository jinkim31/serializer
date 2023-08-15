#include <iostream>
#include <sstream>

#include "../src/serializer.h"

using namespace srz;

struct Point : public Serializable
{
    Point(const int& x, const int& y)
    {
        mX = x;
        mY = y;
        sync(&mX, "x");
        sync(&mY, "y");
    }

    std::string toString()
    {
        std::stringstream ss;
        ss<<"("<<mX<<", "<<mY<<")";
        return ss.str();
    }

    int mX, mY;
};

struct Line : public Serializable
{
    Line(const Point& a, const Point& b) : mA(a), mB(b)
    {
        sync(&mA, "a");
        sync(&mB, "b");
    }

    std::string toString()
    {
        std::stringstream ss;
        ss<<mA.toString()<<", "<<mB.toString();
        return ss.str();
    }

    Point mA, mB;
};

int main()
{
    // make line to save
    Line lineSave({100, 200}, {300, 400});
    // save
    std::cout<<"Saving line: "<<lineSave.toString()<<std::endl;
    json save;
    lineSave.save(save);
    std::ofstream fo("out.json");
    fo<< std::setw(4) << save << std::endl;
    fo.close();

    // make line to load
    Line lineLoad({0,0}, {0,0});
    // load
    std::ifstream fi("out.json");
    save = json::parse(fi);
    lineLoad.load(save);
    std::cout<<"Loaded line: "<<lineSave.toString()<<std::endl;
}
