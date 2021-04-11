#include "primitives.h"

#include <iostream>

int main()
{
    kdtree::PointSet p{};
    if (p.empty()) std::cout << "PointSet is empty" << std::endl;
    std::cout << p.size() << std::endl;

    p.put(Point(0, 0));
    p.put(Point(1., 1.));
    p.put(Point(.4, .4));
    p.put(Point(.5, .5));
    p.put(Point(0.1, 0.1));
//    std::cout << std::endl << "PointSet was filled with points:" << std::endl;
//    if (!p.empty()) std::cout << "PointSet contains smth" << std::endl;
//    std::cout << p.size() << std::endl;
//
//    if (p.contains(Point(0,0))) std::cout << "PointSet contains (0; 0)" << std::endl;
//    if (!p.contains(Point(0.5,0))) std::cout << "PointSet does not contain (0.5; 0)" << std::endl;
//
////    auto it = p.begin();
////    std::cout << "First element of PointSet if (0.; 0.): " << std::endl;
////    if (*it->x() == 0.) std::cout << "abscissa is correct" << std::endl;
////    if (*it->y() == 0.) std::cout << "ordinate is correct" << std::endl;
//
//    std::cout << std::endl;
//    auto n = p.nearest(Point(.4, .4));
//    if (n.has_value()) std::cout << "Nearest to point (0.4; 0.4) was found" << std::endl;
//    if (Point(.4, .4) ==  *n) std::cout << "Nearest point is (0.4; 0.4)" << std::endl;
//
//    std::cout << std::endl;
////    Point po = p.nearest(Point(.4, .4));
    auto m = p.nearest(Point(.4, .4), 3);
//    std::cout << *m.first;
    int cnt = 0;
    auto it3 = m.first;
    while (it3 != m.second) {
        std::cout << *it3 << std::endl;
        ++it3;
        cnt++;
    }
    std::cout << cnt << std::endl;

    auto m1 = p.nearest(Point(.4, .4), 2);
//    std::cout << *m.first;
    int cnt1 = 0;
    auto it31 = m1.first;
    while (it31 != m1.second) {
        std::cout << *it31 << std::endl;
        ++it31;
        cnt1++;
    }
    std::cout << cnt1 << std::endl;

    std::cout << std::endl;
    std::cout << std::endl << "Points inside rectangle {(0.3; 0.3); (0.7; 0.7)}:" << std::endl;
    auto range = p.range(Rect(Point(0.3, 0.3), Point(.7, .7)));
    auto range1 = p.range(Rect(Point(0.0, 0.0), Point(1.1, 1.1)));

    std::cout << 1 << std::endl;
    kdtree::PointSet ans1;
    auto it11 = range1.first;
    auto it21 = range1.second;
    while (it11 != it21) {
        std::cout << *it11 << std::endl;
        ans1.put(*it11);
        ++it11;
    }
    std::cout << ans1.size() << std::endl;

    auto range2 = p.range(Rect(Point(0.3, 0.3), Point(.7, .7)));
    kdtree::PointSet ans;
    auto it1 = range.first;
    auto it2 = range.second;
    while (it1 != it2) {
        std::cout << *it1 << std::endl;
        ans.put(*it1);
        ++it1;
    }
    std::cout << ans.size() << std::endl;

    kdtree::PointSet ans2;
    auto it12 = range2.first;
    auto it22 = range2.second;
    while (it12 != it22) {
        std::cout << *it1 << std::endl;
        ans2.put(*it12);
        ++it12;
    }
    std::cout << ans2.size() << std::endl;

//    std::cout << std::endl << "Points inside rectangle {(0.6; 0.6); (2; 2)}:" << std::endl;
//    auto range1 = p.range(Rect(Point(0.6, 0.6), Point(2, 2)));
//    kdtree::PointSet ans1;
//    auto it11 = range1.first;
//    auto it21 = range1.second;
//    while (it11 != it21) {
//        std::cout << *it11 << std::endl;
//        ans1.put(*it11);
//        ++it11;
//    }
//    std::cout << ans1.size() << std::endl;
    return 0;
}
