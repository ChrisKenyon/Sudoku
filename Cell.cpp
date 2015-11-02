#include "stdafx.h"
#include "Cell.h"
#include <algorithm>
#include <iterator>
Cell::Cell()
{
}

Cell::Cell(int val)
{
	if (val > 9) throw new argumentException();
	if (val > 0)
	{
		SetValue(val);
	}
	else value = -1;
	//possibilities.resize(10);
}

Cell::~Cell()
{
}

int Cell::GetValue()
{
	return value;
}

void Cell::SetValue(int val)
{
	possibilities.clear();
	value = val;
	assignedValue = true;
}

// must send a sorted list in poss
// returns true if it could set the value or false if not
bool Cell::SetPossibilities(vector<int>& poss)
{
	if (assignedValue) throw new programmingErrorException();
	else if (!assignedPossibilities)
	{
		for (int i = 0; i < poss.size(); i++)
		{
			possibilities.push_back(poss[i]);
		}
		assignedPossibilities = true;
		return false;
	}
	else return setVectorIntersection(poss);
}

vector<int> Cell::GetPossibilities()
{
	return possibilities;
}

int Cell::GetNumPossibilities()
{
	return possibilities.size();
}

bool Cell::IsPossible(int lookup)
{
	if (assignedValue) return false;
	for (int x = 0; x < possibilities.size(); x++)
		if (lookup == possibilities[x]) return true;
	return false;
}

bool Cell::isAssignedValue()
{
	return assignedValue;
}

bool Cell::areNoPossibilities()
{
	return (!assignedValue && possibilities.size() == 0);
}

// this function sets the intersection in place for possibilities
bool Cell::setVectorIntersection(vector<int>& v2)
{
	vector<int> poss(possibilities);
	possibilities.clear();
	set_intersection(poss.begin(), poss.end(), v2.begin(), v2.end(),
		std::inserter(possibilities, possibilities.begin()));

	if (possibilities.size() == 1)
	{
		SetValue(possibilities[0]);
		return true;
	}
	else return false;
}

ostream & operator<<(ostream & cstr, const Cell & cell)
{
	if (cell.assignedValue)
	{
		cstr << "The value is: " << cell.value;
	}
	else
	{
		cstr << "Possibilities are: ";
		for (int i = 0; i < cell.possibilities.size(); i++)
		{
			cstr << cell.possibilities[i] << " ";
		}
	}
	return cstr;
}

bool operator==(const Cell& c1, const Cell& c2) 
{
	return c1.value == c2.value;
}
