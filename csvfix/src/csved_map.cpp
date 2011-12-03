//---------------------------------------------------------------------------
// csved_map.cpp
//
// map from one value to another
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_map.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register map command
//---------------------------------------------------------------------------

static RegisterCommand <MapCommand> rc1_(
	CMD_MAP,
	"map betwen CSV field values"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const MAP_HELP = {
	"map input field values to new values on output\n"
	"usage: csvfix map [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to perform mapping on (default is all)\n"
	"  -fv vals\tcomma-separated list of values to map from\n"
	"  -tv vals\tpossibly empty list oof values to map to\n"
	"  -ic\t\tignore case when mapping (default is to respect case)\n"
	"#ALL"
};

//---------------------------------------------------------------------------
// standard command constructor
//---------------------------------------------------------------------------

MapCommand ::	MapCommand( const string & name,
								const string & desc )
		: Command( name, desc, MAP_HELP ), mIgnoreCase( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FROMV, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TOV, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ICASE, false, 0 ) );
}

//---------------------------------------------------------------------------
// Process flags then map inputs
//---------------------------------------------------------------------------

int MapCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );

	while( io.ReadCSV( mRow ) ) {
		DoMapping();
		io.WriteRow( mRow );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Do mapping for single row. If a list of fields was not specified on
// command line, map all fields.
//---------------------------------------------------------------------------

void MapCommand :: DoMapping()  {

	if ( mFields.size() == 0 ) {
		for ( unsigned int i = 0; i < mRow.size(); i++ ) {
			MapValue( mRow[i] );
		}
	}
	else {
		for ( unsigned int i = 0; i < mFields.size(); i++ ) {
			if ( mFields[i] < mRow.size() ) {
				MapValue( mRow[ mFields[i] ] );
			}
		}
	}
}


//---------------------------------------------------------------------------
// Map single value
//---------------------------------------------------------------------------

void MapCommand :: MapValue( std::string & val ) {
	for ( unsigned int i = 0; i < mFrom.Size(); i++ ) {
		if ( (mIgnoreCase && ALib::Equal( val, mFrom.At(i) ))
				|| ( ! mIgnoreCase && val == mFrom.At(i)) ) {
			if ( mTo.Size() == 0 ) {
				val = "";
			}
			else if ( mTo.Size() == mFrom.Size() ) {
				val = Expand( mTo.At( i ) );
			}
			else {
				val = Expand( mTo.At( mTo.Size() - 1 ) );
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------
// Expand a value being mapped to, which may be a field specifier such as
// $1, $2 etc. To quote the $, use $$, so $$1.00 will be returned as $1.00
//----------------------------------------------------------------------------

string MapCommand :: Expand( const string & val ) {
	if ( ALib::Peek( val, 0 ) == '$'  ) {
		string field = val.substr( 1 );
		if ( ALib::Peek( field, 0 ) == '$' ) {
			return field;
		}
		else {
			if ( ! ALib::IsInteger( field ) ) {
				CSVTHROW( "Invalid field specifier " << val );
			}
			int n = ALib::ToInteger( field ) - 1;
			if ( n < 0 ) {
				CSVTHROW( "Field numbers must be greater than zero at " << val );
			}
			return (unsigned int) n > mRow.size() ? "" : mRow[n];
		}
	}
	else {
		return val;
	}
}

//---------------------------------------------------------------------------
// Process command line flags. Need a list of fields and from and to values
//---------------------------------------------------------------------------

void MapCommand :: ProcessFlags( ALib::CommandLine & cmd ) {
	mIgnoreCase = cmd.HasFlag( FLAG_ICASE );

	if ( cmd.HasFlag( FLAG_COLS ) ) {
		CommaListToIndex( ALib::CommaList( cmd.GetValue( FLAG_COLS )), mFields );
		if ( mFields.size() == 0 ) {
			CSVTHROW( "Field list cannot be empty" );
		}
	}

	mFrom = ALib::CommaList( cmd.GetValue( FLAG_FROMV ));
	if ( mFrom.Size() == 0 ) {
		mFrom.Append("");
	}
	mTo = ALib::CommaList( cmd.GetValue( FLAG_TOV ));
	if ( mTo.Size() > mFrom.Size() ) {
		CSVTHROW( "List of 'to values' longer than list of 'from values" );
	}
}

//------------------------------------------------------------------------

} // end namespace

// end

