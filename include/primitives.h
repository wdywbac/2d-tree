#pragma once

#include <ostream>
#include <cmath>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

class Point {
public:

    Point(double x, double y)
            : m_x(x), m_y(y) {}

    Point(const Point &p) : m_x(p.x()), m_y(p.y()) {}

    double x() const { return m_x; }

    double y() const { return m_y; }

    double distance(const Point &other) const
    {
        double a = m_x - other.m_x;
        double b = m_y - other.m_y;
        return sqrt(a * a + b * b);
    }

    bool operator<(const Point & p) const { return m_x < p.x() || (m_x == p.x() && m_y < p.y()); }

    bool operator>(const Point & p) const { return m_x > p.x() || (m_x == p.x() && m_y > p.y()); }

    bool operator<=(const Point & p) const { return m_x < p.x() || (m_x == p.x() && m_y <= p.y()); }

    bool operator>=(const Point & p) const { return m_x > p.x() || (m_x == p.x() && m_y >= p.y()); }

    bool operator==(const Point & p) const { return m_x == p.x() && m_y == p.y(); }

    bool operator!=(const Point & p) const { return m_x != p.x() || m_y != p.y(); }

    friend std::ostream &operator<<(std::ostream & os, const Point & p)
    {
        os << "x = " << p.x() << "; y = " << p.y();
        return os;
    }
private:

    double m_x;
    double m_y;

};

class Rect {
public:

    Rect(const Point &left_bottom, const Point &right_top)
            : m_left_bottom(left_bottom), m_right_top(right_top) {}

    double xmin() const { return m_left_bottom.x(); }

    double ymin() const { return m_left_bottom.y(); }

    double xmax() const { return m_right_top.x(); }

    double ymax() const { return m_right_top.y(); }

    double distance(const Point & p) const
    {
        double dx = std::max({xmin() - p.x(), 0.0, p.x() - xmax()});
        double dy = std::max({ymin() - p.y(), 0.0, p.y() - ymax()});
        return sqrt(dx * dx + dy * dy);
    }

    bool contains(const Point & p) const
    { return p.x() > xmin() && p.x() < xmax() && p.y() > ymin() && p.y() < ymax(); }

    bool intersects(const Rect & rect) const
    { return !(m_right_top < rect.m_left_bottom || rect.m_right_top < m_left_bottom); }

private:

    Point m_left_bottom;
    Point m_right_top;
};

namespace rbtree {

    class PointSet {
    public:

        using ForwardIt = std::set<Point>::iterator;

        PointSet() = default;

        bool empty() const { return m_set.empty(); }

        std::size_t size() const { return m_set.size(); }

        void put(const Point &point) { m_set.emplace(point); }

        bool contains(const Point &point) const { return m_set.find(point) != m_set.end(); }

        // second iterator points to an element out of range
        std::pair<ForwardIt, ForwardIt> range(const Rect & rect) const
        {
            cnt_range++;
            m_ans_range.clear();
            auto it = begin();
            while (it != end())
            {
                if (rect.contains(*it))
                {
                    m_ans_range.emplace(*it);
                }
                it++;
            }
            range_cash.insert({cnt_range, m_ans_range});
            return {range_cash.find(cnt_range)->second.begin(), range_cash.find(cnt_range)->second.end()};
        }

        ForwardIt begin() const { return m_set.begin(); }

        ForwardIt end() const { return m_set.end(); }

        std::optional<Point> nearest(const Point & p) const
        {
            double min_dist = std::numeric_limits<double>::max();
            auto it = begin();
            auto it_ans = end();
            while (it != end())
            {
                double temp = p.distance(*it);
                if (temp < min_dist)
                {
                    min_dist = temp;
                    it_ans = it;
                }
                it++;
            }
            return *it_ans;
        }

        // second iterator points to an element out of range
        std::pair<ForwardIt, ForwardIt> nearest(const Point &p, std::size_t k) const
        {
            cnt_nearest++;
            m_ans_nearest.clear();
            std::map<double, Point> map;
            auto it = begin();
            while (it != end())
            {
                double temp_dist = p.distance(*it);
                map.insert({temp_dist, *it});
                it++;
            }
            auto it_ans = map.begin();
            for (std::size_t i = 0; i < k; i++) {
                if (it_ans == map.end())
                {
                    break;
                }
                m_ans_nearest.emplace((*it_ans).second);
                it_ans++;
            }
            nearest_cash.insert({cnt_nearest, m_ans_nearest});
            return {nearest_cash.find(cnt_nearest)->second.begin(), nearest_cash.find(cnt_nearest)->second.end()};
        }

