/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

/**
 * @file wfa_miscs.c
 * @brief File containing the miscellaneous utility rountines
*/

#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"

#include <sys/timeb.h> /* For prototype of "_ftime()" */

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

/** 
 * Checks whether str is a string
 * @param str string to be checked
 * @return 0 (i.e., true) if indeed str is a string. 1 (i.e., false) otherwise.
*/
int isString(char *str)
{
	if(*str == '\0')
	{
		return WFA_FAILURE;
	}
	if((str[0] >= 'a' && str[0] <= 'z') 
		|| (str[0] > 'A' && str[0] < 'Z'))
	{
		return WFA_SUCCESS;
	}
	else
	{
		return WFA_FAILURE;
	}
}

/** 
 * Checks whether str is a number
 * @param str string to be checked
 * @return 0 (i.e., true) if indeed str is a number. 1 (i.e., false) otherwise.
*/
int isNumber(char *str)
{
	if(*str == '\0')
	{
		return WFA_FAILURE;
	}
	if (str[0] >= '0' && str[0] <= '9')
	{
		return WFA_SUCCESS;
	}
	else
	{
		return WFA_FAILURE;
	}
}

/** 
 * Checks whether str is in IPv4 format
 * @param str string to be checked
 * @return 0 (i.e., true) if indeed str is in IPv4. 1 (i.e., false) otherwise.
*/
int isIpV4Addr(char *str)
{
	int dots = 0;
	char *tmpstr = str;

	if(*str == '\0')
	{
		return WFA_FAILURE;
	}

	while(*tmpstr != '\0')
	{
		if(*tmpstr == '.')
		{
			dots++;
		}
		tmpstr++;
	}

	if(dots <3)
	{
		return WFA_FAILURE;
	}
	else
	{
		return WFA_SUCCESS;
	}
}

/** 
 * Convert a timeval variable to a double
 * @param tval timeval structure variable to be converted
 * @return A number representing the tval member values in double.
*/
double wfa_timeval2double(struct timeval *tval)
{
	return ((double) tval->tv_sec + (double) tval->tv_usec*1e-6);
}

/** 
 * Convert a double to a timeval variable
 * @param tval A timeval variable whose members are converted froma a double
 * @param dval A double number to be converted
*/
void wfa_double2timeval(struct timeval *tval, double dval)
{
	tval->tv_sec = (long int) dval;
	tval->tv_usec = (long int) ((dval - tval->tv_sec) * 1000000);
}

/** 
 * Compute the time difference
 * @param str string to be checked
 * @return 0 (i.e., true) if indeed str is in IPv4. 1 (i.e., false) otherwise.
*/
double wfa_ftime_diff(struct timeval *t1, struct timeval *t2)
{
	double dtime;

	dtime = wfa_timeval2double(t2) - wfa_timeval2double(t1);
	return dtime ;
}

/** 
 * Checks whether str is in IPv4 format
 * @param str string to be checked
 * @return 0 (i.e., true) if indeed str is in IPv4. 1 (i.e., false) otherwise.
*/
int wfa_itime_diff(struct timeval *t1, struct timeval *t2)
{
	int dtime = 0;
	int sec = t2->tv_sec - t1->tv_sec;
	int usec = t2->tv_usec - t1->tv_usec;

	if  (sec < 0)
	{
		return 0;
	}
	else if ( (sec == 0))
	{

		if ( usec >= 0)
			return usec;
		else
		{
			return 0;
		}

	}
	if(usec < 0)
	{
		sec -=1;
		usec += 1000000;
	}

	dtime = sec*1000000 + usec;
	return dtime;
}

/** 
 * Convert integer in Big Endian and store it in a buffer
 * @param val A Big Endian integer to be converted
 * @param buffer A buffer used to store converted integer
*/
void int2BuffBigEndian(int val, char *buf)
{
	char *littleEn = (char *)&val;

	buf[0] = littleEn[3];
	buf[1] = littleEn[2];
	buf[2] = littleEn[1];
	buf[3] = littleEn[0];
}

/** 
 * Convert buffer content to integer in Big Endian
 * @param buff A buffer contains integer elements
 * @return An integer in Big Endian format
*/
int bigEndianBuff2Int(char *buff)
{
	int val;
	char *strval = (char *)&val;

	strval[0] = buff[3];
	strval[1] = buff[2];
	strval[2] = buff[1];
	strval[3] = buff[0];

	return val;
}

