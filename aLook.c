#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

typedef struct Options {
   int dFlag;
   int fFlag;
   int tFlag;
   char termChar;
} Options;

/* Compares the first |numChars| characters of two strings, ignoring
 * any non-alphanumeric characters. Works by first building two new strings
 * out of the strings passed in as arguments, stripping each of 
 * non-alphanumeric characters. Then, StrnDictCmp calls strncmp on the two
 * new strings.
 *
 * Like strncmp, returns a 0 if the first |numChars| characters are the same,
 * a value less than 0 if |str1| comes before |str2| in dictionary order,
 * or greater than 0 if it comes after |str2|. 
 */
int StrnDictCmp(const char *str1, const char *str2, size_t numChars) {
   char *str, *temp1;
   char *compStr, *temp2;   
   int charsCopied = 0;
   int retVal;

   temp1 = str = malloc(sizeof(char) * numChars);
   temp2 = compStr = malloc(sizeof(char) * numChars);
 
   while (*str1 && charsCopied < numChars) {
      if (isalnum(*str1)) {
         *temp1++ = *str1++;
         charsCopied++;
      }
      else {
         str1++;
      }
   }
   *temp1 = '\0';
   charsCopied = 0;
   while (*str2 && charsCopied < numChars) {
      if (isalnum(*str2)) {
         *temp2++ = *str2++;
         charsCopied++;
      }
      else {
         str2++;
      }
   }
   *temp2 = '\0';
 
   retVal = strncmp(str, compStr, numChars);
   
   free(str);
   free(compStr);

   return retVal;
}

/* Compares the first |numChars| characters of two strings, ignoring case and
 * any non-alphanumeric characters. Works by first building two new strings
 * out of the strings passed in as arguments, stripping each of 
 * non-alphanumeric characters. Then, StrnDictCmp calls strncasecmp on the two
 * new strings.
 *
 * Like strncasecmp, returns a 0 if the first |numChars| characters are the 
 * same, a value less than 0 if |str1| comes before |str2| in dictionary 
 * order, or greater than 0 if it comes after |str2|. 
 */
int StrnDictCaseCmp(const char *str1, const char *str2, size_t numChars) { 
   char *str, *temp1;
   char *compStr, *temp2;   
   int charsCopied = 0;
   int retVal;

   temp1 = str = malloc(sizeof(char) * numChars);
   temp2 = compStr = malloc(sizeof(char) * numChars);
 
   while (*str1 && charsCopied < numChars) {
      if (isalnum(*str1)) {
         *temp1++ = *str1++;
         charsCopied++;
      }
      else {
         str1++;
      }
   }
   *temp1 = '\0';
   charsCopied = 0;
   while (*str2 && charsCopied < numChars) {
      if (isalnum(*str2)) {
         *temp2++ = *str2++;
         charsCopied++;
      }
      else {
         str2++;
      }
   }
   *temp2 = '\0';
  
   retVal = strncasecmp(str, compStr, numChars);
   
   free(str);
   free(compStr);

   return retVal;
}

/* Finds the first occurrence of a line in |file| that begins with |search.
 * File will be an mmapped array of characters that contains the text of the
 * entire file aLook is searching. The argument |pos| is expected to be the 
 * index of the beginning of a line in file that begins with |search|. 
 * 
 * FindFirstOccurrence works by repeatedly decrementing |pos| until it points
 * to the beginning of a line (delimited by newline characters), then comparing
 * the start of the line to |search|. This process repeats until a line that
 * does not contain |search| is found, at which point the index of the last 
 * line starting with |search| is found.
 */
int FindFirstOccurrence(char *file, int pos, char *search, Options opts,
 int (*comp)(const char *, const char *, size_t)) {
   int firstPos = pos;
   
   while (*(file + --firstPos) != '\n') 
      ;
  
   while (firstPos > 0 && 
    (*comp)(file + ++firstPos, search, strlen(search)) == 0) {
      firstPos--;
      while (firstPos > 0 && *(file + --firstPos) != '\n')
         ;
   }
   
   if ((*comp)(file + firstPos, search, strlen(search)) != 0) {
      while (*(file + firstPos++) != '\n')
        ;
   }

   return firstPos;
}

/* Performs a recursive binary search on |file| to find a line beginning with 
 * |search|. BinarySearch compares the start of the line in the file nearest 
 * to the middle of the range specified (|start| - |end| / 2) to |search|. If 
 * they are the same, BinarySearch calls FindFirstOccurrence to find the first
 * line in the file beginning with |search|. If the line begins with a string 
 * greater than |search|, BinarySearch is called again, with |start| and |end| 
 * set such that it searches the first half of the file. Otherwise, it searches
 * the second half of the file.
 *
 * After the first line beginning with |search| has been found, BinarySearch
 * returns an index to the start of this line to its original caller.
 *
 * If BinarySearch fails to find a line beginning with |search|, it exits the
 * program with an exit code of 1.
 */
int BinarySearch(char *file, char *search, int start, int end, Options opts,
 int (*comp)(const char *, const char *, size_t)) {
   int middle;
   int charPos;
   int strOrder;
   int searchPos;

   if (start >= end) {
      exit(1);
   }
   
   middle = (start + end) / 2;   
    
   charPos = middle;
   while (*(file + charPos++) != '\n')
      ;
   strOrder = (*comp)(search, file + charPos, strlen(search));

   if (strOrder == 0) { 
      searchPos = FindFirstOccurrence(file, charPos, search, opts, comp);  
   }
   else if (strOrder < 0) {
      searchPos = BinarySearch(file, search, start, middle, opts, comp);  
   }
   else {
      searchPos = BinarySearch(file, search, middle + 1, end, opts, comp);
   }

   return searchPos; 
}

