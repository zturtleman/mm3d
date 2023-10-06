/*  hpagemake.c
 *
 *  Copyright (c) 2021 Zack Middleton
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 *
 *  See the COPYING file for full license text.
 */

// This is a C implementation of mm3d's util/hpagemake.pl that can be
// used without installing Perl and HTTP::Template module.
// It only implements a subset of HTTP::Template functionality needed
// for MM3D documentation.

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <limits.h>

// GNU Hurd doesn't define this.
// TODO: Technically max path length should be dynamically allocated.
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define LAZY_LOAD // Referenced files (e.g., SECLEFT=<secleft.htm) are only loaded if they are used.

typedef struct strpair_s {
	char *key;
	char *value;
	int valueReadFromFile;
} strpair;

typedef struct sublist_s {
	strpair *vars;
	struct sublist_s *templatesub; // Variables for expanding in template files referenced by 'vars'
} sublist;

void TemplateReplace( const char *basepath, const char *template_, sublist *sub1, sublist *sub2, char **output );

void *safe_malloc( size_t size ) {
	void *data = malloc( size );

	if ( !data ) {
		fprintf( stderr, "error: out of memory\n" );
		exit( EXIT_FAILURE );
	}

	return data;
}

void *safe_realloc( void *ptr, size_t size ) {
	void *data = realloc( ptr, size );

	if ( !data ) {
		fprintf( stderr, "error: out of memory\n" );
		exit( EXIT_FAILURE );
	}

	return data;
}

char *my_strcasestr( const char *haystack, const char *needle ) {
	if ( !haystack || !haystack[0] || !needle || !needle[0] ) {
		return NULL;
	}

	while ( haystack[0] ) {
		int i;
		for ( i = 0; needle[i]; i++ ) {
			if ( tolower( (unsigned char)haystack[i] ) != tolower( (unsigned char)needle[i] ) ) {
				break;
			}
		}

		if ( !needle[i] ) {
			return (char*)haystack;
		}
		haystack++;
	}

	return NULL;
}

char *CopyText( const char *string, size_t length ) {
	char *out = (char*)safe_malloc( length + 1 );
	memcpy( out, string, length );
	out[length] = '\0';
	return out;
}

void AppendText( char **string, size_t *length, size_t *bufferSize, const char *add, size_t addLength ) {
	if ( *length + addLength > *bufferSize ) {
		*string = (char*)safe_realloc( *string, *length + addLength + 1 );
		*bufferSize = *length + addLength + 1;
	}

	memcpy( &(*string)[*length], add, addLength );
	(*string)[*length + addLength] = '\0';

	*length += addLength;
}

char *ReadFile( const char *filename ) {
	FILE *f;

	f = fopen( filename, "rb" );
	if ( !f ) {
		fprintf( stderr, "warning: failed to read file '%s'\n", filename );
		return NULL;
	}

	fseek( f, 0, SEEK_END );
	long length = ftell( f );
	fseek( f, 0, SEEK_SET );

	char *contents = (char*)safe_malloc( length + 1 );
	if ( fread( contents, 1, length, f ) != (size_t)length ) {
		free( contents );
		fclose( f );
		fprintf( stderr, "warning: failed to read file '%s'\n", filename );
		return NULL;
	}
	contents[length] = '\0';
	fclose( f );

	//fprintf( stderr, "debug: read file '%s'\n%s\n", filename, contents );

	return contents;
}

strpair *StrPairFindKey( strpair *vars, char *key ) {
	if ( !vars ) {
		return NULL;
	}

	for ( strpair *var = vars; var->key; var++ ) {
		if ( strcasecmp( var->key, key ) == 0 ) {
			return var;
		}
	}

	return NULL;
}

void FreeStrPairs( strpair *vars ) {
	if ( !vars ) {
		return;
	}

	for ( strpair *var = vars; var->key; var++ ) {
		free( var->key );
		free( var->value );
	}

	free( vars );
}

void StrPairResolveValue( strpair *var, const char *basepath, sublist *sub ) {
	if ( !var || var->value[0] != '<' || var->valueReadFromFile ) {
		return;
	}

	var->valueReadFromFile = 1;

	char filename[PATH_MAX];
	snprintf( filename, sizeof( filename ), "%s/%s", basepath, &var->value[1] );
	filename[sizeof(filename)-1] = '\0';

	char *contents = ReadFile( filename );
	if ( contents ) {
		char *output;
		if ( sub ) {
			TemplateReplace( basepath, contents, sub, NULL, &output );
			free( contents );
		} else {
			output = contents;
		}
		free( var->value );
		var->value = output;
	}
}

