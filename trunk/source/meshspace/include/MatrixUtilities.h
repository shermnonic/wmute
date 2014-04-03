#ifndef MATRIXUTILITIES_H
#define MATRIXUTILITIES_H

namespace MatrixUtilities {

// http://stackoverflow.com/questions/13290395/how-to-remove-a-certain-row-or-column-while-using-eigen-library-c
template <typename Derived1>
void removeRow(Derived1& matrix, unsigned int rowToRemove)
{
    unsigned numRows = (unsigned)matrix.rows()-1;
    unsigned numCols = (unsigned)matrix.cols();

    if( rowToRemove < numRows )
        matrix.block(rowToRemove,0,numRows-rowToRemove,numCols) = matrix.block(rowToRemove+1,0,numRows-rowToRemove,numCols);

    matrix.conservativeResize(numRows,numCols);
}

// http://stackoverflow.com/questions/13290395/how-to-remove-a-certain-row-or-column-while-using-eigen-library-c
template <typename Derived1>
void removeColumn(Derived1& matrix, unsigned int colToRemove)
{
    unsigned numRows = (unsigned)matrix.rows();
    unsigned numCols = (unsigned)matrix.cols()-1;

    if( colToRemove < numCols )
        matrix.block(0,colToRemove,numRows,numCols-colToRemove) = matrix.block(0,colToRemove+1,numRows,numCols-colToRemove);

    matrix.conservativeResize(numRows,numCols);
}
	
}; // namespace MatrixUtilities

#endif // MATRIXUTILITIES_H
