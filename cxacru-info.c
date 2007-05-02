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

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINTF(format, args...) do { \
		int ret = printf(format , ##args); \
		if (ret <= 0) { \
			perror("printf"); \
			exit(3); \
		} \
	} while(0);

#define SYS_PATH "/sys/class/atm/"
#define ATM_DEVICES "/proc/net/atm/devices"
#define MAXLEN 255

unsigned int dev_num = -1;
unsigned int aal5[5];

enum { TXcount, TXerr, RXcount, RXerr, RXdrop };

char *cxacru(const char *file) {
	char filename[MAXLEN];
	char *data;
	int ret;

	if (dev_num == -1) {
		fprintf(stderr, "cxacru device not found\n");
		exit(1);
	}

	data = malloc(MAXLEN + 1);
	if (data == NULL) {
		perror("malloc");
		exit(3);
	}
	memset(data, 0, MAXLEN + 1);

	ret = snprintf(filename, MAXLEN, SYS_PATH "cxacru%u/device/%s", dev_num, file);
	if (ret < 0) {
		perror("snprintf");
		exit(3);
	}
	if (ret >= MAXLEN) {
		fprintf(stderr, "snprintf: filename too long\n");
		exit(3);
	}

	FILE *fd = fopen(filename, "r");
	if (fd == NULL) {
		perror(filename);
		exit(1);
	}
	if (fgets(data, MAXLEN, fd) == NULL) {
		perror(filename);
		exit(1);
	}
	if (fclose(fd) != 0) {
		perror(filename);
		exit(1);
	}

	if (strlen(data) > 0) // remove newline
		data[strlen(data) - 1] = 0;
	return data;
}

char *strpad(char *str, int len) {
	if (strlen(str) >= len) {
		return str;
	} else {
		char *ret = malloc(len + 1);

		if (ret == NULL) {
			perror("malloc");
			exit(3);
		}

		memset(ret, ' ', len);
		ret[len] = 0;
		memcpy(ret + (len - strlen(str)), str, strlen(str));
		free(str);

		return ret;
	}
}

char *intpad(unsigned int value, int len) {
	char *ret = malloc(MAXLEN);

	if (ret == NULL) {
		perror("malloc");
		exit(3);
	}

	snprintf(ret, MAXLEN, "%u", value);
	return strpad(ret, len);
}

void find_atm_dev(char *cxacru_num) {
	char *tmp;
	unsigned int num;
	int ret;
	FILE *fd = fopen(ATM_DEVICES, "r");

	if (fd == NULL) {
		perror(ATM_DEVICES);
		exit(1);
	}

	ret = fscanf(fd, "%a[^\n]%*[\n]", &tmp);
	if (ret != 1) {
		fprintf(stderr, ATM_DEVICES ": invalid format\n");
		exit(3);
	}
	free(tmp);

	while (6 == (ret = fscanf(fd,
			"%u cxacru %*s 0 ( %*d %*d %*d %*d %*d ) 5 ( %d %d %d %d %d ) [%*d]%*[\n]",
			&num, &aal5[0], &aal5[1], &aal5[2], &aal5[3], &aal5[4]))) {
		if (cxacru_num == NULL || num == atoi(cxacru_num)) {
			dev_num = num;
			break;
		}
	}
	fclose(fd);
}

int main(int argc, char *argv[]) {
	char *modulation;

	if (argc > 2 || (argc == 2 && !strncmp(argv[1], "-h", 3))) {
		PRINTF("Usage: %s [device num]\n", argv[0]);
		return 2;
	}
	find_atm_dev(argc == 2 ? argv[1] : NULL);

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
