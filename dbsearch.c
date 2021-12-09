#include <stdio.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include "arxiv.h"

int main(int argc, char *argv[]){
	search_single_word("quadratic");
	return 0;
}