/* Prints all lines in |file| beginning with |search|. The parameter |file| is
 * expected to be a pointer to the first line in an mmapped array that begins
 * with |search|.
 *
 * PrintAllLines prints information line-by-line, continuing until it reaches
 * a line that does not start with |search|.
 */
void PrintAllLines(char *file, char *search, Options opts, 
 int (*comp)(const char *, const char *, size_t)) {
   char *tempChar = file;
   
   while ((*comp)(tempChar, search, strlen(search)) == 0) {
      while (*tempChar != '\n') {
         printf("%c", *tempChar++);
      }
      printf("%c", *tempChar++);
   }
}

/* Used when the -t flag is specified to return a truncated search string.
 * GetTermString builds a new string, copying characters from |search| until 
 * it reaches an instance of |termChar| or the end of the string. It then
 * returns a pointer to the beginning of this new string.
 */
char *GetTermString(char *search, char termChar) {
   char *newString; 
   char *temp;
   newString = temp = malloc(sizeof(char) * strlen(search));   

   while (*search && *search != termChar) {
      *temp++ = *search++;
   }
   if (*search) {
      *temp++ = *search;
   }
   *temp = '\0';

   return newString;
}

/* Used when the -d flag is specified to return a new search string.
 * GetDictString builds a new string, copying characters from |search| while
 * ignoring any non-alphanumeric characters. It then returns a pointer to
 * the beginning of this new string.
 */
char *GetDictString(char *search, Options opts) {
   char *str, *temp;
   char *searchTemp = search;
  
   temp = str = malloc(sizeof(char) * strlen(search));  
 
   while (*searchTemp) {
      if (isalnum(*searchTemp)) {
         *temp++ = *searchTemp++;
      }
      else {
         searchTemp++;
      }
   }
   *temp = '\0';

   return str;
}

/* Using the getOpts function, GetArgvOpts reads in command line arguments
 * to determine the flags specified while running the program. The accepted
 * flags are -d, -f, and -t. The -t flag also takes a required character
 * argument. If this character for the -t flag is not specified, or if any
 * flags other than the accepted ones are found, GetArgvOpts exits the program,
 * returning an exit code of 2.
 *
 * GetArgvOpts returns an Options struct, that includes values set indicating 
 * the flags specified, as well as the terminating character to be used with
 * the -t argument, if it was specified.
 */
Options GetArgvOpts(int argc, char *argv[]) {
   Options opts = {0, 0, 0, 0};
   int arg;
 
   if (argc < 2) {
      printf("usage: aLook [-df] [-t char] string [file]\n");
      exit(2); 
   }
    
   while ((arg = getopt(argc, argv, "dft:")) != -1) {
      switch (arg) {
      case 'd':
         opts.dFlag = 1;
         break;

      case 'f':
         opts.fFlag = 1;
         break;

      case 't':
         opts.tFlag = 1;
         opts.termChar = *optarg;
         break;

      case '?':
         if (optopt == 't') {
            printf("usage: aLook [-df] [-t char] string [file]\n");
            exit(2);
         }
         else {
            printf("aLook: No option -%c\n", optopt);
            exit(2);
         }
         break;

      default:
         assert(0);
      }
   }

   return opts;
}

int main(int argc, char *argv[]) {
   Options opts;
   char *searchString, *fileName, *fileMap;
   struct stat buf;
   int inFile, firstPos;
   int (*compFp)(const char *, const char *, size_t);
 
   opts = GetArgvOpts(argc, argv);
  
   if (optind == argc) {   
      printf("usage: aLook [-df] [-t char] string [file]\n");
      exit(2);
   }

   searchString = argv[optind];

   if (++optind < argc) {
      fileName = argv[optind];   
   }
   else {
      opts.dFlag = opts.fFlag = 1;
      fileName = "/usr/share/dict/words";
   }
 
   searchString = opts.tFlag ? GetTermString(searchString, opts.termChar) :
    searchString;
   
   searchString = opts.dFlag ? GetDictString(searchString, opts) 
    : searchString;

   if ((inFile = open(fileName, O_RDONLY)) == -1) {
      printf("aLook: file %s not found\n", fileName);
      exit(2);
   }
   
   fstat(inFile, &buf); 
 
   fileMap = (char *) mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, inFile, 0);
   
   if (fileMap == MAP_FAILED) {
      close(inFile);
      printf("aLook: Error mapping file\n");
      exit(2);
   }

   if (opts.dFlag && opts.fFlag) {
      compFp = StrnDictCaseCmp;
   }
   else if (opts.dFlag) {
      compFp = StrnDictCmp;
   }
   else if (opts.fFlag) {
      compFp = strncasecmp;
   }
   else {
      compFp = strncmp;
   }

   firstPos = BinarySearch(fileMap, searchString, 0, buf.st_size - 1, opts, 
    compFp);
   
   PrintAllLines(fileMap + firstPos, searchString, opts, compFp);

   munmap(fileMap, buf.st_size);

   return 0;
}
