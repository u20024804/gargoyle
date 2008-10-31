/*
 * Copyright © 2008 by Eric Bishop <eric@gargoyle-router.com>
 * 
 * This work ‘as-is’ we provide.
 * No warranty, express or implied.
 * We’ve done our best,
 * to debug and test.
 * Liability for damages denied.
 *
 * Permission is granted hereby,
 * to copy, share, and modify.
 * Use as is fit,
 * free or for profit.
 * On this notice these rights rely.
 *
 *
 *
 *  Note that unlike other portions of Gargoyle this code
 *  does not fall under the GPL, but the rather whimsical
 *  'Poetic License' above.
 *
 *  Basically, this library contains a bunch of utilities
 *  that I find useful.  I'm sure other libraries exist
 *  that are just as good or better, but I like these tools 
 *  because I personally wrote them, so I know their quirks.
 *  (i.e. I know where the bodies are buried).  I want to 
 *  make sure that I can re-use these utilities for whatever
 *  code I may want to write in the future be it
 *  proprietary or open-source, so I've put them under
 *  a very, very permissive license.
 *
 *  If you find this code useful, use it.  If not, don't.
 *  I really don't care.
 *
 */

#include "erics_tools.h"

char* replace_prefix(char* original, char* old_prefix, char* new_prefix)
{
	char* replaced = NULL;
	if(original != NULL && old_prefix != NULL && new_prefix != NULL && strstr(original, old_prefix) == original)
	{
		int old_prefix_length = strlen(old_prefix);
		int new_prefix_length = strlen(new_prefix);
		int remainder_length = strlen(original) - old_prefix_length;
		int new_length = new_prefix_length + remainder_length;
		/* printf("%d %d %d %d\n", old_prefix_length, new_prefix_length, remainder_length, new_length); */
		
		replaced = malloc(new_length+1);
		memcpy(replaced, new_prefix, new_prefix_length);
		memcpy(replaced+new_prefix_length, original+old_prefix_length, remainder_length);
		replaced[new_prefix_length+remainder_length] = '\0';
	}
	return replaced;
}

char* trim_flanking_whitespace(char* str)
{
	int new_start = 0;
	int new_length = 0;

	char whitespace[5] = { ' ', '\t', '\n', '\r', '\0' };
	int num_whitespace_chars = 4;
	
	
	int index = 0;
	int is_whitespace = 1;
	int test;
	while( (test = str[index]) != '\0' && is_whitespace == 1)
	{
		int whitespace_index;
		is_whitespace = 0;
		for(whitespace_index = 0; whitespace_index < num_whitespace_chars && is_whitespace == 0; whitespace_index++)
		{
			is_whitespace = test == whitespace[whitespace_index] ? 1 : 0;
		}
		index = is_whitespace == 1 ? index+1 : index;
	}
	new_start = index;


	index = strlen(str) - 1;
	is_whitespace = 1;
	while( index >= new_start && is_whitespace == 1)
	{
		int whitespace_index;
		is_whitespace = 0;
		for(whitespace_index = 0; whitespace_index < num_whitespace_chars && is_whitespace == 0; whitespace_index++)
		{
			is_whitespace = str[index] == whitespace[whitespace_index] ? 1 : 0;
		}
		index = is_whitespace == 1 ? index-1 : index;
	}
	new_length = str[new_start] == '\0' ? 0 : index + 1 - new_start;
	

	if(new_start > 0)
	{
		for(index = 0; index < new_length; index++)
		{
			str[index] = str[index+new_start];
		}
	}
	str[new_length] = 0;
	return str;
}

/* note: str element in return value is dynamically allocated, need to free */
dyn_read_t dynamic_read(FILE* open_file, char* terminators, int num_terminators)
{
	fpos_t start_pos;
	int size_to_read = 0;
	int terminator_found = 0;
	int terminator;
	char* str;
	dyn_read_t ret_value;

	fgetpos(open_file, &start_pos);
	
	while(terminator_found == 0)
	{
		int nextch = fgetc(open_file);
		int terminator_index = 0;
		for(terminator_index = 0; terminator_index < num_terminators && terminator_found == 0; terminator_index++)
		{
			terminator_found = nextch == terminators[terminator_index] ? 1 : 0;
			terminator = nextch;
		}
		terminator_found = nextch == EOF ? 1 : terminator_found;
		terminator = nextch == EOF ? EOF : nextch;
		if(terminator_found == 0)
		{
			size_to_read++;
		}
	}

	str = (char*)malloc((size_to_read+1)*sizeof(char));
	if(size_to_read > 0)
	{
		int i;
		fsetpos(open_file, &start_pos);
		for(i=0; i<size_to_read; i++)
		{
			str[i] = (char)fgetc(open_file);
		}
		fgetc(open_file); /* read the terminator */
	}
	str[size_to_read] = '\0';
	
	
	
	ret_value.str = str;
	ret_value.terminator = terminator;


	return ret_value;
}

char* read_entire_file(FILE* in, int read_block_size)
{
	int max_read_size = read_block_size;
	char* read_string = (char*)malloc(max_read_size+1);
	int bytes_read = 0;
	int end_found = 0;
	while(end_found == 0)
	{
		int nextch = '?';
		while(nextch != EOF && bytes_read < max_read_size)
		{
			nextch = fgetc(in);
			if(nextch != EOF)
			{
				read_string[bytes_read] = nextch;
				bytes_read++;
			}
		}
		read_string[bytes_read] = '\0';
		end_found = (nextch == EOF) ? 1 : 0;
		if(end_found == 0)
		{
			char *new_str = (char*)malloc(max_read_size+1);
			max_read_size = max_read_size + read_block_size;
			strcpy(new_str, read_string);
			free(read_string);
			read_string = new_str;
		}
	}
	return read_string;
}



