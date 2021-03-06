/*
    File: coordinateArray.h
*/
/*
Open Source License
Copyright (c) 2016, Christian E. Schafmeister
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
This is an open source license for the CANDO software from Temple University, but it is not the only one. Contact Temple University at mailto:techtransfer@temple.edu if you would like a different license.
*/
/* -^- */
       
       
//
// (C) 2004 Christian E. Schafmeister
//


/*
 *      File:   coordinateArray.h
 *
 *      Description:
 *              Routines to maintain a vector of Vector3's
 */

#ifndef CoordinateArray_H
#define CoordinateArray_H

#include <vector>
#include <clasp/core/foundation.h>
#include <clasp/core/object.h>
#include <clasp/core/lispVector.h>
//#include "geom/render.fwd.h"
#include <cando/geom/geomPackage.fwd.h>
#include <cando/geom/vector3.h>
#include <cando/geom/matrix.h>
#include <clasp/core/lispStream.fwd.h>



namespace geom
{
  SMART(DisplayList);

  SMART(CoordinateArray);
  class CoordinateArray_O : public core::Vector_O
  {
    LISP_CLASS(geom,GeomPkg,CoordinateArray_O,"CoordinateArray",core::Vector_O);
  public:
#if 0
    bool fieldsp() const { return true; };
    void	fields(core::Record_sp node);
#endif
  public:
    static CoordinateArray_sp make(gc::Nilable<core::Fixnum_sp> fnsize,core::List_sp vals);
  public:
    void initialize();
  public:
    virtual core::List_sp encode();
    virtual void decode(core::List_sp);
  private:
    vector<Vector3>	_Points;
  private:
    void fillFromList(core::List_sp vals);
  public:
    static CoordinateArray_sp create(uint size)
    {
      GC_ALLOCATE(CoordinateArray_O,a);
      a->resize(size);
      return a;
    }
    static CoordinateArray_sp create(core::List_sp vertexCons);
  public:
    typedef vector<Vector3>::iterator	iterator;
  public:

    CoordinateArray_O( const CoordinateArray_O& orig );

    virtual std::vector<core::cl_index> dimensions() const {
      vector<core::cl_index> dims;
      dims.emplace_back(this->_Points.size());
      return dims;
    }

    INHERIT_SEQUENCE virtual core::T_sp elt(int index) const { return this->getElementObject(index); };
    INHERIT_SEQUENCE virtual core::T_sp setf_elt(int index, core::T_sp value) { return this->setElementObject(index,value); };

    virtual gc::Fixnum dimension() const { return this->_Points.size(); };

    iterator	begin();
    iterator	end();
    void zeroAllCoordinates();
    void add(CoordinateArray_sp c);
    void multiplyByScalar(double d);


	



    double distanceTo(CoordinateArray_sp arr);

    gc::Fixnum	length() const { return this->size(); };
CL_NAME("size");
CL_DEFMETHOD     uint	size() const { return this->_Points.size(); };
    Vector3& getElement(uint i);
    Vector3 value_getElement(uint i);
    core::T_sp getElementObject(uint i) const;
    void	setElement(uint i, Vector3 pos );
    core::T_sp setElementObject(uint i, core::T_sp val);
    void	appendElement(const Vector3& pos) { this->_Points.push_back(pos); };
CL_NAME("coordinate-array-append");
CL_DEFMETHOD     void	append(const Vector3& pos) { this->_Points.push_back(pos); };
    void	push_back(const Vector3& pos) { this->_Points.push_back(pos); };
    void	resize(int i);
    void	clear() { this->_Points.clear();};
    void	transform(const Matrix& transMatrix);
    void	multiplyBy(double multiplier);

    string __repr__() const;
    string	asString() const;

#ifdef RENDER
    Render_sp asDisplayListOfPoints();
#endif

//	string parseFromStream(core::Stream_sp stream);
    void writeToStream(string const& info, core::T_sp stream);

    CoordinateArray_sp	copy() const;
    virtual core::T_sp deepCopy() const { return this->copy();};
    virtual core::T_sp shallowCopy() const { return this->copy();};
    DEFAULT_CTOR_DTOR(CoordinateArray_O);
  };




  SMART(CoordinateArrayWithHash);
  class CoordinateArrayWithHash_O : public CoordinateArray_O
  {
    LISP_CLASS(geom,GeomPkg,CoordinateArrayWithHash_O,"CoordinateArrayWithHash",CoordinateArray_O);
//	DECLARE_SERIALIZE();
  public:
    void initialize();
  public:
//	void	archiveBase(core::ArchiveP node);
  private:
    uint	_Hash;
  public:
    static CoordinateArrayWithHash_sp create(uint sz);
  public:
    CoordinateArrayWithHash_O( const CoordinateArrayWithHash_O& orig );


CL_NAME("coordinate-array-with-hash-gethash");
CL_DEFMETHOD     uint getHash() { return this->_Hash;};
CL_NAME("coordinate-array-with-hash-setHash");
CL_DEFMETHOD     void setHash(uint h) { this->_Hash = h;};

    DEFAULT_CTOR_DTOR(CoordinateArrayWithHash_O);
  };


};


TRANSLATE(geom::CoordinateArray_O);
TRANSLATE(geom::CoordinateArrayWithHash_O);
#endif          /* ifndef CoordinateArray_H */
