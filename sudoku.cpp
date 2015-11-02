// Declarations and functions for the Sudoku project
#include "stdafx.h"
#include <iostream>
#include <limits.h>
#include "d_matrix.h"
#include "d_except.h"
#include <list>
#include <fstream>
#include "Cell.h"
#include "sudoku.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <time.h>
using namespace std;

//typedef int Cell; // The type of the value in a cell
const int Blank = -1;  // Indicates that a cell is blank
 
const int SquareSize = 3;  //  The number of cells in a small square
                           //  (usually 3).  The board has
                           //  SquareSize^2 rows and SquareSize^2
                           //  columns.

const int BoardSize = SquareSize * SquareSize;

const int MinValue = 1;
const int MaxValue = 9;

int numSolutions = 0;

class board
// Stores the entire Sudoku board
{
   public:
	  // constructor with size
      board(int);

	  // clears the board and resets the recursions
      void clear();

	  // must be called after construction, reads from sudoku.txt
      void initialize(ifstream &fin);

	  // outputs
      void print();
	  void printConflicts();

	  // cell check for no value
      bool isBlank(int, int);

	  // get/set for cells by index
      Cell getCell(int, int);
      void setCell(int i,int j, Cell val);
	  
	  // recursive solving function
	  void solve();
	  int recursions = 0;
   private:

      // The following matrices go from 1 to BoardSize in each
      // dimension.  I.e. they are each (BoardSize+1) X (BoardSize+1)
		
      matrix<Cell> value;

	  // rows, columns, and boxes - Referenced as "groups" from here on
	  vector<vector<Cell*>> rows;
	  vector<vector<Cell*>> columns;
	  vector<vector<Cell*>> boxes;

	  // instantiate pointers to the board
	  void initializeCombinations();

	  // possibility checks
	  bool checkForPotentialConflicts(vector<Cell*>& groupToCheck, int val);
	  bool checkUniquePossibility(vector<Cell*>& groupToCheck, int val);

	  // update
	  void updatePossibilities();

	  // conflict check
	  bool checkForExistingConflicts();

	  // assign definite cells based on the rest of the board
	  void makeAssignments();

	  // solved checks
	  bool isSolved();
	  void findFirstBlank(int & x, int & y);
	  void findBlankLeastPossibilities(int &x, int &y);

	  // row,column to box #
	  int squareNumber(int i, int j);
};

board::board(int sqSize)
   : value(BoardSize,BoardSize)
// Board constructor
{
}

void board::clear()
// Clear the entire board.
{
	for (int i = 0; i < BoardSize; i++)
	{
		for (int j = 0; j < BoardSize; j++)
		{
			value[i][j] = Cell(-1);
		}
	}
	recursions = 0;
}

void  board::setCell(int i,int j, Cell val)
// set cell i,j to val and update conflicts
{
	if (i >= BoardSize || i < 0 || j >= BoardSize || j < 0)
		throw new indexRangeException();
	value[i][j] = val;
}

// set up references
void board::initializeCombinations()
{
	for (int r = 0; r < BoardSize; r++)
	{
		vector<Cell*> row;
		for (int c = 0; c < BoardSize; c++)
		{
			row.push_back(&value[r][c]);
		}
		rows.push_back(row); // add each row
	}
	for (int c = 0; c < BoardSize; c++)
	{
		vector<Cell*> column;
		for (int r = 0; r < BoardSize; r++)
		{
			column.push_back(&value[r][c]);
		}
		columns.push_back(column); // add each column
	}
	for (int r = 0; r < BoardSize; r += SquareSize)
	{
		for (int c = 0; c < BoardSize; c += SquareSize)
		{
			vector<Cell*> box;
			for (int innerR = 0; innerR < SquareSize; innerR++)
			{
				for (int innerC = 0; innerC < SquareSize; innerC++)
				{
					box.push_back(&value[r + innerR][c + innerC]);
				}
			}
			boxes.push_back(box); // add each box
		}
	}
}

