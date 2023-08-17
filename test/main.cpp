#include <iostream>
#include <sstream>
#include <serializer.h>

using namespace srz;

struct Point : public Serializable
{
    Point() : Point(0, 0)
    {
    }

    Point(const int& x, const int& y)
    {
        mName = "test";
        sync(&mX, "x");
        sync(&mY, "y");
        sync(&mName, "name");
        mX = x;
        mY = y;
    }

    std::string toString()
    {
        std::stringstream ss;
        ss<<"("<<mX<<", "<<mY<<")";
        return ss.str();
    }

    int mX, mY;
    std::string mName;
};

struct Line : public Serializable
{
    Line() : Line({0, 0}, {0, 0})
    {

    }
    Line(const Point& a, const Point& b)
    {
        mPoints.get().push_back(a);
        mPoints.get().push_back(b);
        sync(&mPoints, "points");
    }

    std::string toString()
    {
        std::stringstream ss;
        ss<<mPoints.get()[0].toString()<<", "<<mPoints.get()[1].toString();
        return ss.str();
    }

    SerializableVector<Point> mPoints;
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
