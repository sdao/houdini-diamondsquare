#pragma once

/** Represents a grid mesh that can be deformed. */
class Grid
{
protected:
    const float _width;
    const float _length;
    const int _widthSegs;
    const int _lengthSegs;
    const int _widthVtex;
    const int _lengthVtex;
    float* _vertices;

    void Diamond(int u, int v, int radius, float rand);
    void Square(int u, int v, int radius, float rand);

public:
    /**
    Creates a new Grid with the vertices uninitialized. Use the Clear() method to zero out all of the vertex heights.
    \param width the width along the U-axis
    \param length the length along the V-axis
    \param segs the number of faces along the U and V-axes
    */
    Grid(float width, float length, int segs);

    /**
    Creates a new Grid with the vertices initialized to the specified array.
    \param width the width along the U-axis
    \param length the length along the V-axis
    \param segs the number of faces along the U and V-axes
    \param vertices the heights of the vertices; this array must be of size (widthSegs + 1) * (lengthSegs + 1)
    */
    Grid(float width, float length, int segs, float* vertexHeights);

    ~Grid(void);

    /** Gets the display width. */
    float GetWidth() const;

    /** Gets the display height. */
    float GetLength() const;

    /** Gets the number of faces along the U-axis. */
    int GetWidthSegs() const;

    /** Gets the number of faces along the V-axis. */
    int GetLengthSegs() const;

    /** Gets the number of vertices along the U-axis. */
    int GetWidthVertices() const;

    /** Gets the number of vertices along the V-axis. */
    int GetLengthVertices() const;

    /** Gets the total number of vertices. */
    int GetTotalVertices() const;

    /**
    Gets the array of heights corresponding to each UV coordinate on the grid.
    The vertices are arranged U-major; e.g. UV coords (0, 0.1), (0, 0.2), (0, 0.3), ..., (0.1, 0.1), (0.1, 0.2), (0.1, 0.3), ..., (1.0, 1.0).
    */
    float* GetVertexHeights() const;

    /**
    Sets the height at the given point on the grid.
    If u or v is out of bounds, it will be wrapped around.
    */
    void SetVertexHeightWrap(int u, int v, float height);

    /**
    Gets the height at the given point on the grid.
    If u or v is out of bounds, it will be wrapped around.
    */
    float GetVertexHeightWrap(int u, int v) const;

    /**
    Zeroes out all of the values on the height map.
    */
    void Clear();

    /**
    Fills the grid up with a diamond-square generated terrain.
    Rough should be on [0.0, 1.0], where 0.0 is the least rough and 1.0 is the most.
    */
    void DiamondSquare(unsigned long seed, float rough, float height);
};
