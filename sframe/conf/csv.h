
#ifndef SFRAME_CSV_H
#define SFRAME_CSV_H

#include "Table.h"

namespace sframe {

class CSV
{
public:

	// ����
	static bool Parse(const std::string & content, Table & tbl);

};

}

#endif