        friend std::ostream &operator<<(std::ostream & os, const PointSet & point_set)
        {
            auto it = point_set.begin();
            while (it != point_set.end())
            {
                os << *it << std::endl;
                it++;
            }
            return os;
        }

    private:
        std::set<Point> m_set;

        mutable std::set<Point> m_ans_range;
        mutable int cnt_range = 0;
        mutable std::map<int, std::set<Point>> range_cash;

        mutable std::set<Point> m_ans_nearest;
        mutable int cnt_nearest = 0;
        mutable std::map<int, std::set<Point>> nearest_cash;

    };

}

namespace kdtree {

    struct Node
    {
        Point m_point = {0, 0};
        std::shared_ptr<Node> m_left;
        std::shared_ptr<Node> m_right;
        std::shared_ptr<Node> m_next_dfs;
        bool m_split = true; // true for vertical; false for horizontal

        Node(const Point &p, bool split, std::shared_ptr<Node> next_dfs)
                : m_point(p), m_left(), m_right(), m_next_dfs(std::move(next_dfs)), m_split(split) {}

        explicit Node(const std::shared_ptr<Node>& node)
                : m_point(node->m_point), m_left(node->m_left), m_right(node->m_right), m_next_dfs(node->m_next_dfs), m_split(node->m_split) {}

        void unlink() {
            auto next = m_next_dfs;
            m_next_dfs = nullptr;
            m_left = nullptr;
            m_right = nullptr;
            if (next) next->unlink();
        }

        ~Node() = default;
    };

    class PointSetIterator: public std::iterator<std::forward_iterator_tag, std::shared_ptr<Node>>
    {
    public:
        explicit PointSetIterator(std::shared_ptr<Node> node) : m_node(std::move(node)) {}

        Point &operator*()
        {
            return m_node->m_point;
        }

        Point *operator->()
        {
            return &m_node->m_point;
        }

        PointSetIterator &operator++()
        {
            m_node = m_node->m_next_dfs;
            return *this;
        }
        // ++i

        PointSetIterator operator++(int)
        {
            PointSetIterator it = *this;
            ++*this;
            return it;
        }
        // i++

        std::shared_ptr<Node> node()
        {
            return this->m_node;
        }

        bool operator==(const PointSetIterator &it) const
        {
            return m_node == it.m_node;
        }

        bool operator!=(const PointSetIterator &it) const
        {
            return !(it == *this);
        }

    private:
        std::shared_ptr<Node> m_node;
    };

    class PointSet {
    public:

        using ForwardIt = PointSetIterator;

        ~PointSet();

        bool empty() const;

        std::size_t size() const;

        void put(const Point &);

        bool contains(const Point &) const;

        std::pair<ForwardIt, ForwardIt> range(const Rect &) const;

        ForwardIt begin() const;

        ForwardIt end() const;

        std::optional<Point> nearest(const Point &) const;

        std::pair<ForwardIt, ForwardIt> nearest(const Point &, std::size_t) const;

        friend std::ostream &operator<<(std::ostream &, const PointSet &);

    private:

        std::shared_ptr<Node> m_root{};
        std::shared_ptr<Node> m_begin{};
        std::shared_ptr<Node> m_end{};
        int m_size = 0;

        mutable std::shared_ptr<Node> m_ans_range_root{};
        mutable std::shared_ptr<Node> m_ans_range_begin{};
        mutable std::shared_ptr<Node> m_ans_range_end{};
        mutable size_t range_count = 0;
        mutable std::map<size_t, std::pair<ForwardIt, ForwardIt>> range_cash{};

        mutable std::shared_ptr<Node> m_ans_nearest_root{};
        mutable std::shared_ptr<Node> m_ans_nearest_begin{};
        mutable std::shared_ptr<Node> m_ans_nearest_end{};
        mutable size_t nearest_count = 0;
        mutable std::map<size_t, std::pair<ForwardIt, ForwardIt>> nearest_cash{};

        void clear(const std::shared_ptr<Node>& node) const;

        void put(const std::shared_ptr<Node>& node, const Point &p, bool split, const std::shared_ptr<Node>& next_dfs) const;

        bool contains(const std::shared_ptr<Node>& node, const Point &p) const;

        void range(const std::shared_ptr<Node>& node, const Rect &rect) const;

        Point nearest(const std::shared_ptr<Node>& node, const Point &p, const Point& nearest) const;

        void nearest(const std::shared_ptr<Node>& node, bool split, std::map<double, std::shared_ptr<Node>> &nodes,
                     double closest, const Point &p, const std::size_t &k, size_t size) const;

    };

}