int safe_strcmp(const char* str1, const char* str2)
{
	if(str1 == NULL && str2 == NULL)
	{
		return 0;
	}
	else if(str1 == NULL && str2 != NULL)
	{
		return 1;
	}
	else if(str1 != NULL && str2 == NULL)
	{
		return -1;
	}
	return strcmp(str1, str2);
}

/*
 * line is the line to be parsed -- it is not modified in any way
 * max_pieces indicates number of pieces to return, if negative this is determined dynamically
 * include_remainder_at_max indicates whether the last piece, when max pieces are reached, 
 * 	should be what it would normally be (0) or the entire remainder of the line (1)
 * 	if max_pieces < 0 this parameter is ignored
 *
 *
 * returns all non-separator pieces in a line
 * result is dynamically allocated, MUST be freed after call-- even if 
 * line is empty (you still get a valid char** pointer to to a NULL char*)
 */
char** split_on_separators(char* line, char* separators, int num_separators, int max_pieces, int include_remainder_at_max)
{
	char** split;

	if(line != NULL)
	{
		int split_index;
		int non_separator_found;
		char* dup_line;
		char* start;

		if(max_pieces < 0)
		{
			/* count number of separator characters in line -- this count + 1 is an upperbound on number of pieces */
			int separator_count = 0;
			int line_index;
			for(line_index = 0; line[line_index] != '\0'; line_index++)
			{
				int sep_index;
				int found = 0;
				for(sep_index =0; found == 0 && sep_index < num_separators; sep_index++)
				{
					found = separators[sep_index] == line[line_index] ? 1 : 0;
				}
				separator_count = separator_count+ found;
			}
			max_pieces = separator_count + 1;
		}
		split = (char**)malloc((1+max_pieces)*sizeof(char*));
		split_index = 0;
		split[split_index] = NULL;


		dup_line = strdup(line);
		start = dup_line;
		non_separator_found = 0;
		while(non_separator_found == 0)
		{
			int matches = 0;
			int sep_index;
			for(sep_index =0; sep_index < num_separators; sep_index++)
			{
				matches = matches == 1 || separators[sep_index] == start[0] ? 1 : 0;
			}
			non_separator_found = matches==0 || start[0] == '\0' ? 1 : 0;
			if(non_separator_found == 0)
			{
				start++;
			}
		}

		while(start[0] != '\0' && split_index < max_pieces)
		{
			/* find first separator index */
			int first_separator_index = 0;
			int separator_found = 0;
			while(	separator_found == 0 )
			{
				int sep_index;
				for(sep_index =0; separator_found == 0 && sep_index < num_separators; sep_index++)
				{
					separator_found = separators[sep_index] == start[first_separator_index] || start[first_separator_index] == '\0' ? 1 : 0;
				}
				if(separator_found == 0)
				{
					first_separator_index++;
				}
			}
			
			/* copy next piece to split array */
			if(first_separator_index > 0)
			{
				char* next_piece = NULL;
				if(split_index +1 < max_pieces || include_remainder_at_max <= 0)
				{
					next_piece = (char*)malloc((first_separator_index+1)*sizeof(char));
					memcpy(next_piece, start, first_separator_index);
					next_piece[first_separator_index] = '\0';
				}
				else
				{
					next_piece = strdup(start);
				}
				split[split_index] = next_piece;
				split[split_index+1] = NULL;
				split_index++;
			}


			/* find next non-separator index, indicating start of next piece */
			start = start+ first_separator_index;
			non_separator_found = 0;
			while(non_separator_found == 0)
			{
				int matches = 0;
				int sep_index;
				for(sep_index =0; sep_index < num_separators; sep_index++)
				{
					matches = matches == 1 || separators[sep_index] == start[0] ? 1 : 0;
				}
				non_separator_found = matches==0 || start[0] == '\0' ? 1 : 0;
				if(non_separator_found == 0)
				{
					start++;
				}
			}
		}
		free(dup_line);
	}
	else
	{
		split = (char**)malloc((1)*sizeof(char*));
		split[0] = NULL;
	}
	return split;
}

void to_lowercase(char* str)
{
	int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		str[i] = tolower(str[i]);
	}
}
void to_uppercase(char* str)
{
	int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		str[i] = toupper(str[i]);
	}
}

char* dynamic_strcat(int num_strs, ...)
{
	
	va_list strs;
	int new_length = 0;
	int i;
	int next_start;
	char* new_str;
		
	va_start(strs, num_strs);
	for(i=0; i < num_strs; i++)
	{
		char* next_arg = va_arg(strs, char*);
		if(next_arg != NULL)
		{
			new_length = new_length + strlen(next_arg);
		}
	}
	va_end(strs);
	
	new_str = malloc((1+new_length)*sizeof(char));
	va_start(strs, num_strs);
	next_start = 0;
	for(i=0; i < num_strs; i++)
	{
		char* next_arg = va_arg(strs, char*);
		if(next_arg != NULL)
		{
			int next_length = strlen(next_arg);
			memcpy(new_str+next_start,next_arg, next_length);
			next_start = next_start+next_length;
		}
	}
	new_str[next_start] = '\0';
	
	return new_str;
}
