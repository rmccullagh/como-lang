#include <object.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define OBJECT_MAP_INITIAL_SIZE 16U
#define OBJECT_SUCCESS 1

static int object_map_delete(Object *o, const char *key)
{
	Map *map = O_MVAL(o);
	size_t len = strlen(key);
	uint32_t h = stringHash(key, len);
	uint32_t index = h % map->capacity;
	Bucket *be = map->buckets[index];
	Bucket *prev = NULL;

	while(be) {
		if(len == be->key->length && 
			!memcmp(be->key->value, key, be->key->length)) {
			if(prev) {
				prev->next = be->next;
			} else {
				map->buckets[index] = be->next;
			}
			free(be->key->value);
			free(be->key);
			objectDestroy(be->value);
			free(be);
			map->size--;
			return 1;
		}
		prev = be;
		be = be->next;
	}

	return 0;
}


static void object_print_stats(Object *o)
{
	Map *map = O_MVAL(o);

	fprintf(stdout, "Map stats:\n");
	fprintf(stdout, " table capacity: %u\n", map->capacity);
	fprintf(stdout, " slots used: %u\n", map->size);
}

static uint32_t _object_map_hash_key(Map *map, const char *key,
	const size_t keylen) {
	uint32_t h = stringHash(key, keylen);
	return h % map->capacity;
}

static int object_map_add(Map *map, const char *key, 
	const size_t keylen, Object *value)
{
	uint32_t index = _object_map_hash_key(map, key, keylen);
	Bucket *b = map->buckets[index];
	

}

static Map *object_map_create() 
{
	Map *map = malloc(sizeof(*map));
	if(!map)
		return map;

	map->capacity = OBJECT_MAP_INITIAL_SIZE;
	map->size = 0U;
	map->buckets = malloc(map->capacity * sizeof(Bucket));

	if(!map->buckets) {
		map->capacity = 0;
		free(map);
		return NULL;
	}

	memset(map->buckets, 0, map->capacity * sizeof(Bucket));

	return map;
}

int main(void)
{

	Map *map = object_map_create();
	object_map_add(map, "name", sizeof("name") -1, newString("Ryan"));



/*
	FILE *fp = fopen("b.data", "wb");

	short a = SHRT_MAX;

	const unsigned char *data = (const unsigned char *)&a;
	size_t i;
	for(i = 0; i < sizeof(a); i++) {
		fwrite(data + i, 1, 1, fp);
	}

	fclose(fp);

	fp = fopen("b.data", "rb");

	unsigned char nbytes[2];
	size_t nread = fread((void *)nbytes, 2, 1, fp);

	printf("nread=%zu\n", nread);
	for(i = 0; i < 2; i++) 
		printf("nbytes[%zu]=%02X\n", i, (unsigned int)(*(nbytes + i)));

	fclose(fp);*/


	return 0;
}