#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "info_fetch.h"

const char default_location[] = "/usr/local/bin";

//This function is responsible for pulling off address for the requested folder from an environment variable's contents. 
char* fetch_addr(char *file_name, char *env_var_name)
{
	char *default_help_addr = malloc( (1+strlen(file_name)+strlen(default_location))*sizeof(char) );
	
	//If there's not enough memory to even store the help file address then, the help file reading cannot take place
	if( default_help_addr == NULL )
	{
		return NULL;
	}

	//First try scanning the default help file location.
	sprintf(default_help_addr, "%s/%s", default_location, file_name);
	if( access(default_help_addr, F_OK) == 0 )
	{
		return default_help_addr;
	}
	else
	{
		//If the help file is not available at the default location then scan the addresses held in PATH environment variable for obtaining the help file
		char *env_var_data = getenv(env_var_name); //obtain data present in the environment variable, whose name is present in 'env_var_name'

		if(env_var_data == NULL)
		{	
			//If the help file is available at the default location then move on to the help file scanning procedure
			printf("[error] There's no variable named as '%s' in the environment.\n", env_var_name);
			exit(EXIT_FAILURE);
		}	
		
		//Below is the hardcoded logic for scanning the data of PATH environment variable
		int i = 0, j = 0, max_addr_len = 50;
		int addr_start_ind = 0, addr_end_ind = 0, colon_count = 0;
		char *addr = malloc(max_addr_len*sizeof(char));
		
		//Scan the obtained data
		while( env_var_data[i] != '\0' )
		{
			//If the scanned character is not a colon, then it's a part of an address
			if(env_var_data[i] != ':')
			{
				addr[j++] = env_var_data[i];
				
				if( j == max_addr_len )
				{
					//If the allocated space is less, then perform reallocation
					max_addr_len += 10;
					addr = realloc(addr, max_addr_len);
					if(addr == NULL)
						return NULL;
				}
			}
			else
			{
				addr[j] = '\0'; //end the 'addr' string by placing the end of string marker
				//Check if the address stored in 'addr' matches the 'default_location'.
				if( strcmp(default_location, addr) == 0 )
				{
					//If they do match, then immediately skip over to next address
					j = 0;
					continue;
				}
				//If they don't match, then move on with the further procedures
				
				//Check if the memory location pointed to by 'addr' is capable enough of holding the concatenated string
				int new_len = j+strlen(file_name)+2;
				if( new_len > max_addr_len )
				{
					//If it ain't capable enough, then allocate required space to it	
					addr = realloc(addr, new_len);
					if(addr == NULL)
						return NULL;
					else
						max_addr_len = new_len;
				}
				
				//As soon as the end of an address, i.e. ':' is found, concatenate the address with the input 'file_name' to form the complete address
				addr[j] = '/'; //Place the forward slash at the end of 'addr' by overwriting the end of string marker i.e. '\0' placed above. This will allow the address held in 'addr' to become almost ready for concatenation with the 'file_name'. i.e. convert the address string held in 'addr' from 'address' to 'address/'
				addr[++j] = '\0'; //Why almost ready? Since the '/' charachter has overwritten the prior end of string marker i.e. '\0' at the end of the 'addr' string, therefore a new end of string marker has to be placed after '\' charachter so that the address string held in 'addr' becomes completely ready for concatenation.
				
				//Perform the concatenation
				addr = strcat(addr, file_name); 
				//Now check if the requested executable file exists at the address held in 'addr'
				if( access(addr, F_OK) == 0 )
				{
					//If the file exists, then return the address to that file
					return addr;
				}
				else
				{
					//If the file doesn't exist, then reset the index of 'addr' to 0, in order to move on to the next address
					j = 0;
				}
			}
				
			i++;
		}
		
		return NULL;
	}
}

int fetch_help(char *topic)
{
	//Fetch the address of the help file 'help_doc' from the 'PATH' environment variable
	char *help_file_name = "help_doc";
	char *file_addr = fetch_addr(help_file_name, "PATH");
	
	if(file_addr == NULL)
	{
		exit(EXIT_FAILURE);
	}

	FILE *read_ptr = fopen(file_addr, "r");
	int return_value = 0;
	
	if(read_ptr == NULL)
	{
		return_value = 1;
	}	
	else
	{
		char *line = malloc(MAXLINE*sizeof(char));
		char *chr_ptr = NULL;
		int word_count = 0;
		/*
		malloc() allocates requested amt. of memory and hands over
		the address of the allocated memory to 'line' pointer.
		This allows the fgets() fn. below to easily fetch a line
		from the file it reads(via tha 'read_ptr') and copy it 
		to the memory location allocated to the 'line' pointer.
		This ultimately helps to access the contents present in the
		file (pointed to by 'read_ptr') in a line by line fashion.
		*/
			
		if( strlen(topic) == 0 )
		{
			//display how to use help
			printf("[info] Let's say that you wish to know about one of the following topics:\n\n");
			while(fgets(line, MAXLINE, read_ptr) != NULL)
			{
				if( line[0] == '>' || line[0] == '|' )
				{
					//Replace the end of line character, i.e. '\n' from the topic names with the end of string charachter i.e. '\0', so that it becomes easier to display the topic names
					chr_ptr = strchr(line, '\n'); 
					if( chr_ptr != NULL )
						*chr_ptr = '\0';
						
					printf("%s ", line+1);
					word_count++;
				}
			}
			printf("\n\n[info] Then all you have to do is enter the following command :\n");
			printf("[....] $ compute -h topic_name\n");
			printf("[info] However, if you wish to see the entirety of the help document, then simply enter 'all' as your 'topic_name'\n");
				
		}
		else if( strcmp(topic, "all") == 0 )
		{
			while(fgets(line, MAXLINE, read_ptr) != NULL)
				if( line[0] != '>' && line[0] != '|' )
					printf("%s", line);
		}
		else
		{
			while( fgets(line, MAXLINE, read_ptr) != NULL )
			{
				//Check if the scanned line's first charachter is a topic marker.
				//Note that in the 'help_doc' file, topic markers are placed over every topic.
				//These topic markers are '>' and '|' charachters
				if( line[0] == '>' || line[0] == '|' )
				{	
					char start_marker = line[0]; //Record the marker 
					//Recording the marker charachter will help us to know when to stop the reading and displaying of lines from the 'help_doc'
					
					//Match the requested topic against the topic names present in the help doc
					if( strstr(line, topic) != NULL )
					{
						//If a match is found then keep displaying the successive lines until the end of file is reached OR until a marker charachter which is same as that of 'start_marker' is found
						while( fgets(line, MAXLINE, read_ptr) != NULL )
						{	
							if(line[0] == start_marker)
								break;
							else
							{
								//Check if the line doesn't start with a topic marker.
								if(line[0] != '>' && line[0] != '|')
									printf("%s", line);
								//If it doesn't start with a topic marker then display the line, otherwise move on to the next line
							}
						}
						break;
					}
				}
			}
		}
					
		free(line); //Don't forget to free the memory allocated to 'line'
		free(file_addr); //Don't forget to free the memory allocated to 'file_addr'
		fclose(read_ptr);//Close the file iff the above mentioned file existed. Otherwise, closing a file which didn't exist, will lead to segmentation fault.
	}
		
	return return_value;	
}

int fetch_version()
{
	printf("[info] %s version %s\n", PROGRAM_NAME, VERSION);
	return 0;
}