/** 
 * Print a traffic stream's profile info
 * @param pf A tgProfile_t structure storing the stream profile information
*/
void printProfile(tgProfile_t *pf)
{
	DPRINT_INFO(WFA_OUT, "profile type %i direction %i Dest ipAddr %s Dest port %i So ipAddr %s So port %i rate %i duration %i pksize %i\n", pf->profile, pf->direction, pf->dipaddr, pf->dport, pf->sipaddr, pf->sport, pf->rate, pf->duration, pf->pksize);
}

/** 
 * Get the date and time
 * @param curTimeP Store the current time, expressed as seconds and microseconds since the Epoch.
 * @param tz If tzp is not a null pointer, the behavior is unspecified.
 * @return 0 and no value shall be reserved to indicate an error.
*/
int gettimeofday(struct timeval *curTimeP, void *tz)
{
	struct _timeb  localTime;

	if (curTimeP == (struct timeval *) 0)
	{
		errno = EFAULT;
		return (-1);
	}

	/*
	*  Compute the elapsed time since Jan 1, 1970 by first
	*  obtaining the elapsed time from the system using the
	*  _ftime(..) call and then convert to the "timeval"
	*  equivalent.
	*/

	_ftime(&localTime);

	curTimeP->tv_sec  = (long) (localTime.time + localTime.timezone);
	curTimeP->tv_usec = (long) (localTime.millitm * 1000);

	return(0);

}

/** 
 * Estimate machine's timer latency
 * @return Timer's latency in sec.
*/
int wfa_estimate_timer_latency()
{
	struct timeval t1, t2, tp2;
	int sleep=20; /* two miniseconds */
	int latency =0;

	gettimeofday(&t1, NULL);

	Sleep(sleep);

	gettimeofday(&t2, NULL); 

	tp2.tv_usec = t1.tv_usec + 20000;
	if( tp2.tv_usec >= 1000000)
	{
		tp2.tv_sec = t1.tv_sec +1;
		tp2.tv_usec -= 1000000;
	}
	else
		tp2.tv_sec = t1.tv_sec;

	DPRINT_INFOL(WFA_OUT, "before sec %i, usec %i sleep %i after sec %i usec %i\n",t1.tv_sec,t1.tv_usec,t2.tv_sec,t2.tv_usec);

	return latency = (t2.tv_sec - tp2.tv_sec) * 1000000 + (t2.tv_usec - tp2.tv_usec); 
}

/** 
 * Split string into tokens
 * @param string Point to a null-terminated string.
 * @param sepset Point to a null-terminated string of separator characters.
 * @param lasts Point to a char * variable that is used internally by strtok_r().
 * @return A pointer to the token found, or a null pointer when no token is found.
*/
char * strtok_r(char *string, const char *sepset, char **lasts)
{
	char    *q, *r;

	/* first or subsequent call */
	if (string == NULL)
		string = *lasts;

	if (string == NULL)             /* return if no tokens remaining */
		return (NULL);

	q = string + strspn(string, sepset);    /* skip leading separators */

	if (*q == '\0')         /* return if no tokens remaining */
		return (NULL);

	if ((r = strpbrk(q, sepset)) == NULL)   /* move past token */
		*lasts = NULL;  /* indicate this is last token */
	else 
	{
		*r = '\0';
		*lasts = r + 1;
	}

	return (q);
}

/** 
 * Compare the two strings s1 and s2, ignoring the case of the characters
 * @param s1 First string to be compared
 * @param s2 Second string to be compared
 * @return An integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found to be less than, to match, or be greater than s2, respectively
*/
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	while(n > 0
		&& toupper((unsigned char)*s1) == toupper((unsigned char)*s2))
	{
		if(*s1 == '\0')
			return 0;
		s1++;
		s2++;
		n--;
	}
	if(n == 0)
		return 0;

	return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

/** 
 * Compare the two strings s1 and s2
 * @param s1 First string to be compared
 * @param s2 Second string to be compared
 * @return An integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found to be less than, to match, or be greater than s2, respectively
*/
int strcasecmp(const char *s1, const char *s2)
{
	while(toupper((unsigned char)*s1) == toupper((unsigned char)*s2)) 
	{
		if(*s1 == '\0')
			return 0;
		s1++;
		s2++;
	}

	return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}
