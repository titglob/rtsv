#include <lib.h>

/**
 * check for space character
 */
static int char_isspace(char c)
{
   return ((c == ' ') 
        || (c == '\n')
        || (c == '\t'));
}

/**
 * \brief return the next option as a string, and return the pointer on the next char (can point on '\0')
 *
 * Internaly, find the first non space character in \a args.
 * Then extract the string by removing all ESCAPE characters, until we find either a non escaped space character, or the end of the string
 *
 * the option is memset-ed to zero before starting the algorithm
 * if max limit is reached, the algorithm continue but no more characters are copied to the option string.
 * max is supposed to be the maximum number of characters that we can copy in \a option
 *
 * if no options was found, string_len(option) egals 0
 *
 * \return NULL if no more charaters follows, or a pointer on next space character.
 */
char * gopt_next(char * args, char * option, int max)
{
   int i = 0;
   int esc = 0;

   if((option == NULL) || (args == NULL))
      return 0;

   mem_set(option, 0, max);

   // find the first non 'space' character
   while((*args != 0) && char_isspace(*args))
      args++;

   // no option found
   if(*args == 0)
      return NULL;

   // special case where the first non space char is ESCAPE
   if(*args == ESCAPE)
      esc = 1;

   while((*args != 0) && (!char_isspace(*args) || esc))
   {
      if(*args == ESCAPE)
      {
         esc = 1;
         args++;
         continue;
      }
      esc = 0;
      if(i < max)
         option[i++] = *args;

      args++;
   }

   return args;
}

/*
 * convert a table argv argc of parameters into a single string (may contain ESCAPE characters).
 *
 * max is supposed to be the maximum number of characters that we can set in \a args string
 * special case of cmd line options like > program "-a 1 -b 2" -v gives argc = 3 and argv=
 * argv[0] = 'program'
 * argv[1] = '-a 1 -b 2'
 * argv[2] = '-v'
 *
 * is then formated with gopt_string and gives args="program\n-a\033 1\033 -b\033 2\n-v" so that when
 * calling the reverse method we retreive the smae arguments
 */

int gopt_format(int argc, char ** argv, char * args, int max)
{
   int i, j, k;
   j=0; // args current length
   mem_set(args, 0, max);
   for(i=0; i<argc; i++)
   {
      // separated formatted options a '\n'
      if(i > 0)
      {  
         if(j > max) 
            return -1;
         args[j++] = '\n';
      }

      /*
       * Ex: option -z '-a1 -b 2' or -z "-a1 -b 2", operating system gives us the string '-a1 -b 2' in the same argv entry. 
       * We need to insert escaped sequence to retreive this behaviour at the end
       */
      for(k=0; k<string_len(argv[i]); k++)
      {
         if(j > max)
            return -1;

         if(char_isspace(argv[i][k]))
         {
            // insert escape sequence
            args[j++] = ESCAPE;
            if(j > max)
               return -1;
         }
         args[j++] = argv[i][k];
      }
   }
   return j;
}


/**
 * \brief convert a string into a table of argv[] argc strings.
 *
 * this function asserts that a table of maxArgs strings of max_argv length has been allocated by the user
 * this function do the opposite of gopt_format
 *
 * \param[in] args the string to convert into argc+argv
 * \param[out] argv an array of max \a macArgc string pointer
 * \param[int] max_argc size of argv array
 * \param[int] max_argv size of all argv[*] strings
 * \return argc, the real number of options found
 */
int gopt_extract(char * args, char * argv, int max_argc, int max_argv)
{
   int argc = 0;
   int len;

   while(args && (argc < max_argc))
   {
      args = gopt_next(args, argv, max_argv);
      len = string_len(argv);
      if(len > 0)
         argc++;
      argv += len + 1;
   }
   return argc;
}

char * gopt_find(const char * var_name, char * args, int max_len)
{
   char option[GETOPT_CFG_MAX_OPTION_LENGTH];

   while(args)
   {
      args = gopt_next(args, option, GETOPT_CFG_MAX_OPTION_LENGTH);
      if(string_ncmp(option, var_name, max_len) == 0)
         break;
   }
   return args;
}


char * gopt_string(char * var_val, const char * var_name, char * args, int max_len)
{
   args = gopt_find(var_name, args, GETOPT_CFG_MAX_OPTION_LENGTH);

   // option is not found, don't modify the var_val default value
   if(args == NULL)
      return NULL;

   // the next string is supposed to be the outputed value
   return gopt_next(args, var_val, max_len);
}

char * gopt_bool(int * var_val, const char * var_name, char * args)
{
   args = gopt_find(var_name, args, GETOPT_CFG_MAX_OPTION_LENGTH);

   // option is not found, don't modify the var_val default value
   if(args == NULL)
      return NULL;

   *var_val = 1;

   return args;
}

char * gopt_long(long int * var_val, const char * var_name, char * args)
{
   char number[GETOPT_CFG_MAX_INTEGER_LENGTH];
   long int v;

   args = gopt_string(number, var_name, args, GETOPT_CFG_MAX_INTEGER_LENGTH);

   // option is not found, don't modify the var_val default value
   if(args == NULL)
      return NULL;

   // modify the output value really if necessary
   if(string_tol(number, &v) == 0)
      *var_val = v;

   return args;
}


char * gopt_integer(int * var_val, const char * var_name, char * args)
{
   char * p;
   long int val = (long int)*var_val;;
   p = gopt_long(&val, var_name, args);
   *var_val = (int)val;
   return p;
}

int gopt_basename(const char *path, char *file)
{
   int cpt, cpt2;
   int last=0;
   for (cpt=0; cpt<string_len(path); cpt++)
      if (path[cpt]=='/')
         last=cpt;

   cpt2=0;
   if(last > 0)
      last++; // skip '/'
   for (cpt=last; cpt<string_len(path)+1; cpt++)
   {
      file[cpt2]=path[cpt];
      cpt2++;
   }
   return 0;
}

