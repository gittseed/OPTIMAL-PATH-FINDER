
#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <vector>
#include <functional>
#include <set>

namespace AStar
{
    struct Vec2i
    {
        int x, y;

        bool operator == (const Vec2i& coordinates_);

        bool operator!=(const Vec2i& coordinates_) const {
        return (x != coordinates_.x && y != coordinates_.y);
        
    }
    };

    using uint = unsigned int;
    using HeuristicFunction = std::function<uint(Vec2i, Vec2i)>;
    using CoordinateList = std::vector<Vec2i>;

    struct Node
    {
        uint G, H;
        Vec2i coordinates;
        Node *parent;

        Node(Vec2i coord_, Node *parent_ = nullptr);
        uint getScore();
    };

    using NodeSet = std::vector<Node*>;

    class Generator
    {
        bool detectCollision(Vec2i coordinates_);
        Node* findNodeOnList(NodeSet& nodes_, Vec2i coordinates_);
        void releaseNodes(NodeSet& nodes_);

    public:
        Generator();
        void setWorldSize(Vec2i worldSize_);
        void setDiagonalMovement(bool enable_);
        void setHeuristic(HeuristicFunction heuristic_);
        CoordinateList findPath(Vec2i source_, Vec2i target_);
        void addCollision(Vec2i coordinates_);
        void removeCollision(Vec2i coordinates_);
        void clearCollisions();

    //private:
        HeuristicFunction heuristic;
        CoordinateList direction, walls;
        Vec2i worldSize;
        uint directions;
    };

    class Heuristic
    {
        static Vec2i getDelta(Vec2i source_, Vec2i target_);

    public:
        static uint manhattan(Vec2i source_, Vec2i target_);
        static uint euclidean(Vec2i source_, Vec2i target_);
        static uint octagonal(Vec2i source_, Vec2i target_);
    };
}

#endif 
