//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2011 Rainer Kupke
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GLClientAndServerBuffer_h_
#define _GLClientAndServerBuffer_h_


#include <vector>
#include <GG/Base.h>


namespace GG {

///////////////////////////////////////////////////////////////////////////
// GLBufferBase common base class for Buffer classes
///////////////////////////////////////////////////////////////////////////
class GG_API GLBufferBase
{
public:
    GLBufferBase() = default;

    /** Required to automatically drop server buffer in case of delete. */
    virtual ~GLBufferBase();

    // use this if you want to make sure that two buffers both
    // have server buffers or not, drops the buffer for mixed cases
    void harmonizeBufferType(GLBufferBase& other);

protected:
    // drops the server buffer if one exists
    void dropServerBuffer();

    GLuint b_name = 0;
};

///////////////////////////////////////////////////////////////////////////
// GLClientAndServerBufferBase
// template class for buffers with different types of content
///////////////////////////////////////////////////////////////////////////
template <typename vtype>
class GG_API GLClientAndServerBufferBase : public GLBufferBase
{
private:
    GLClientAndServerBufferBase(); // default ctor forbidden,
                                   // buffer needs to know number
                                   // of elements per item
public:
    GLClientAndServerBufferBase(std::size_t elementsPerItem);
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool        empty() const;

    // pre-allocate space for item data
    void reserve(std::size_t num_items);

    // store items, buffers usually store tupels, convenience functions
    // do not use while server buffer exists
    void store(vtype item);
    void store(vtype item1, vtype item2);
    void store(vtype item1, vtype item2, vtype item3);
    void store(vtype item1, vtype item2, vtype item3, vtype item4);

    // try to store the buffered data in a server buffer
    void createServerBuffer();

    // drops a server buffer if one exists,
    // clears the client side buffer
    void clear();

protected:
    std::vector<vtype>  b_data;
    std::size_t         b_size = 0;
    std::size_t         b_elements_per_item = 0;

    // used in derived classes to activate the buffer
    // implementations should use glBindBuffer, gl...Pointer if
    // server buffer exists (b_name! = 0), just gl...Pointer otherwise
    virtual void activate() const = 0;
};

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer specialized class for RGBA color values
///////////////////////////////////////////////////////////////////////////
class GG_API GLRGBAColorBuffer : public GLClientAndServerBufferBase<unsigned char>
{
public:
    GLRGBAColorBuffer();
    void store(const Clr& color);
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer specialized class for 2d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL2DVertexBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GL2DVertexBuffer();
    void store(const Pt& pt);
    void store(X x, Y y);
    void store(X x, float y);
    void store(float x, Y y);
    void store(float x, float y);
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer specialized class for texture coordinate data
///////////////////////////////////////////////////////////////////////////
class GG_API GLTexCoordBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GLTexCoordBuffer();
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL3DVertexBuffer specialized class for 3d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL3DVertexBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GL3DVertexBuffer();
    void store(float x, float y, float z);
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLNormalBuffer specialized class for 3d normal data
///////////////////////////////////////////////////////////////////////////
class GG_API GLNormalBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GLNormalBuffer();
    void store(float x, float y, float z);
    void activate() const override;
};

}


#endif
