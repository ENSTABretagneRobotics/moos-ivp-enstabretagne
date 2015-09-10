/***************************************************************************************************************:')

CONFUtils.c

CONF files to store programs parameters.

Fabrice Le Bars

Created : 2009-06-28

Version status : Not finished

***************************************************************************************************************:)*/
#include "CONFUtils.h"

/*
Reads the value at a specified key in a CONF file. The memory for the value read is allocated.

char** value : (OUT) value read
char* key : (IN) key to find
FILE* fp : (IN) file to read

Returns : EXIT_SUCCESS if the key specified was found or EXIT_FAILURE otherwise
*/
int fread_value_conf(char** value, char* key, FILE* fp)	{

	char* buf = NULL;
	char* keybuf = NULL;

	if (fp == NULL)	{
		fprintf(stderr, "Invalid file\n");
		return EXIT_FAILURE;
	}

	buf = (char*)calloc(256, sizeof(char));
	keybuf = (char*)calloc(256+1, sizeof(char));

	sprintf(keybuf, "%s=", key); // Puts a '=' after the key name

	rewind(fp); // Resets the file pointer at the beginning
	while (fgets(buf, 256, fp) != NULL)	{ // Reads a line in the file

		sscanf(buf, "%[^\n\r]", buf); // Deletes the newline character copied by fgets
		if (strncmp(buf, keybuf, min(256-1, strlen(keybuf))) == 0)	{ // Compares the beginning of the line read with key and check if it is followed immediately by a '='
			*value = (char*)calloc(strlen(buf)-strlen(keybuf)+1, sizeof(char));
			memcpy(*value, buf+strlen(keybuf), strlen(buf)-strlen(keybuf)); // Copies what follows the '='
			free(buf);buf = NULL;
			free(keybuf);keybuf = NULL;
			return EXIT_SUCCESS;
		}
		memset(buf, 0, 256);
	}

	free(buf);buf = NULL;
	free(keybuf);keybuf = NULL;
	return EXIT_FAILURE;
}

/*
Reads the value at a specified key in a CONF file. The memory for the value read is allocated.

char* filename : (IN) file to read
char** value : (OUT) value read
char* key : (IN) key to find

Returns : EXIT_SUCCESS if the key specified was found or EXIT_FAILURE otherwise
*/
int LoadFromCONFFile(char* filename, char** value, char* key)	{

	int ret = 0;

	FILE* fp = fopen(filename, "r");
	if (fp == NULL)	{
		fprintf(stderr, "Error opening the file\n");
		return EXIT_FAILURE;
	}

	ret = fread_value_conf(value, key, fp);

	fclose(fp);fp = NULL;

	return ret;
}

