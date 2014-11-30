#include "Grid.h"
#include <assert.h>
#include <cmath>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

int main() {

}

Grid::Grid(float width, float length, int segs)
    : _width(width), _length(length), _widthSegs(segs), _lengthSegs(segs), _widthVtex(segs + 1), _lengthVtex(segs + 1)
{
    _vertices = new float[_widthSegs * _lengthSegs];
}

Grid::Grid(float width, float length, int segs, float* vertexHeights)
    : _width(width), _length(length), _widthSegs(segs), _lengthSegs(segs), _widthVtex(segs + 1), _lengthVtex(segs + 1)
{
    _vertices = vertexHeights;
}

Grid::~Grid(void)
{
    delete [] _vertices;
}

float Grid::GetWidth() const
{
    return _width;
}

float Grid::GetLength() const
{
    return _length;
}

int Grid::GetWidthSegs() const
{
    return _widthSegs;
}

int Grid::GetLengthSegs() const
{
    return _lengthSegs;
}

int Grid::GetWidthVertices() const
{
    return _widthVtex;
}

int Grid::GetLengthVertices() const
{
    return _lengthVtex;
}

int Grid::GetTotalVertices() const
{
    return _widthVtex * _lengthVtex;
}

float* Grid::GetVertexHeights() const
{
    return _vertices;
}

void Grid::SetVertexHeightWrap(int u, int v, float height)
{
    u = (u + _widthSegs) % _widthSegs;
    v = (v + _lengthSegs) % _lengthSegs;

    *(_vertices + u * _lengthSegs + v) = height;
}

float Grid::GetVertexHeightWrap(int u, int v) const
{
    u = (u + _widthSegs) % _widthSegs;
    v = (v + _lengthSegs) % _lengthSegs;

    return *(_vertices + u * _lengthSegs + v);
}

void Grid::Clear()
{
    int total = _widthSegs * _lengthSegs;
    for (int i = 0; i < total; i++)
    {
        _vertices[i] = 0.0;
    }
}

void Grid::Diamond(int u, int v, int r, float rand)
{
    float avg = (GetVertexHeightWrap(u - r, v - r)
        + GetVertexHeightWrap(u - r, v + r)
        + GetVertexHeightWrap(u + r, v - r)
        + GetVertexHeightWrap(u + r, v + r)) / 4.0f;

    SetVertexHeightWrap(u, v, avg + rand);
}

void Grid::Square(int u, int v, int r, float rand)
{
    float avg = (GetVertexHeightWrap(u - r, v)
        + GetVertexHeightWrap(u + r, v)
        + GetVertexHeightWrap(u, v - r)
        + GetVertexHeightWrap(u, v + r)) / 4.0f;

    SetVertexHeightWrap(u, v, avg + rand);
}

void Grid::DiamondSquare(unsigned long seed, float rough, float height)
{
    Clear();

    boost::mt19937 engine;
    engine.seed(seed);
    float d = height;

    // Shift will divide radius in half, except 1 >> 1 becomes 0.
    for (int r = _widthSegs / 2; r > 0; r >>= 1) {
        // Before each iteration.
        boost::random::uniform_real_distribution<float> dist(-d, d);

        for (int i = 0; i < _widthSegs; i += 2 * r)
        {
            for (int j = 0; j < _lengthSegs; j += 2 * r)
            {
                Diamond(i + r, j + r, r, dist(engine));
                Square(i + r, j, r, dist(engine));
                Square(i, j + r, r, dist(engine));
            }
        }

        d *= pow(2.0f, -1.0f + rough); // After each iteration.
    }
}
