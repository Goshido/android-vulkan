#pragma once
#include "GeomExport.h"
#include "matrix3.h"

#define STACK_DEPTH 32 // default stack depth

/*! \sa  Class Matrix3.\n\n
\par Description:
A stack of Matrix3's (4x3 matrix). All methods of this class are implemented by
the system. No error (bounds) checking is performed.  */
class Matrix3Stack
{
public:
	/*! \remarks Constructor. Creates a new 32 element stack of Matrix3s. */
	GEOMEXPORT Matrix3Stack();
	/*! \remarks Constructor. Creates a new <b>depth</b> element stack of
	Matrix3s. */
	GEOMEXPORT Matrix3Stack(int depth);
	/*! \remarks Destructor. Frees the stack. */
	GEOMEXPORT ~Matrix3Stack();

	Matrix3Stack(const Matrix3Stack&) = delete;
	Matrix3Stack& operator=(const Matrix3Stack&) = delete;

	/*! \remarks Replaces the item on the top of the stack with the specified
	matrix.
	\par Parameters:
	<b>const Matrix3 \&m</b>\n\n
	The matrix to replace the matrix at the top of the stack.
	\return  Always TRUE. */
	bool replace(const Matrix3& m)
	{
		stk[index] = m;
		return true;
	}
	/*! \remarks Pushes the specified matrix onto the stack.
	\par Parameters:
	<b>const Matrix3 \&m</b>\n\n
	The matrix to push on the top of the stack.
	\return  TRUE if there is still room on the stack; otherwise FALSE. */
	bool push(const Matrix3& m)
	{
		stk[index++] = m;
		return index < maxDepth;
	}
	/*! \remarks Duplicates the matrix on the top of the stack.
	\return  TRUE if there is still room on the stack after the <b>dup</b>;
	otherwise FALSE. */
	bool dup(void)
	{
		stk[index + 1] = stk[index];
		return ++index < maxDepth;
	}
	/*! \remarks Concatenates the specified matrix with the matrix on the top
	of the stack (performs matrix multiplication).
	\par Parameters:
	<b>const Matrix3 \&m</b>\n\n
	The matrix to multiply (<b>m * stack[top]</b>).
	\return  Always TRUE. */
	bool concat(const Matrix3& m)
	{
		stk[index] = m * stk[index];
		return true;
	}
	/*! \remarks Returns the matrix on the top of the stack. */
	Matrix3& get(void)
	{
		return stk[index];
	}
	/*! \remarks Pops the matrix from the stack.
	\return  The matrix at the top of the stack. */
	Matrix3& pop(void)
	{
		return stk[index--];
	}
	/*! \remarks Removes the matrix at the top of the stack.
	\return  TRUE if the number of items on the stack is \>= 0 (after removing
	the matrix); otherwise 0. */
	bool remove(void)
	{
		return --index >= 0;
	}
	/*! \remarks Resets the stack to contain zero items.
	\return  Always TRUE. */
	bool reset(void)
	{
		index = 0;
		stk[0].IdentityMatrix();
		return true;
	}

private:
	int maxDepth;
	int index;
	Matrix3* stk;
};
