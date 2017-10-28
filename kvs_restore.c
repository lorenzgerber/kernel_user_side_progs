/*
 * kvs_restore.c
 *
 *  Created on: Oct 28, 2017
 *      Author: knig
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct keyvalue_pair{
	int key;
	char data[];
};

int main(int argc, char* argv[]){

	char c;
	char* keystring = calloc(0,sizeof(char)*255);
	char* data = calloc(0,sizeof(char)*1024);
	struct keyvalue_pair* pair = malloc(sizeof(struct keyvalue_pair)+sizeof(char)*1024);
	char testdata[31] = "3\0stures data\05\0uwe bolls data";

	FILE *fp = fopen("keystore.backup", "ab+");
	if (fp == NULL) {
		printf("Problem opening file");
		return EXIT_FAILURE;
	}
	//fill with testdata
	for(int i = 0;i<30;i++){
		int results = fputc(testdata[i], fp);
		if (results == EOF) {
			// Failed to write do error code here.
		}
	}
	// Read contents from file
	rewind(fp);
	c = fgetc(fp);
	int i = 0;
	while (c != EOF){
		while(c != '\0'){
			keystring[i] = c;
			c = fgetc(fp);
			i++;
		}
		keystring[i] = '\0';
		c = fgetc(fp);
		i=0;
		while(c != '\0'){
			data[i] = c;
			c = fgetc(fp);
			i++;
		}
		data[i] = '\0';
		i=0;
		pair->key = atoi(keystring);
		strcpy(pair->data, data);
		printf ("\n%d %s", pair->key, pair->data);
		c = fgetc(fp);
	}
	free(pair);
	free(keystring);
	free(data);
	fclose(fp);
	return 0;
}
