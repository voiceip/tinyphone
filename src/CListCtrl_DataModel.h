#pragma once

#include <string>
#include <vector>

using namespace std;

struct CListCtrl_DataRecord
{
	CListCtrl_DataRecord()
	{}

	CListCtrl_DataRecord(const string& city, const string& state, const string& country)
		:m_City(city)
		,m_State(state)
		,m_Country(country)
	{}

	string	m_City;
	string	m_State;
	string	m_Country;

	const string& GetCellText(int col, bool title) const
	{
		switch(col)
		{
		case 0: { static string title0("City"); return title ? title0 : m_City; }
		case 1: { static string title1("State"); return title ? title1 : m_State; }
		case 2: { static string title2("Country"); return title ? title2 : m_Country; }
		default:{ static string emptyStr; return emptyStr; }
		}
	}

	int  GetColCount() const { return 3; }
};

class CListCtrl_DataModel
{
	vector<CListCtrl_DataRecord> m_Records;
	int	m_LookupTime;
	int m_RowMultiplier;

public:
	CListCtrl_DataModel()
		:m_RowMultiplier(0)
		,m_LookupTime(0)
	{
		InitDataModel();
	}

	void InitDataModel()
	{
		m_Records.clear();
		m_Records.push_back( CListCtrl_DataRecord("Copenhagen", "Sjaelland", "Denmark") );
		m_Records.push_back( CListCtrl_DataRecord("Aarhus", "Jutland", "Denmark") );
		m_Records.push_back( CListCtrl_DataRecord("Odense", "Fyn", "Denmark") );
		m_Records.push_back( CListCtrl_DataRecord("Malmoe", "Skaane", "Sweeden") );

		if (m_RowMultiplier > 1)
		{
			vector<CListCtrl_DataRecord> rowset(m_Records);
			m_Records.reserve((m_RowMultiplier-1) * rowset.size());
			for(int i = 0 ; i < m_RowMultiplier ; ++i)
			{
				m_Records.insert(m_Records.end(), rowset.begin(), rowset.end());
			}
		}
	}

	const string& GetCellText(size_t lookupId, int col) const
	{
		if (lookupId >= m_Records.size())
		{
			static const string oob("Out of Bound");
			return oob;
		}
		// How many times should we search sequential for the row ?
		for(int i=0; i < m_LookupTime; ++i)
		{
			for(size_t rowId = 0; rowId < m_Records.size(); ++rowId)
			{
				if (rowId==lookupId)
					break;
			}
		}
		return m_Records.at(lookupId).GetCellText(col, false);
	}

	size_t GetRowIds() const { return m_Records.size(); }
	int GetColCount() const { return CListCtrl_DataRecord().GetColCount(); }
	const string& GetColTitle(int col) const { return CListCtrl_DataRecord().GetCellText(col, true); }

	vector<CListCtrl_DataRecord>& GetRecords() { return m_Records; }
	void SetLookupTime(int lookupTimes) { m_LookupTime = lookupTimes; }
	void SetRowMultiplier(int multiply) { if (m_RowMultiplier != multiply ) { m_RowMultiplier = multiply; InitDataModel(); } }
};