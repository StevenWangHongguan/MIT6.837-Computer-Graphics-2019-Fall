#include "matrixstack.h"

MatrixStack::MatrixStack()
{
	// Initialize the matrix stack with the identity matrix.
	m_matrices.push_back(Matrix4f::identity());
}

void MatrixStack::clear()
{
	// Revert to just containing the identity matrix.
	while (m_matrices.size() > 1)
		m_matrices.pop_back();
}

Matrix4f MatrixStack::top()
{
	// Return the top of the stack
	// return Matrix4f();
	if (!m_matrices.empty())
		return m_matrices.back();

    return Matrix4f::identity();
}

void MatrixStack::push( const Matrix4f& m )
{
	// Push m onto the stack.
	// The new top should be "old * m", so that conceptually the new matrix
    // is applied first in right-to-left evaluation.
	Matrix4f old = top();
	m_matrices.push_back(old * m);
}

void MatrixStack::pop()
{
	// Remove the top element from the stack
	if (!m_matrices.empty())
		m_matrices.pop_back();
}