void board::solve()
// recursive function that solves using backtracking when it cannot find a solution
{
	// optimized function for making required assignments for the 
	// values that are already in the board
	makeAssignments(); 
	updatePossibilities();
	if (checkForExistingConflicts() || isSolved())
		return;

	recursions++;

	matrix<Cell> savedBoard(value);
	int x, y;
	findBlankLeastPossibilities(x, y);
	vector<int> poss = value[x][y].GetPossibilities();
	for (int i = 0; i < poss.size(); i++)
	{
		value[x][y].SetValue(poss[i]);
		solve();
		if (!isSolved())
		{
			value = savedBoard;
		}
		else return;
	}
}

bool board::checkForPotentialConflicts(vector<Cell*>& groupToCheck, int val)
// checks if a value can be added
{
	// lambda expression that uses an iterator through the pointer list looking for val
	// and returns when found or reaching the end
	if (std::find_if(groupToCheck.begin(), groupToCheck.end(),
		[&groupToCheck,val](Cell* c) { return c->GetValue() == val;}) != groupToCheck.end()) {
		return true;
	}
	else return false;
}

bool board::checkUniquePossibility(vector<Cell*>& groupToCheck, int val)
// checks if only one cell in the group can have the value "val"
{
	int matches = 0;
	for (int i = 0; i < BoardSize; i++)
	{
		if (matches > 1) return false; // its either 1 or more than one that we care about
		else if (groupToCheck[i]->IsPossible(val))
			matches++;
	}
	return matches == 1;
}

void board::updatePossibilities()
// called after assignments are made
// this will update each cell's possibilities vector
// based on the other values in its row, column, and box
{
	vector<int> fullElement{ 1,2,3,4,5,6,7,8,9 };

	// iterate through the rows
	// although they are the same checks, each group is separated
	// because it could be solved after any group is set (because cell value is set when possibilities == 0)
	int numNotUpdated = 0;
	for (int row = 0; row < BoardSize; row++)
	{
		vector<int> possibilities;
		vector<int> values;
		for (int i = 0; i < BoardSize; i++)
		{
			int val = rows[row][i]->GetValue();
			if (val > 0)
			{
				values.push_back(val);
			}
		}
		sort(values.begin(), values.end());
		set_difference(fullElement.begin(), fullElement.end(), values.begin(), values.end(),
			std::inserter(possibilities, possibilities.begin()));
		if (possibilities.size() == 0)
		{
			numNotUpdated++;
			continue;
		}
		else
		{
			for (int cell = 0; cell < BoardSize; cell++)
			{
				if (!rows[row][cell]->isAssignedValue())
					rows[row][cell]->SetPossibilities(possibilities);
			}
		}
	}
	if (numNotUpdated == 9)
		return; // solved

	numNotUpdated = 0;
	for (int column = 0; column < BoardSize; column++)
	{
		vector<int> possibilities;
		vector<int> values;
		for (int i = 0; i < BoardSize; i++)
		{
			int val = columns[column][i]->GetValue();
			if (val > 0)
			{
				values.push_back(val);
			}
		}
		sort(values.begin(), values.end());
		set_difference(fullElement.begin(), fullElement.end(), values.begin(), values.end(),
			std::inserter(possibilities, possibilities.begin()));
		if (possibilities.size() == 0)
		{
			numNotUpdated++;
			continue;
		}
		else
		{
			for (int cell = 0; cell < BoardSize; cell++)
			{
				if (!columns[column][cell]->isAssignedValue())
					columns[column][cell]->SetPossibilities(possibilities);
			}
		}
	}
	if (numNotUpdated == 9)
		return; // solved

	numNotUpdated = 0;
	for (int box = 0; box < BoardSize; box++)
	{
		vector<int> possibilities;
		vector<int> values;
		for (int i = 0; i < BoardSize; i++)
		{
			int val = boxes[box][i]->GetValue();
			if (val > 0)
			{
				values.push_back(val);
			}
		}
		sort(values.begin(), values.end());
		set_difference(fullElement.begin(), fullElement.end(), values.begin(), values.end(),
			std::inserter(possibilities, possibilities.begin()));
		if (possibilities.size() == 0)
		{
			numNotUpdated++;
			continue;
		}
		else
		{
			for (int cell = 0; cell < BoardSize; cell++)
			{
				if (!boxes[box][cell]->isAssignedValue())
					boxes[box][cell]->SetPossibilities(possibilities);
			}
		}
	}
	if (numNotUpdated == 9)
		return; // solved
}

