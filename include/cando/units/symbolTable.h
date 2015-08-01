#ifndef	_units_symbolTable_H
#define _units_symbolTable_H

#include <clasp/core/foundation.h>

namespace units
{

#define	UnitsPkg_SYMBOLS
#define DO_SYMBOL(cname,idx,pkg,lispname,export) extern core::Symbol_sp cname;
#include SYMBOLS_SCRAPED_INC_H
#undef DO_SYMBOL
#undef UnitsPkg_SYMBOLS

}; /* units */

#endif /* _units_symbolTable_H */
