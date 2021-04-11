#include "primitives.h"
#include <memory>
#include <iostream>

namespace kdtree
{

    PointSet::~PointSet()
    {
        if (m_begin) m_begin->unlink();
        for (size_t i = 1; i <= range_count; i++)
        {
            auto it = range_cash.find(i)->second.first.node();
            if (it) it->unlink();
        }
        for (size_t i = 1; i <= nearest_count; i++)
        {
            auto it = nearest_cash.find(i)->second.first.node();
            if (it) it->unlink();
        }
    }

    bool PointSet::empty() const
    {
        return m_root == nullptr;
    }

    std::size_t PointSet::size() const
    {
        return m_size;
    }

    void PointSet::put(const Point &p)
    {
        if (m_root == nullptr)
        {
            m_size++;
            m_root = std::make_shared<Node>(p, true, m_end);
            m_begin = m_root;
            return;
        }
        if (!contains(p))
        {
            m_size++;
            put(m_root, p, true, m_end);
            if (m_begin->m_left != nullptr)
            {
                m_begin = m_begin->m_left;
            }
        }
    }

    void PointSet::put(const std::shared_ptr<Node>& node, const Point &p, bool split, const std::shared_ptr<Node>& next_dfs) const
    {
        if ((node->m_split && p.x() < node->m_point.x()) || (!node->m_split && p.y() < node->m_point.y()))
        {
            if (node->m_left == nullptr)
            {
                node->m_left = std::make_shared<Node>(p, !split, node);
            }
            else
            {
                put(node->m_left, p, !split, node);
            }
        }
        else
        {
            if (node->m_right == nullptr)
            {
                node->m_right = std::make_shared<Node>(p, !split, next_dfs);
                node->m_next_dfs = node->m_right;
            }
            else
            {
                put(node->m_right, p, !split, next_dfs);
                if (node->m_next_dfs->m_left != nullptr)
                {
                    node->m_next_dfs = node->m_next_dfs->m_left;
                }
            }
        }
    }

    bool PointSet::contains(const Point &p) const
    {
        return contains(m_root, p);
    }

    bool PointSet::contains(const std::shared_ptr<Node>& node, const Point &p) const
    {
        if (node == nullptr)
        {
            return false;
        }

        if (p == node->m_point)
        {
            return true;
        }

        if ((node->m_split && p.x() < node->m_point.x()) ||
            (!node->m_split && p.y() < node->m_point.y()))
        {
            return contains(node->m_left, p);
        }
        else
        {
            return contains(node->m_right, p);
        }
    }

    std::pair<PointSet::ForwardIt, PointSet::ForwardIt> PointSet::range(const Rect &rect) const
    {
        range_count++;
        m_ans_range_root = nullptr;
        m_ans_range_begin = nullptr;
        m_ans_range_end = nullptr;
        if (!empty())
        {
            range(m_root, rect);
        }

        range_cash.insert({range_count, {PointSetIterator(m_ans_range_begin), PointSetIterator(m_ans_range_end)}});
        return {range_cash.find(range_count)->second.first, range_cash.find(range_count)->second.second};
    }

    void PointSet::range(const std::shared_ptr<Node>& node, const Rect &rect) const
    {
        if (node == nullptr)
        {
            return;
        }

        if (rect.contains(node->m_point))
        {
            if (m_ans_range_root == nullptr)
            {
                m_ans_range_root = std::make_shared<Node>(node->m_point, true, m_ans_range_end);
                m_ans_range_begin = m_ans_range_root;
            }
            else
            {
                put(m_ans_range_root, node->m_point, true, m_ans_range_end);
                if (m_ans_range_begin->m_left != nullptr)
                {
                    m_ans_range_begin = m_ans_range_begin->m_left;
                }
            }
        }

        double min = (node->m_split) ? rect.xmin() : rect.ymin();
        double max = (node->m_split) ? rect.xmax() : rect.ymax();
        double coord = (node->m_split) ? node->m_point.x() : node->m_point.y();
        if (min <= coord && coord <= max)
        {
            range(node->m_left, rect);
            range(node->m_right, rect);
        }
        else if (min > coord)
        {
            range(node->m_right, rect);
        }
        else
        {
            range(node->m_left, rect);
        }

    }

