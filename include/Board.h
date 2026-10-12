#pragma once

#include "Types.h"

#include <array>
#include <cstdint>
#include <vector>

struct Neighbors {
    std::array<Point, 4> points{};
    int count = 0;

    const Point* begin() const { return points.data(); }
    const Point* end() const { return points.data() + count; }
};

struct Group {
    Stone color = Stone::Empty;
    std::vector<Point> stones;
    std::vector<Point> liberties;
};

enum class PlaceStatus { Ok, OffBoard, Occupied, Suicide };

struct PlaceResult {
    PlaceStatus status = PlaceStatus::Ok;
    std::vector<Point> captured;
};

class Board {
public:
    static constexpr int maxSize = 19;

    explicit Board(int size = maxSize);

    int size() const { return size_; }
    int area() const { return size_ * size_; }
    bool contains(Point point) const;
    Stone at(Point point) const;
    void set(Point point, Stone stone);
    std::uint64_t hash() const { return hash_; }
    int stoneCount(Stone stone) const;
    bool isEmpty() const;

    Neighbors neighbors(Point point) const;
    Group groupAt(Point point) const;
    int libertyCount(Point point) const;
    PlaceResult place(Point point, Stone stone);

    friend bool operator==(const Board& left, const Board& right);

private:
    int index(Point point) const { return point.row * size_ + point.col; }

    int size_;
    std::vector<Stone> cells_;
    std::uint64_t hash_ = 0;
};

bool isValidBoardSize(int size);