strpair *StrPairsFromText( const char *contents ) {
	// Count the number of variables.
	const char *text = contents;
	int numVars = 0;
	while ( 1 ) {
		const char *start = text;
		const char *end = NULL;
		const char *split = NULL;
		if ( !*start ) {
			break;
		}
		for ( int i = 0; text[i] != '\0'; i++ ) {
			if ( text[i] == '=' ) {
				split = &text[i];
			}
			if ( text[i] == '\r' || text[i] == '\n' ) {
				end = &text[i];

				text = end + 1;
				break;
			}
		}
		if ( !split ) {
			continue;
		}

		numVars++;
	}

	// Allocate the variables.
	strpair *vars = (strpair *)safe_malloc( ( numVars + 1 ) * sizeof( strpair ) );
	numVars = 0;

	text = contents;
	while ( 1 ) {
		const char *start = text;
		const char *end = NULL;
		const char *split = NULL;
		if ( !*start ) {
			break;
		}
		for ( int i = 0; text[i] != '\0'; i++ ) {
			if ( text[i] == '=' ) {
				split = &text[i];
			}
			if ( text[i] == '\r' || text[i] == '\n' ) {
				end = &text[i];

				text = end + 1;
				break;
			}
		}
		if ( !split ) {
			continue;
		}

		// Replace existing value for key.
		int varIndex = -1;
		for ( int i = 0; i < numVars; i++ ) {
			if ( strncasecmp( vars[i].key, start, split - start ) == 0 ) {
				varIndex = i;
				break;
			}
		}

		if ( varIndex != -1 ) {
			// Replace key if it's a different case so debug messages make more sense.
			if ( strncmp( vars[varIndex].key, start, split - start ) != 0 ) {
				free( vars[varIndex].key );
				vars[varIndex].key = CopyText( start, split - start );
			}
			free( vars[varIndex].value );
		} else {
			varIndex = numVars;
			numVars++;

			vars[varIndex].key = CopyText( start, split - start );
		}

		vars[varIndex].value = CopyText( split+1, end - (split+1) );
		vars[varIndex].valueReadFromFile = 0;

		//fprintf( stderr, "var %d: key='%s', value='%s'\n", varIndex, vars[varIndex].key, vars[varIndex].value );
	}

	vars[numVars].key = NULL;
	vars[numVars].value = NULL;
	vars[numVars].valueReadFromFile = 0;

	return vars;
}

strpair *StrPairsFromFile( const char *filename ) {
	char *contents = ReadFile( filename );
	strpair *vars = StrPairsFromText( contents );
	free( contents );
	return vars;
}

void TemplateReplace( const char *basepath, const char *template_, sublist *sub1, sublist *sub2, char **output ) {
	const char *text = template_;
	size_t outputLength = 0;
	size_t outputBufferSize = 0;

	*output = NULL;

	while ( 1 ) {
		char *tmpl_start = my_strcasestr( text, "<tmpl_var" );
		if ( !tmpl_start ) {
			break;
		}
		char *tmpl_end = strchr( tmpl_start+1, '>' );
		if ( !tmpl_end ) {
			break;
		}
		tmpl_end++;

		char *name_start = my_strcasestr( tmpl_start + 1, "name=" );
		if ( !name_start ) {
			fprintf( stderr, "warning: TMPL_VAR without name\n" );
			// Skip <TMPL_VAR> in the output text.
			text = tmpl_end;
			continue;
		}
		name_start += strlen( "name=" );
		int quote = 0;
		if ( name_start[0] == '\'' ) {
			name_start++;
			quote = 1;
		}
		if ( name_start[0] == '\"' ) {
			name_start++;
			quote = 2;
		}

		char *name_end = NULL;
		if ( quote == 2 ) {
			name_end = strchr( name_start, '\"' );
		} else if ( quote == 1 ) {
			name_end = strchr( name_start, '\'' );
		} else {
			name_end = strchr( name_start, ' ' );
		}
		if ( !name_end || name_end >= tmpl_end ) {
			// Missing end quote.
			name_end = tmpl_end - 1;
			// <TMPL_VAR NAME=apple/>
			if ( name_end[0] == '/' ) {
				name_end--;
			}
		}

		char *key = CopyText( name_start, name_end - name_start );
		strpair *var = NULL;
		sublist *templatesub = NULL;
		if ( !var && sub1 ) {
			var = StrPairFindKey( sub1->vars, key );
			if ( var ) {
				templatesub = sub1->templatesub;
			}
		}
		if ( !var && sub2 ) {
			var = StrPairFindKey( sub2->vars, key );
			if ( var ) {
				templatesub = sub2->templatesub;
			}
		}

		if ( !var ) {
			fprintf( stderr, "warning: no defined value for TMPL_VAR name '%s'\n", key );

			//AppendText( output, &outputLength, &outputBufferSize, text, tmpl_end - text );
			// Skip <TMPL_VAR> in the output text.
			AppendText( output, &outputLength, &outputBufferSize, text, tmpl_start - text );
		} else {
			StrPairResolveValue( var, basepath, templatesub );

			//fprintf( stderr, "debug: replaced '%.*s' with '%s' (key '%s')\n", tmpl_end - tmpl_start, tmpl_start, value, key );

			AppendText( output, &outputLength, &outputBufferSize, text, tmpl_start - text );
			if ( var ) {
				AppendText( output, &outputLength, &outputBufferSize, var->value, strlen( var->value ) );
			}
		}

		text = tmpl_end;

		free( key );
	}

	AppendText( output, &outputLength, &outputBufferSize, text, strlen( text ) );
}