    PointSet::ForwardIt PointSet::begin() const
    {
        return PointSetIterator(m_begin);
    }

    PointSet::ForwardIt PointSet::end() const
    {
        return PointSetIterator(m_end);
    }

    std::optional<Point> PointSet::nearest(const Point &p) const
    {
        if (!empty())
        {
            return nearest(m_root, p, m_root->m_point);
        }
        return {};
    }

    Point PointSet::nearest(const std::shared_ptr<Node>& node, const Point &p, const Point &cur_nearest) const
    {
        if (node == nullptr)
        {
            return cur_nearest;
        }

        double point_coord = (node->m_split) ? p.x() : p.y();
        double node_coord = (node->m_split) ? node->m_point.x() : node->m_point.y();

        Point temp_near1(nearest(point_coord > node_coord ? node->m_right : node->m_left, p,
                                 node->m_point.distance(p) < cur_nearest.distance(p) ? node->m_point : cur_nearest));
        Point temp_near2(nearest(point_coord > node_coord ? node->m_left : node->m_right, p, temp_near1));

        if (temp_near1.distance(p) > std::abs(node_coord - point_coord))
        {
            return temp_near1.distance(p) > temp_near2.distance(p) ? temp_near2 : temp_near1;
        }
        else
        {
            return temp_near1;
        }
    }

    std::pair<PointSet::ForwardIt, PointSet::ForwardIt> PointSet::nearest(const Point &p, std::size_t k) const
    {
        nearest_count++;
        m_ans_nearest_root = nullptr;
        m_ans_nearest_begin = nullptr;
        m_ans_nearest_end = nullptr;
        std::map<double, std::shared_ptr<Node>> nodes;
        nearest(m_root, true, nodes, std::numeric_limits<double>::max(), p, k, 0);

        auto it_ans = nodes.begin();
        for (std::size_t i = 0; i < k; i++) {
            if (it_ans == nodes.end())
            {
                break;
            }
            if (m_ans_nearest_root == nullptr)
            {
                m_ans_nearest_root = std::make_shared<Node>((*it_ans).second->m_point, true, m_ans_nearest_end);
                m_ans_nearest_begin = m_ans_nearest_root;
            }
            else
            {
                put(m_ans_nearest_root, (*it_ans).second->m_point, true, m_ans_nearest_end);
                if (m_ans_nearest_begin->m_left != nullptr)
                {
                    m_ans_nearest_begin = m_ans_nearest_begin->m_left;
                }
            }
            it_ans++;
        }

        nearest_cash.insert({nearest_count, {PointSetIterator(m_ans_nearest_begin), PointSetIterator(m_ans_nearest_end)}});
        return {nearest_cash.find(nearest_count)->second.first, nearest_cash.find(nearest_count)->second.second};
    }

    void PointSet::nearest(const std::shared_ptr<Node>& node, bool split, std::map<double, std::shared_ptr<Node>> &nodes,
                           double closest, const Point &p, const std::size_t &k, size_t size) const
    {
        if (node == nullptr)
            return;
        double temp_dist = p.distance(node->m_point);
        if (closest > temp_dist)
        {
            closest = temp_dist;
        }
        nodes.insert({temp_dist, node});
        size++;

        double diff = split ? p.x() - node->m_point.x() : p.y() - node->m_point.y();
        std::shared_ptr<Node> node1(diff < 0 ? (node->m_left ? node->m_left : nullptr) : (node->m_right ? node->m_right : nullptr));
        std::shared_ptr<Node> node2(diff < 0 ? (node->m_right ? node->m_right : nullptr) : (node->m_left ? node->m_left : nullptr));

        nearest(node1, !split, nodes, closest, p, k, size);
        if (diff < closest || size < k)
        {
            nearest(node2, !split, nodes, closest, p, k, size);
        }

    }

    std::ostream &operator<<(std::ostream &os, const PointSet &p)
    {
        auto it = p.begin();
        while (it != p.end())
        {
            os << *it << std::endl;
            ++it;
        }
        return os;
    }
}