bool board::checkForExistingConflicts()
// Makes sure there are no duplicate values in the groups
{
	for (int x = 0; x < BoardSize; x++)
	{
		vector<bool> rowVals(9, false);
		vector<bool> colVals(9, false);
		vector<bool> boxVals(9, false);
		for (int y = 0; y < BoardSize; y++)
		{
			// quick check to see if there is no value assigned and no possibilities left
			if (value[x][y].areNoPossibilities())
				return true;

			// check if there are duplicates through the rows
			int val = rows[x][y]->GetValue();
			if (val > 0)
			{
				if (rowVals[val - 1])
					return true;
				else rowVals[val - 1] = true;
			}

			// ... columns
			val = columns[x][y]->GetValue();
			if (val > 0)
			{
				if (colVals[val - 1])
					return true;
				else colVals[val - 1] = true;
			}

			// ... and boxes
			val = boxes[x][y]->GetValue();
			if (val > 0)
			{
				if (boxVals[val - 1])
					return true;
				else boxVals[val - 1] = true;
			}
		}
	}
	return false;
}

void board::makeAssignments()
// optimized assignments based on finding cells that have to be a certain value
// by looking for a value that can only be matched to one cell in a row, column, or box
{
	// row to column position ->  rows[x][y] == columns[y][x]
	// row to box position -> rows[x][y] == boxes[(x/3)*3+y/3]
	while (true)
	{
		bool assignmentMade = false;
		for (int num = 1; num <= BoardSize; num++)
		{
			for (int r = 0; r < BoardSize; r++)
			{
				for (int c = 0; c < BoardSize; c++)
				{
					if (value[r][c].IsPossible(num) &&
						(checkUniquePossibility(rows[r], num) ||
						checkUniquePossibility(columns[c], num) ||
						checkUniquePossibility(boxes[squareNumber(r,c)], num)) &&
						!checkForPotentialConflicts(rows[r], num) &&
						!checkForPotentialConflicts(columns[c], num) &&
						!checkForPotentialConflicts(boxes[squareNumber(r,c)], num))
					{
						value[r][c].SetValue(num);
						assignmentMade = true;
					}
				}
			}
		}
		if (!assignmentMade) return;
		else updatePossibilities();
	}
}

bool board::isSolved()
// first checks for a blank
// if full, checks if there's a conflict
{
	int r, c;
	findFirstBlank(r, c);
	if ((r < 0 || c < 0) && !checkForExistingConflicts())
		return true;
	else return false;
}

void board::findFirstBlank(int & x, int &y)
// finds the first blank cell from left to right, top to bottom
// changes x and y to -1 to indicate there are no blanks and it is solved
{
	for (x = 0; x < BoardSize; x++)
	{
		for (y = 0; y < BoardSize; y++)
		{
			if (isBlank(x, y))
				return;
		}
	}
	x = -1;
	y = -1;
}

void board::findBlankLeastPossibilities(int & x, int &y)
// finds the blank cell with the least possibilities
{
	int minPossibilities = 9;
	for (int i = 0; i < BoardSize; i++)
	{
		for (int j = 0; j < BoardSize; j++)
		{
			if (isBlank(i, j))
			{
				int numPoss = getCell(i, j).GetNumPossibilities();
				if (numPoss < minPossibilities)
				{
					minPossibilities = numPoss;
					x = i; y = j;
				}
				if (numPoss == 2)
					return;
			}
		}
	}
}

void board::initialize(ifstream &fin)
// Read a Sudoku board from the input file.
{
	char ch;

	clear();
	for (int i = 0; i < BoardSize; i++)
	{
		for (int j = 0; j < BoardSize; j++)
		{
			fin >> ch;

			// If the read char is not Blank
			if (ch != '.')
			{
				setCell(i, j, Cell(ch-'0'));   // Convert char to int with ascii subtraction of 48
			}
			else setCell(i, j, Cell(Blank));
		}
	}
	initializeCombinations();
}

