#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#if defined(_WIN32)

#include <fcntl.h>
#include <io.h>
#define FMODE_WRITEBIN "wb"
#define FMODE_READBIN "rb"
#if defined(_MSC_VER)
#define SETBINMODE(f) _setmode(_fileno(f), _O_BINARY)
#define ISATTY(f) _isatty(_fileno(f))
#else
#define SETBINMODE(f) setmode(fileno(f), _O_BINARY)
#define ISATTY(f) isatty(fileno(f))
#endif

#elif definde(DJGPP)

#include <fcntl.h>
#include <io.h>
#define FMODE_WRITEBIN "wb"
#define FMODE_READBIN "rb"
#define SETBINMODE(f) setmode(fileno(f), O_BINARY)
#define ISATTY(f) isatty(fileno(f))

#elif defined(TOWNS)

#include <fcntl.h>
#include <io.h>
#define FMODE_WRITEBIN "wb"
#define FMODE_READBIN "rb"
#define SETBINMODE(f) setmode(f, _BINARY)
#define ISATTY(f) isatty(fileno(f))

#else

#define FMODE_WRITEBIN "w"
#define FMODE_READBIN "r"
#define SETBINMODE(f)
#define ISATTY(f) 0

#endif

#ifdef _MSC_VER
#if _MSC_VER >= 1200
#pragma comment(linker, "/OPT:NOWIN98")
#endif
#endif
typedef enum {
	BC2_NO_ERROR = 0,
	BC2_ERROR_INPUT,
	BC2_ERROR_OUTPUT,
	BC2_ERROR_PARAM,
	BC2_ERROR_SHORT_OF_MEMORY
} BC2_ERROR;

/* static unsigned char buf[4096]; */

typedef struct PATCH_TAG {
	int addr;
	int data;
	struct PATCH_TAG *next;
} PATCH;

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

typedef struct {
	PATCH *patch_top;
	char inpath[MAX_PATH + 1];
	char outpath[MAX_PATH + 1];
	int help;
	int skip;
	int len;
	int unit;
	int num;
	int block;
	int zero;
} BC2_PARAM;

/* パッチ関係 */

static void free_patch(BC2_PARAM *param) {
	PATCH *cur, *next;
	for (cur = param->patch_top; cur; cur = next)
	{
		next = cur->next;
		free(cur);
	}
	param->patch_top = 0;
}

static BC2_ERROR alloc_patch(BC2_PARAM *param, int addr, int data)
{
	PATCH *cur = malloc(sizeof(PATCH));
	if (!cur) return BC2_ERROR_SHORT_OF_MEMORY;
	cur->addr = addr;
	cur->data = data;
	cur->next = param->patch_top;
	param->patch_top = cur;
	return BC2_NO_ERROR;
}

static void patch_patch(BC2_PARAM *param, int base, unsigned char *p, int size)
{
	int limit = base + size;
	PATCH *cur;
	for (cur = param->patch_top; cur; cur = cur->next)
	{
		if (base <= cur->addr && cur->addr < limit)
		{
			p[cur->addr - base] = cur->data;
		}
	}
}

/* パラメータ関係 */

static void reset_param(BC2_PARAM *param)
{
	/* helpは初期化しない */
	free_patch(param);
	param->skip = 0;
	param->len = 0;
	param->unit = 0;
	param->num = 0;
	param->block = 0;
	param->zero = 0;
	param->inpath[0] = '\0';
	param->outpath[0] = '\0';
}

static void init_param(BC2_PARAM *param)
{
	param->inpath[MAX_PATH] = '\0';
	param->outpath[MAX_PATH] = '\0';
	param->help = 1;
	param->patch_top = 0;
	reset_param(param);
}


static void term_param(BC2_PARAM *param)
{
	free_patch(param);
}

static char *hatoi2(char *p, int *pi)
{
	int c, r;
	while (*p == ' ' || *p == '\t') p++;
	for (r = 0; *p; p++)
	{
		c = *p;
		switch (c)
		{
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				c = c + 'A' - 'a';
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				c = c + '0' + 10 - 'A';
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				r = r * 16 + (c - '0');
				continue;
		}
		break;
	}
	*pi = r;
	return p;
}

static int hatoi(char *p)
{
	int ret;
	hatoi2(p, &ret);
	return ret;
}

#define LINE_MAX 4096
#define OPT_MAX 4096
typedef struct
{
	/* 無駄は多いが気にしない */
	enum {
		getopt_argcv,
		getopt_file
	} mode;
	/* argcv */
	int argp;
	int argc;
	char **argv;
	/* file */
	FILE *fp;
	char optbuf[OPT_MAX + 1];
	char linebuf[LINE_MAX + 1];
	char *linep;
} GETOPT;

