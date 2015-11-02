#pragma once
#include "stdafx.h"
class Cell
{
public:
	Cell(); // invalid - necessary for matrix template
	Cell(int val);
	~Cell();
	int GetValue();
	void SetValue(int);
	bool SetPossibilities(vector<int>&);
	vector<int> GetPossibilities();
	int GetNumPossibilities();
	bool IsPossible(int);
	bool isAssignedValue();
	bool areNoPossibilities();
	friend bool operator==(const Cell& c1, const Cell& c2);
	friend ostream & operator<<(ostream&, const Cell&);
private:
	int value = -1;
	bool assignedValue = false;
	bool assignedPossibilities = false;
	vector<int> possibilities;
	bool setVectorIntersection(vector<int>& v2);
};

