#ifndef ALOOK_H
#define ALOOK_H

/* Similar to the function strncmp form the stdio.h library. Compares
 * the first |numChars| characters of |str1| and |str2|, ignoring any 
 * characters that are not alphanumeric.
 *
 * Returns 0 if |str1| = |str2|, >0 if |str1| > |str2|, 
 * and <0 if |str1| < |str2|.
 */
int StrnDictCmp(const char *str1, const char *str2, size_t numChars)

/* Similar to the function strncasecmp form the stdio.h library. Compares
 * the first |numChars| characters of |str1| and |str2|, ignoring case and any 
 * characters that are not alphanumeric. 
 *
 * Returns 0 if |str1| = |str2|, >0 if |str1| > |str2|, 
 * and <0 if |str1| < |str2|.
 */
int StrnDictCaseCmp(const char *str1, const char *str2, size_t numChars)

/* Called in BinarySearch, FindFirstOccurrence finds the first line in the
 * mmapped-array |file| that begins with |search| and returns its position. 
 * FindFirstOccurrence expects |pos| to be an index of |file| that points to 
 * the start of a line beginning with |search|.
 *
 * The function pointed to by |comp| will be a function to compare two strings,
 * which varies depending on whether case or non-alphanumeric characters should
 * be ignored.
 */
int FindFirstOccurrence(char *file, int pos, char *search, Options opts,
 int (*comp)(const char *, const char *, size_t))

/* Recursive binary search function to find a line in |file| (an mmapped
 * array of characters) that begins with the string |search|. Once a single
 * line starting with |search| has been found, BinarySearch calls 
 * FindFirstOccurrence to find the first line starting with this string.
 *
 * The function pointed to by |comp| will be a function to compare two strings,
 * and it will vary depending on whether case or non-alphanumeric characters
 * should be ignored.
 *
 * BinarySearch will eventually return the position of the line in |file|
 * containing the first occurrence of |search|. If no line is found,
 * the program exits with exit code 1.
 */
int BinarySearch(char *file, char *search, int start, int end, Options opts,
 int (*comp)(const char *, const char *, size_t))

/* Prints all lines in |file| that begin with the string |search|. |file| is 
 * expected to point to the first occurrence in an mmapped array of a line
 * that begins with |search|.
 *
 * The function pointed to by comp will be a function to compare two strings,
 * which differs based on the command line arguments specified when running
 * the program.
 */
void PrintAllLines(char *file, char *search, Options opts,
 int (*comp)(const char *, const char *, size_t))

/* Used when the -t command line argument is specified. GetTermString returns
 * a new string to be used as the search string. This new string will be the 
 * string |search|, truncated after the first occurrence of |termChar|.
 */
char *GetTermString(char *search, char termChar)

/* Used if the -d flag is specified. Returns a new string to be used as the 
 * search string. This new string will be |search|, stripped of all 
 * non-alphanumeric characters.
 */
char *GetDictString(char *search)

/* Used to set flags for the specified command line arguments. Accepted flags
 * are -d, -f, and -t. The -t flag also requires an additional character to be
 * specified. If the command line arguments were improperly formed, the 
 * function prints a usage message and exits with an exit code of 2. Otherwise,
 * an Options struct with values set indicating which flags were specified is
 * returned.
 */
Options GetArgvOpts(int argc, char *argv[])
#endif