char *GetOptNext(GETOPT *args)
{
	switch (args->mode)
	{
		case getopt_argcv:
			if (args->argp < args->argc)
			{
				return args->argv[args->argp++];
			}
			return 0;
		case getopt_file:
			if (!args->fp) return 0;
			if (*args->linep == '\0' || *args->linep == '\n' || *args->linep == '\r')
			{
				if (!fgets(args->linebuf, LINE_MAX, args->fp)) return 0;
				args->linep = args->linebuf;
				while (*args->linep == ' ' || *args->linep == '\t') args->linep++;
			}
			if (*args->linep == '\"')
			{
				int i = 0;
				*args->linep++;
				while (*args->linep != '\"' && *args->linep != '\0' && *args->linep != '\n' && *args->linep != '\r' && i < OPT_MAX) args->optbuf[i++] = *args->linep++;
				args->optbuf[i++] = '\0';
				if (*args->linep == '\"') args->linep++;
			}
			else
			{
				int i = 0;
				while (*args->linep != ' ' && *args->linep != '\t' && *args->linep != '\0' && *args->linep != '\n' && *args->linep != '\r' && i < OPT_MAX) args->optbuf[i++] = *args->linep++;
				args->optbuf[i++] = '\0';
			}
			while (*args->linep == ' ' || *args->linep == '\t') args->linep++;
			return args->optbuf[0] ? args->optbuf : 0;
	}
	return 0;
}

BC2_ERROR GetOptInit(GETOPT *args, int argc, char **argv)
{
	args->mode = getopt_argcv;
	args->argp = 1;
	args->argc = argc;
	args->argv = argv;
	return BC2_NO_ERROR;
}

BC2_ERROR GetOptTerm(GETOPT *args)
{
	switch (args->mode)
	{
		case getopt_argcv:
			break;
		case getopt_file:
			if (args->fp) fclose(args->fp);
			break;
	}
	return BC2_NO_ERROR;
}

BC2_ERROR GetOptInitFile(GETOPT *args, char *path)
{
	if (!path || !path[0]) return BC2_ERROR_INPUT;
	args->mode = getopt_file;
	args->fp = fopen(path, "r");
	args->linebuf[0] = '\0';
	args->linep = args->linebuf;
	return args->fp ? BC2_NO_ERROR : BC2_ERROR_INPUT;
}



BC2_ERROR bincut2main(BC2_PARAM *param)
{
	char buf[4096];
	FILE *ifp, *ofp;
	int base = 0, size, uskip = 0;
	if (param->inpath[0])
	{
		ifp = fopen(param->inpath, FMODE_READBIN);
		if(!ifp) return BC2_ERROR_INPUT;
	}
	else
	{
		ifp = stdin;
		if (ISATTY(ifp)) return BC2_ERROR_INPUT;
		SETBINMODE(ifp);
	}
	fseek(ifp, 0, SEEK_END);
	size = ftell(ifp);
	if (param->unit)
	{
		uskip = param->unit * param->block + (size % param->unit);
		size -= uskip;
	}
	size -= param->skip;
	if (size  < 1)
	{
		if (param->inpath[0]) fclose(ifp);
		return BC2_ERROR_INPUT;
	}
	if (param->outpath[0])
	{
		ofp = fopen(param->outpath, FMODE_WRITEBIN);
		if (!ofp)
		{
			if (param->inpath[0]) fclose(ifp);
			return BC2_ERROR_OUTPUT;
		}
	}
	else
	{
		ofp = stdout;
		SETBINMODE(ofp);
	}
	fseek(ifp, param->skip + uskip, SEEK_SET);
	if (param->len && size > param->len) size = param->len;
	if (param->num * param->unit && size > param->num * param->unit)
		size = param->num * param->unit;
	while (size > sizeof(buf))
	{
		fread(buf, 1, sizeof(buf), ifp);
		patch_patch(param, base, buf, sizeof(buf));
		fwrite(buf, 1, sizeof(buf), ofp);
		size -= sizeof(buf);
		base += sizeof(buf);
	}
	fread(buf, 1, size, ifp);
	patch_patch(param, base, buf, size);
	fwrite(buf, 1, size, ofp);
	if (param->zero)
	{
		memset(buf, 0, sizeof(buf));
		for (size = param->zero; size > sizeof(buf); size -= sizeof(buf))
		{
			fwrite(buf, 1, sizeof(buf), ofp);
		}
		fwrite(buf, 1, size, ofp);
	}
	if (param->outpath[0]) fclose(ofp);
	if (param->inpath[0]) fclose(ifp);
	return BC2_NO_ERROR;
}