int board::squareNumber(int i, int j)
// Return the square number of cell i,j (counting from left to right,
// top to bottom.  Note that i and j each go from 1 to BoardSize
{
   // Note that (int) i/SquareSize and (int) j/SquareSize are the x-y
   // coordinates of the square that i,j is in.  

   return SquareSize * (i/SquareSize) + j/SquareSize;
}

Cell board::getCell(int i, int j)
// Returns the value stored in a cell.  Throws an exception
// if bad values are passed.
{
   if (i >= 0 && i < BoardSize && j >= 0 && j < BoardSize)
      return value[i][j];
   else
      throw rangeError("bad value in getCell");
}

bool board::isBlank(int i, int j)
// Returns true if cell i,j is blank, and false otherwise.
{
   if (i < 0 || i >= BoardSize || j < 0 || j >= BoardSize)
      throw rangeError("bad value in setCell");
   else return !getCell(i, j).isAssignedValue();
}

void board::print()
// Prints the current board.
{
   for (int i = 0; i < BoardSize; i++)
   {
      if ((i) % SquareSize == 0)
      {
         cout << " -";
	 for (int j = 0; j < BoardSize; j++)
	    cout << "---";
         cout << "-";
	 cout << endl;
      }
      for (int j = 0; j < BoardSize; j++)
      {
	 if ((j) % SquareSize == 0)
	    cout << "|";
	 if (!isBlank(i,j))
	    cout << " " << getCell(i,j).GetValue() << " ";
	 else
	    cout << "   ";
      }
      cout << "|";
      cout << endl;
   }

   cout << " -";
   for (int j = 0; j < BoardSize; j++)
      cout << "---";
   cout << "-";
   cout << endl;
}

void board::printConflicts()
{
	updatePossibilities();
	for (int r = 0; r < BoardSize; r++)
	{
		for (int c = 0; c < BoardSize; c++)
		{
			cout << "[" << r << "][" << c << "]: " << value[r][c] << endl;
		}
	}
}

ostream &operator<<(ostream &ostr, vector<int> &v)
// Overloaded output operator for vector class.
{
   for (int i = 0; i < v.size(); i++)
      ostr << v[i] << " ";
   ostr << endl;
   return ostr;
}

int main()
{
	bool project4B = true;
	bool project4A = !project4B;
	ifstream fin;

	// Read the sample grid from the file.
	string fileName = "sudoku.txt";

	try
	{
		fin.open(fileName.c_str());
		if (!fin)
		{
			cerr << "Cannot open " << fileName << endl;
			exit(1);
		}

		board b1(SquareSize);

		if (project4A)
		{
			int i = 0;
			while (fin && fin.peek() != 'Z')
			{
				b1.initialize(fin);
				cout << "Board #" << ++i << endl;
				b1.print();
				b1.printConflicts();
			}
			int z;
			cin >> z;
		}
		else if (project4B)
		{
			double sumTimes = 0;
			double sumRecursions = 0;
			int numBoards = 0;
			while (fin && fin.peek() != 'Z')
			{
				b1.initialize(fin);
				cout << "Starting Board:" << endl;
				b1.print();

				clock_t startTime = clock();

				b1.solve();
				cout << "Solved Board:" << endl;
				b1.print();

				clock_t execTime = clock() - startTime;
				double time = (float)execTime / CLOCKS_PER_SEC;
				numBoards++;
				cout << "Board #"<< numBoards <<" solved in " << time << " seconds." << endl;
				cout << "Number of recursions: " << b1.recursions << endl << endl;
				sumTimes += time;
				sumRecursions += b1.recursions;
				b1.clear();
			}
			double meanTime = sumTimes / numBoards;
			double meanRecursions = sumRecursions / numBoards;

			cout << endl << "Average Time For Solving = " << meanTime << " seconds" << endl;
			cout << "Average Number of Recursions: " << meanRecursions << endl;

			int z;
			cin >> z;
		}
	}
	catch (indexRangeError &ex)
	{
		cout << ex.what() << endl;
		exit(1);
	}
}