int main( int argc, char **argv ) {
	time_t sec = time( NULL );
	struct tm localtm;
	memcpy( &localtm, localtime( &sec ), sizeof( struct tm ) );

	// Mimic the output of `LC_ALL=C date` on Linux.
	// Sat Aug 1 08:56:58 PDT 2009
	// (Though on Windows it uses non-abbreviated timezone name.)
	const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	const char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	char timestamp[128];
	tzset();
	snprintf( timestamp, sizeof( timestamp ), "%s %s %d %d:%02d:%02d %s %d",
			days[localtm.tm_wday], months[localtm.tm_mon],
			localtm.tm_mday, localtm.tm_hour, localtm.tm_min, localtm.tm_sec,
			tzname[localtm.tm_isdst], localtm.tm_year + 1900 );
	timestamp[sizeof(timestamp)-1] = '\0';

	int date_year = localtm.tm_year + 1900;

	char commonpage[1024];
	snprintf( commonpage, sizeof( commonpage ),
		"SECLEFT=<secleft.htm\n"
		"SECRIGHT=<secright.htm\n"
		"SECEND=<secend.htm\n"
		"TIMESTAMP=%s\n"
		"DATE_YEAR=%d\n",
		timestamp, date_year );
	commonpage[sizeof(commonpage)-1] = '\0';

	if ( argc < 2 ) {
		fprintf( stdout,
			"Usage: hpagemake <filename.page>\n"
			"\n"
			"hpagemake reads a file containing key value pairs:\n"
			"\n"
			"PAGE_NAME=The Page Name\n"
			"PAGE_CONTENT=<filename.htm (starting with < reads content from file)\n"
			"\n"
			"and then substitutes the values in template.htm in the same directory:\n"
			"\n"
			"<html>\n"
			"<head><title><TMPL_VAR name=\"PAGE_NAME\"/></title></head>\n"
			"<body><TMPL_VAR name=\"PAGE_CONTENT\"/></body>\n"
			"</html>\n"
			"\n"
			"and print it to standard out.\n"
			"\n"
			"The following built-in variables exist which can be used in both the\n"
			"template and files referenced by the .page file:\n"
			"\n"
			"%s", commonpage );

		return EXIT_SUCCESS;
	}

	for ( int i = 1; i < argc; i++ ) {
		char *argv_i_copy = CopyText( argv[i], strlen( argv[i] ) );
		char *basepath = dirname( argv_i_copy );

		sublist commonsub;
		commonsub.vars = StrPairsFromText( commonpage );
		commonsub.templatesub = &commonsub;

#ifndef LAZY_LOAD
		if ( commonsub.vars ) {
			for ( strpair *var = commonsub.vars; var->key; var++ ) {
				StrPairResolveValue( var, basepath, commonsub.templatesub );
			}
		}
#endif

		sublist pagesub;
		pagesub.vars = StrPairsFromFile( argv[i] );
		pagesub.templatesub = &commonsub;

#ifndef LAZY_LOAD
		if ( pagesub.vars ) {
			for ( strpair *var = pagesub.vars; var->key; var++ ) {
				StrPairResolveValue( var, basepath, pagesub.templatesub );
			}
		}
#endif

		char filename[PATH_MAX];
		snprintf( filename, sizeof( filename ), "%s/%s", basepath, "template.htm" );
		filename[sizeof(filename)-1] = '\0';

		char *contents = ReadFile( filename );
		if ( contents ) {
			char *output;

			TemplateReplace( basepath, contents, &pagesub, &commonsub, &output );
			fprintf( stdout, "%s", output );

			free( output );
			free( contents );
		}

		FreeStrPairs( pagesub.vars );
		FreeStrPairs( commonsub.vars );
		free( argv_i_copy );
	}

	return EXIT_SUCCESS;
}