BC2_ERROR bc2_error(BC2_PARAM *param, BC2_ERROR code, char *para)
{
	switch (code)
	{
	case BC2_NO_ERROR:
		break;
	case BC2_ERROR_SHORT_OF_MEMORY:
		fprintf(stderr, "Short of memory.\n");
		break;
	case BC2_ERROR_PARAM:
		fprintf(stderr, "Short of parameter.\n");
		break;
	case BC2_ERROR_INPUT:
		if (para[0])
			fprintf(stderr, "File input error \'%s\'.\n", para);
		else
			fprintf(stderr, "File input error <stdin>.\n", para);
		break;
	case BC2_ERROR_OUTPUT:
		fprintf(stderr, "File output error \'%s\'.\n", param->outpath);
		break;
	}
	return code;
}

BC2_ERROR bincut2(BC2_PARAM *param,  GETOPT *opt)
{
	int ret, optend = 0;
	while (1)
	{
		char *p = GetOptNext(opt);
		if (!p) return BC2_NO_ERROR;
		if (!optend && p[0] == '@')
		{
			GETOPT inopt;
			ret = GetOptInitFile(&inopt, p + 1);
			if (ret) return bc2_error(param, ret, p + 1);
			ret = bincut2(param, &inopt);
			if (ret) return ret;
			continue;
		}
		if (!optend && p[0] == '-')
		{
			switch (p[1])
			{
			case '-':
				optend = 1;
				continue;
			case 'o':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				strncpy(param->outpath, p, MAX_PATH);
				continue;
			case 's':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->skip = hatoi(p);
				continue;
			case 'l':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->len = hatoi(p);
				continue;
			case 'u':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->unit = hatoi(p);
				continue;
			case 'n':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->num = hatoi(p);
				continue;
			case 'b':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->block = hatoi(p);
				continue;
			case 'z':
				p = p[2] ? p + 2 : GetOptNext(opt);
				if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
				param->zero = hatoi(p);
				continue;
			case 'p':
				{
					int addr, data;
					p = p[2] ? p + 2 : GetOptNext(opt);
					if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
					p = hatoi2(p, &addr);
					p = (*p == ':') ? p + 1 : GetOptNext(opt);
					if (!p) return bc2_error(param, BC2_ERROR_PARAM, 0);
					do {
						p = hatoi2(p, &data);
						alloc_patch(param, addr++, data);
					} while (*p++ == ',');
				}
				continue;
			case 'i':
				param->inpath[0] = '\0';
				break;
			default:
				return bc2_error(param, BC2_ERROR_PARAM, 0);
			}
		}
		else
		{
			strncpy(param->inpath, p, MAX_PATH);
		}
		ret = bincut2main(param);
		if (ret) return bc2_error(param, ret, param->inpath);
		reset_param(param);
		optend = 0;
		param->help = 0;
		/*  1つ以上成功した場合ヘルプは出さない */
	}
}

int main(int argc, char **argv)
{
	BC2_PARAM bc2param;
	GETOPT getopt;
	init_param(&bc2param);
	GetOptInit(&getopt, argc, argv);
	bincut2(&bc2param, &getopt);
	if (bc2param.help)
	{
		fprintf(stderr, "bincut2.1 by Mamiya\n",argv[0]);
		fprintf(stderr, "Usage: %s [OPTIONS] [--] SOURCE\n",argv[0]);
		fprintf(stderr, "\tOPTIONS\n");
		fprintf(stderr, "\t\t-o OUTPUT FILENAME\n");
		fprintf(stderr, "\t\t-s SKIP LENGTH IN BYTES\n");
		fprintf(stderr, "\t\t-l OUTPUT LENGTH IN BYTES\n");
		fprintf(stderr, "\t\t-u BLOCK UNIT LENGTH IN BYTES\n");
		fprintf(stderr, "\t\t-n OUTPUT LENGTH IN BLOCKS\n");
		fprintf(stderr, "\t\t-b SKIP LENGTH IN BLOCKS\n");
		fprintf(stderr, "\t\t-p PATCH OFFSET:DATA[,DATA]...\n");
		fprintf(stderr, "\t\t-z ZERO PADDING LENGTH IN BYTES\n");
		fprintf(stderr, "\t\t-i USE STANDARD INPUT\n");
	}
	term_param(&bc2param);
	return bc2param.help;
}
