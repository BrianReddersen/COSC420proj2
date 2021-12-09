#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>

int main(){
	FILE *fp;
	char *line = malloc(5000*sizeof(char));
	char index_buffer[50];
	int bufsiz;
	int nread;
	fp = fopen("arxiv-metadata.json", "r");
	
	mongoc_client_t *client;
	mongoc_collection_t *coll;
	bson_oid_t oid;
	bson_t *doc;
	const char *dbstr = "mongodb://localhost:27017";
	bson_error_t err;
	
	doc = bson_new();
	client = mongoc_client_new(dbstr);
	coll = mongoc_client_get_collection(client, "project2", "papers");
	if (!client) return -1;
// 	BSON_APPEND_OID(doc, "_id", &oid);
// 	BSON_APPEND_OID(doc, "testid2", &oid);
	
	int i = 0;
	
	while (fgets(line, 50, fp)){	//gets paper id and keeps looping til EOF
		if (i == 2){
// 			break;
		}
		bson_reinit(doc);			//empties and reinits the bson document
		bson_oid_init(&oid, NULL);	//inits object id
		BSON_APPEND_OID(doc, "_id", &oid);
		sprintf(index_buffer, "%d", i);
		BSON_APPEND_INT32(doc, "index", i);
		BSON_APPEND_UTF8(doc, "paper_id", line);
		fgets(line, 200, fp);		//gets paper title
		BSON_APPEND_UTF8(doc, "title", line);
		fgets(line, 300, fp);		//gets authors
		BSON_APPEND_UTF8(doc, "authors", line);
		BSON_APPEND_INT32(doc, "pagerank", 1);
		BSON_APPEND_INT32(doc, "hub", 1);
		BSON_APPEND_INT32(doc, "auth", 1);
		fgets(line, 5000, fp);		//gets abstract
		BSON_APPEND_UTF8(doc, "abstract", line);
		mongoc_collection_insert_one(coll, doc, NULL, NULL, &err);
		fgets(line, 100, fp);		//skips line of pluses
		i++;
	}
	
// 	if (!strcmp(line, "++++++\n")) printf("next doc\n");
	bson_destroy(doc);
	free(line);
	return 0;
}
