/**
  * GreenPois0n Absinthe - file.c
  * Copyright (C) 2010 Chronic-Dev Team
  * Copyright (C) 2010 Joshua Hill
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

#define BUFSIZE 4096

file_t* file_create() {
	file_t* file = (file_t*) malloc(sizeof(file_t));
	if (file) {
		memset(file, '\0', sizeof(file_t));
	}
	return file;
}

file_t* file_open(const char* path) {
	size_t got = 0;
	unsigned char buffer[4096];

	file_t* file = file_create();
	if (file) {
		file->desc = fopen(path, "rb");
		if (file->desc == NULL) {
			fprintf(stderr, "Unable to open file %s\n", path);
			return NULL;
		}

		file->path = strdup(path);
		if (file->path == NULL) {
			fprintf(stderr, "Unable to allocate memory for file path\n");
			file_free(file);
			return NULL;
		}
		file->offset = 0;

		fseek(file->desc, 0, SEEK_END);
		file->size = ftell(file->desc);
		fseek(file->desc, 0, SEEK_SET);

		file->data = (unsigned char*) malloc(file->size);
		if (file->data == NULL) {
			fprintf(stderr, "Unable to allocate memory for file\n");
			file_free(file);
			return NULL;
		}

		while (file->offset < file->size) {
			memset(buffer, '\0', BUFSIZE);
			got = fread(buffer, 1, BUFSIZE, file->desc);
			if (got > 0) {
				file->offset += got;
				memcpy(&file->data[file->offset], buffer, got);
			} else {
				break;
			}
		}
		fprintf(stderr, "Read in %llu of %llu bytes from %s\n", file->offset, file->size, file->path);
		// We have the data stored in memory now, so we don't need to keep this open anymore
		//fseek(file->desc, 0, SEEK_SET);
		file_close(file);
		file->offset = 0;
	}
	return file;
}

void file_close(file_t* file) {
	if (file) {
		if (file->desc) {
			fclose(file->desc);
			file->desc = NULL;
		}
	}
}

void file_free(file_t* file) {
	if (file) {
		if (file->desc) {
			file_close(file);
			file->desc = NULL;
		}
		if (file->path) {
			free(file->path);
			file->path = NULL;
		}
		if(file->data) {
			free(file->data);
			file->data = NULL;
		}
		file->size = 0;
		file->offset = 0;
		free(file);
	}
}

int file_read(const char* file, unsigned char** buf, unsigned int* length) {
	FILE* fd = NULL;
	fd = fopen(file, "r+");
	if(fd == NULL) {
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	long size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	unsigned char* data = malloc(size);

	int bytes = fread(data, 1, size, fd);
	if(bytes != size) {
		fclose(fd);
		return -1;
	}
	fclose(fd);

	*buf = data;
	*length = bytes;
	return bytes;
}

int file_write(const char* file, unsigned char* buf, unsigned int length) {
	FILE* fd = NULL;
	fd = fopen(file, "w+");
	if(fd == NULL) {
		return -1;
	}

	int bytes = fwrite(buf, 1, length, fd);
	if(bytes != length) {
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return bytes;
}
