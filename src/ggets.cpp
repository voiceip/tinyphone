/* File ggets.c  - goodgets is a safe alternative to gets */
/* By C.B. Falconer.  Public domain 2002-06-22            */
/*    attribution appreciated.                            */

/* Revised 2002-06-26  New prototype.
           2002-06-27  Incomplete final lines
			  2006-03-29  This version "templatised" for 
								Unicode and made to use "new/delete"
								by David Pritchard
 */

/* fggets and ggets [which is fggets(ln, stdin)] set *ln to
   a buffer filled with the next complete line from the text
   stream f.  The storage has been allocated within fggets,
   and is normally reduced to be an exact fit.  The trailing
   \n has been removed, so the resultant line is ready for
   dumping with puts.  The buffer will be as large as is
   required to hold the complete line.

   Note: this means a final file line without a \n terminator
   has an effective \n appended, as EOF occurs within the read.

   If no error occurs fggets returns 0.  If an EOF occurs on
   the input file, EOF is returned.  For memory allocation
   errors some positive value is returned.  In this case *ln
   may point to a partial line.  For other errors memory is
   freed and *ln is set to NULL.

   Freeing of assigned storage is the callers responsibility
 */
#include "stdafx.h"

#include <stdio.h>
#include <string.h>  /* strchr */
#include <stdlib.h>
//#include "ggets.h"


// Adapted from code by Ana Sayfa. Thanks also to Dave Kondrad.
template<class TTCHAR>
	TTCHAR *ReallocString(TTCHAR *ptr, size_t newsize, size_t oldsize, IN OUT bool& bOwnBuffer) /*size_t unsigned int*/ 
{
	// Only reallocate if needed
	if (newsize > oldsize)
	{
		TTCHAR* pTemp = NULL;

		// Declare the new string and zero it out
		pTemp = new TTCHAR[newsize] ;
		memset(pTemp, '\0', newsize*sizeof(TTCHAR));

		// Copy the old string if there is one
		if (ptr)
		{
			// Ensure we only copy up to the size of the new buffer
			memcpy(pTemp, ptr, min(oldsize*sizeof(TTCHAR), newsize*sizeof(TTCHAR)));

			// Don't delete a default buffer we didn't create
			if (bOwnBuffer)
			{
				if(oldsize > 1)
				{
					delete [] ptr;
				}
				else
				{
					delete ptr;
				}
			}
		}

		// We own it now!
		bOwnBuffer = true;

		return pTemp; 
	}

	return ptr;
} 

// Encapsulate multibyte and Unicode versions of fgets in functions with same
// name. Use this function in our template fggets function and when TTCHAR is
// resolved the correct one will be called
char *GetFileString( char *string, int n, FILE *stream )
{
	return fgets( string, n, stream );
}

wchar_t *GetFileString( wchar_t *string, int n, FILE *stream )
{
	return fgetws( string, n, stream );
}

// Same with strchr
char *FindCharInString( const char *string, int c )
{	
	// This *should* work OK even with UTF-8, since we're only searching for
	// '\n' in practice. Careful if you try to use this for anything other
	// than ASCII characters. 
	return (char*)_mbschr( (const unsigned char *)string, c );
}

wchar_t *FindCharInString( const wchar_t *string, wint_t c )
{	
	return (wchar_t *)wcschr( string, c );
}


#define INITSIZE   112  /* power of 2 minus 16, helps malloc */
#define DELTASIZE (INITSIZE + 16)

// DP 29/03/2006 Modified to "templatise" it so it will work with Unicode and MB
// DP 24/06/2006 Now accepts default buffer to avoid the need for dynamic memory allocation
template<class TTCHAR>
	int fggets(TTCHAR* *ln, FILE *f, IN OUT bool& bOwnBuffer, IN const int nDefaultBufferSizeChars)
//	int fggets(CTemplateSmartPtrArray<TTCHAR>& spln, FILE *f, const int nDefaultBufferSizeChars)
{
   int		cursize, rdsize, nOldSize;
   TTCHAR	*buffer, *temp, *rdpoint, *crlocn;
	
	bOwnBuffer = false;

	ASSERT(*ln != NULL || nDefaultBufferSizeChars==0);

	// DP 24/06/2007: Allow default buffer to be passed in for reading. Avoids need for "new"s/"delete"s in 99% of cases
	if (nDefaultBufferSizeChars == 0 || *ln == NULL)
	{
		// DP 29/3/2006: Changed to use new
		buffer = new TTCHAR[INITSIZE];
		if (NULL == buffer)
			return FGGETS_NOMEM;

		// DP 24/06/2007: Signal that we own the buffer now
		bOwnBuffer = true;
	}
	else
	{
		buffer = *ln;
	}

   *ln = NULL; /* default */
//   if (NULL == (buffer = (char*)malloc(INITSIZE)))

	// DP 29/3/2006: Added oldsize variable
	// Initialise original size to default buffer size or INITSIZE, whichever applies
	cursize = rdsize = nOldSize = (nDefaultBufferSizeChars==0 ? INITSIZE:nDefaultBufferSizeChars);
   rdpoint = buffer;
   *buffer = '\0';

//   if (NULL == fgets(rdpoint, rdsize, f)) 
   if (NULL == GetFileString(rdpoint, rdsize, f)) 
	{
//      free(buffer);
		// DP 29/3/2006: Changed to use delete
		if (bOwnBuffer)
		{
			delete buffer;
		}
      return EOF;
   }

   /* initial read succeeded, now decide about expansion */
//   while (NULL == (crlocn = strchr(rdpoint, '\n'))) 
   while (NULL == (crlocn = FindCharInString(rdpoint, '\n'))) 
	{
      /* line is not completed, expand */

		// DP 29/3/2006: Save current buffer size
		nOldSize = cursize;

      /* set up cursize, rdpoint and rdsize, expand buffer */
      rdsize = DELTASIZE + 1;   /* allow for a final '\0' */
      cursize += DELTASIZE;
//      if (NULL == (temp = (char*)realloc(buffer, (size_t)cursize))) {

		// DP 29/3/2006: Changed to use custom realloc
      if (NULL == (temp = ReallocString<TTCHAR>(buffer, (size_t)cursize, (size_t)nOldSize, bOwnBuffer))) 
		{
         /* ran out of memory */
         *ln = buffer; /* partial line, next call may fail */
         return FGGETS_NOMEM;
      }
      buffer = temp;
      /* Read into the '\0' up */
      rdpoint = buffer + (cursize - DELTASIZE - 1);

      /* get the next piece of this line */
//      if (NULL == fgets(rdpoint, rdsize, f)) {
      if (NULL == GetFileString(rdpoint, rdsize, f)) 
		{
         /* early EOF encountered */
//         crlocn = strchr(buffer, '\0');
         crlocn = FindCharInString(buffer, '\0');
         break;
      }
   } /* while line not complete */

   *crlocn = '\0';  /* mark line end, strip \n */
   rdsize = (int)(crlocn - buffer);

//   if (NULL == (temp = (char*)realloc(buffer, (size_t)rdsize + 1))) {

	// DP 29/3/2006: Changed to use custom realloc
   if (NULL == (temp = ReallocString<TTCHAR>(buffer, (size_t)rdsize + 1, (size_t)cursize, bOwnBuffer))) 
	{
      *ln = buffer;  /* without reducing it */
      return FGGETS_OK;
   }
   *ln = temp;
   return FGGETS_OK;
} /* fggets */
/* End of ggets.c */
