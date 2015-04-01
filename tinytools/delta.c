/* delta.c - loveemu labo
 * public domained source code
 * 
 * example 1 : delta 1 <input> <output>
 * input     : 00 01 02 03 04 05 06 07
 * output    : 00 01 01 01 01 01 01 01
 * 
 * example 2 : delta 2 <input> <output>
 * input     : 00 FF 01 FE 02 FD 03 FC
 * output    : 00 FF 01 FF 01 FF 01 FF
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int result = 0;
	if(argc != 4)
	{
		printf("usage: %s <pitch> <input> <output>\n", argv[0]);
		result = 1;
	}
	else
	{
		int pitch = atoi(argv[1]);
		if(pitch < 1)
		{
			puts("error: 'pitch' out of range");
			result = 1;
		}
		else
		{
			FILE *in_fp = fopen(argv[2], "rb");
			if(!in_fp)
			{
				puts("error: cannot open input file");
			}
			else
			{
				FILE *out_fp = fopen(argv[3], "wb");
				if(!out_fp)
				{
					puts("error: cannot open output file");
				}
				else
				{
					int io_size = 256 * 1024;
					char *in_buf = (char *)malloc(io_size);
					if(!in_buf)
					{
						puts("error: cannot allocate input buffer");
					}
					else
					{
						char *out_buf = (char *)malloc(io_size);
						if(!out_buf)
						{
							puts("error: cannot allocate output buffer");
						}
						else
						{
							size_t head_left = pitch;
							while(head_left > 0 && !feof(in_fp))
							{
								size_t size;
								size = fread(out_buf, 1, (pitch < io_size ? pitch : io_size), in_fp);
								fwrite(out_buf, size, 1, out_fp);
								head_left -= size;
							}
							while(!feof(in_fp))
							{
								size_t i;
								size_t size;

								fseek(in_fp, -pitch, SEEK_CUR);
								size = fread(in_buf, 1, io_size, in_fp);
								fseek(in_fp, pitch - (int)size, SEEK_CUR);
								size = fread(out_buf, 1, io_size, in_fp);

								for(i = 0; i < size; i++)
								{
									out_buf[i] -= in_buf[i];
								}
								fwrite(out_buf, size, 1, out_fp);
							}
							free(out_buf);
						}
						free(in_buf);
					}
					fclose(out_fp);
				}
				fclose(in_fp);
			}
		}
	}
	return result;
}
