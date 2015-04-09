#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

int main(int argc, char * argv[])
{
	if (argc != 4) {
		printf("Usage: %s <out basename> <file count> <hex string>\n", argv[0]);
		return EXIT_FAILURE;
	}

	long longval;
	char *endptr = NULL;

	std::string out_basename(argv[1]);
	std::string hexstring(argv[3]);

	longval = strtol(argv[2], &endptr, 10);
	if (*endptr != '\0' || errno == ERANGE) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[2]);
		return EXIT_FAILURE;
	}
	int out_filecount = (int)longval;

	if (hexstring.length() % 2 != 0) {
		fprintf(stderr, "Error: Hex string length error\n");
		return EXIT_FAILURE;
	}

	int seqnum_offset = -1;
	int seqnum_size = 0;
	bool seqnum_scan = false;
	std::vector<uint8_t> hex(hexstring.length() / 2);
	for (size_t offset = 0; offset < hex.size(); offset++) {
		std::string bytestr = hexstring.substr(offset * 2, 2);

		if (strcasecmp(bytestr.c_str(), "NN") == 0) {
			if (seqnum_offset == -1) {
				seqnum_offset = offset;
				seqnum_size = 1;
				seqnum_scan = true;
			}
			else {
				if (!seqnum_scan) {
					fprintf(stderr, "Error: Multiple number field\n");
					return EXIT_FAILURE;
				}

				seqnum_size++;
			}
		}
		else {
			seqnum_scan = false;

			longval = strtol(bytestr.c_str(), &endptr, 16);
			if (*endptr != '\0' || errno == ERANGE) {
				fprintf(stderr, "Error: Number format error \"%s\"\n", bytestr.c_str());
				return EXIT_FAILURE;
			}

			hex[offset] = (uint8_t)longval;
		}
	}

	for (int file_no = 0; file_no < out_filecount; file_no++) {
		for (int offset = 0; offset < seqnum_size; offset++) {
			hex[seqnum_offset + offset] = (file_no >> (8 * offset)) & 0xff;
		}

		char out_path[1024];
		sprintf(out_path, "%s-%04d.bin", out_basename.c_str(), file_no);

		FILE * fp = fopen(out_path, "wb");
		if (fp == NULL) {
			fprintf(stderr, "Error: File open error \"%s\"\n", out_path);
			return EXIT_FAILURE;
		}

		if (fwrite(&hex[0], 1, hex.size(), fp) != hex.size()) {
			fprintf(stderr, "Error: File write error \"%s\"\n", out_path);
			fclose(fp);
			return EXIT_FAILURE;
		}

		fclose(fp);

		printf("Created %s\n", out_path);
	}

	return EXIT_SUCCESS;
}
