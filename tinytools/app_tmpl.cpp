
#define NOMINMAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <string>
#include <vector>

#if defined(WIN32) || defined(_MSC_VER)
#include <windows.h>
#include <sys/stat.h>
#ifndef PATH_MAX
#define PATH_MAX	_MAX_PATH
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static void writeInt(uint8_t * buf, uint32_t value)
{
	buf[0] = value & 0xff;
	buf[1] = (value >> 8) & 0xff;
	buf[2] = (value >> 16) & 0xff;
	buf[3] = (value >> 24) & 0xff;
}

static void usage(const char * progname)
{
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s [filename]`\n", progname);
	printf("\n");

	printf("### Options\n");
	printf("\n");
	printf("`--help`\n");
	printf("  : Show help\n");
	printf("\n");
}

bool modify_data(std::vector<uint8_t> & data)
{
	//data.push_back('/');
	//data.push_back('/');
	//data.push_back('t');
	//data.push_back('e');
	//data.push_back('s');
	//data.push_back('t');
	return true;
}

int main(int argc, char *argv[])
{
	FILE * fp;

	if (argc == 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	int argi = 1;
	while (argi < argc && argv[argi][0] == '-') {
		if (strcmp(argv[argi], "--help") == 0) {
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		else {
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	int argnum = argc - argi;
	if (argnum != 1) {
		fprintf(stderr, "Error: Too few/many arguments\n");
		return EXIT_FAILURE;
	}

	std::string filename(argv[argi]);

	struct stat st;
	if (stat(filename.c_str(), &st) != 0 || st.st_size == -1) {
		fprintf(stderr, "Error: File size error\n");
		return EXIT_FAILURE;
	}
	size_t filesize = st.st_size;

	std::vector<uint8_t> data;

	fp = fopen(filename.c_str(), "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: File open error\n");
		return EXIT_FAILURE;
	}

	data.resize(filesize);
	if (fread(&data[0], 1, filesize, fp) != filesize) {
		fprintf(stderr, "Error: File read error\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	fclose(fp);

	if (!modify_data(data)) {
		fprintf(stderr, "Error: Something went wrong\n");
		return EXIT_FAILURE;
	}

	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		fprintf(stderr, "Error: File open error\n");
		return EXIT_FAILURE;
	}

	if (fwrite(&data[0], 1, data.size(), fp) != data.size()) {
		fprintf(stderr, "Error: File write error\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	fclose(fp);

	return EXIT_SUCCESS;
}
