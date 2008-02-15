/*
	cxacru-info - outputs cxacru status information from sysfs
	(also an example of obsessive checking of the return value
	and insane code to keep the layout of the output the same)

	Copyright ©2007 Simon Arlott

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License v2
	as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
	Or, point your browser to http://www.gnu.org/copyleft/gpl.html


	http://simon.arlott.org/sw/cxacru-info/
	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC(ptr, size) do { \
		ptr = malloc(size); \
		if (ptr == NULL) { \
			perror("malloc"); \
			exit(3); \
		} \
	} while(0)

#define ZMALLOC(ptr, size) do { \
		MALLOC(ptr,size); \
		memset(ptr, 0, size); \
	} while(0)

#define OOPS(what) \
	fprintf(stderr, "%s at %s:%u\n", what, __FILE__, __LINE__)

#define PRINTF(format, args...) do { \
		int __ret = printf(format , ##args); \
		if (__ret <= 0) \
			exit(3); \
	} while(0)

#define FPRINTF(stream, format, args...) do { \
		int __ret = fprintf(stream, format , ##args); \
		if (__ret <= 0) \
			exit(3); \
	} while(0)

#define SNPRINTF(str, size, format, args...) do { \
		int __ret = snprintf(str, size, format , ##args); \
		if (__ret < 0) { \
			OOPS("snprintf failed"); \
			exit(3); \
		} else if (__ret >= size) { \
			OOPS("snprintf truncated"); \
			exit(3); \
		} \
	} while(0)

#define ERR_IF(expr, str) do { \
		if (expr) { \
			perror(str); \
			exit(1); \
		} \
	} while(0)

#define SYS_PATH "/sys/class/atm/"
#define ATM_DEVICES "/proc/net/atm/devices"
#define MAXLEN 255

unsigned long dev_num = -1;
unsigned long aal5[5];

enum { TXcount, TXerr, RXcount, RXerr, RXdrop };

char *cxacru(const char *file) {
	char filename[MAXLEN];
	char *data;

	ZMALLOC(data, MAXLEN + 1); // ensure string is always terminated
	SNPRINTF(filename, MAXLEN, SYS_PATH "cxacru%u/device/%s", dev_num, file);
	FILE *fd = fopen(filename, "r");
	ERR_IF(fd == NULL, filename);
	ERR_IF(fgets(data, MAXLEN, fd) == NULL, filename);
	ERR_IF(fclose(fd) != 0, filename);

	if (strlen(data) > 0) // remove newline
		data[strlen(data) - 1] = 0;
	return data;
}

char *strpad(char *str, int len) {
	if (strlen(str) >= len) {
		return str;
	} else {
		char *ret;

		MALLOC(ret, len + 1);
		memset(ret, ' ', len);
		memcpy(ret + (len - strlen(str)), str, strlen(str) + 1);
		free(str);

		return ret;
	}
}

char *intpad(unsigned int value, int len) {
	char *ret;

	MALLOC(ret, MAXLEN);
	SNPRINTF(ret, MAXLEN, "%u", value);

	return strpad(ret, len);
}

void find_atm_dev(long cxacru_num) {
	unsigned int num;
	int ret;
	FILE *fd = fopen(ATM_DEVICES, "r");

	ERR_IF(fd == NULL, ATM_DEVICES);
	ret = fscanf(fd, "%*[^\n][\n]");
	ERR_IF(ret < 0, ATM_DEVICES);

	while (6 == (ret = fscanf(fd,
			"%u cxacru %*s 0 ( %*ld %*ld %*ld %*ld %*ld ) 5 ( %ld %ld %ld %ld %ld ) [%*ld]%*[\n]",
			&num, &aal5[0], &aal5[1], &aal5[2], &aal5[3], &aal5[4]))) {
		if (cxacru_num < 0 || cxacru_num == num) {
			dev_num = num;
			break;
		}
	}
	ERR_IF(fclose(fd) != 0, ATM_DEVICES);
}

int main(int argc, char *argv[]) {
	char *modulation;

	if (argc > 2 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))) {
		PRINTF("Usage: %s [device num]\n", argv[0]);
		return 2;
	} else if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
		PRINTF("cxacru-info $Revision$ $Date$\n");
		return 0;
	}

	find_atm_dev(argc == 2 ? atol(argv[1]) : -1);
	if (dev_num == -1) {
		FPRINTF(stderr, "cxacru device not found\n");
		return 1;
	}

	PRINTF("                   Downstream     Upstream\n");
	PRINTF("\n");
	PRINTF("Line rate          %s"""" kbps    %s"""" kbps\n",
		strpad(cxacru("downstream_rate"), 6),
		strpad(cxacru("upstream_rate"), 6));
	PRINTF("Attenuation        %s"""" dB      %s"""" dB\n",
		strpad(cxacru("downstream_attenuation"), 6),
		strpad(cxacru("upstream_attenuation"), 6));
	PRINTF("Noise margin       %s"""" dB      %s"""" dB\n",
		strpad(cxacru("downstream_snr_margin"), 6),
		strpad(cxacru("upstream_snr_margin"), 6));
	PRINTF("Power                             %s"""" dBm/Hz\n",
		strpad(cxacru("transmitter_power"), 6));
	PRINTF("\n");
	PRINTF("AAL5 Frames    %s""""""""     %s""""""""\n",
		intpad(aal5[RXcount], 10), intpad(aal5[TXcount], 10));
	PRINTF("     Errors    %s""""""""     %s""""""""\n",
		intpad(aal5[RXerr], 10), intpad(aal5[TXerr], 10));
	PRINTF("     Dropped   %s""""""""\n",
		intpad(aal5[RXdrop], 10));
	PRINTF("\n");
	PRINTF("CRC errors     %s""""""""     %s""""""""\n",
		strpad(cxacru("downstream_crc_errors"), 10),
		strpad(cxacru("upstream_crc_errors"), 10));
	PRINTF("FEC errors     %s""""""""     %s""""""""\n",
		strpad(cxacru("downstream_fec_errors"), 10),
		strpad(cxacru("upstream_fec_errors"), 10));
	PRINTF("HEC errors     %s""""""""     %s""""""""\n",
		strpad(cxacru("downstream_hec_errors"), 10),
		strpad(cxacru("upstream_hec_errors"), 10));
	PRINTF("\n");
	PRINTF("Line status        %s\n", cxacru("line_status"));
	PRINTF("Link status        %s\n", cxacru("link_status"));
	modulation = cxacru("modulation");
	if (strlen(modulation) > 0)
	PRINTF("Modulation         %s\n", cxacru("modulation"));
	PRINTF("\n");
	PRINTF("MAC address        %s\n", cxacru("mac_address"));
	return 0;
}
