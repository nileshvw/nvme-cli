#include<stdio.h>
#include<stdlib.h>
#include<linux/nvme_ioctl.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>

#define BLK_SIZE 512
#define BLKS (256*1024/BLK_SIZE) // 256KB
#define SIZE (BLKS*BLK_SIZE)

#define NVME_GET(value, name) \
    (((value) >> NVME_##name##_SHIFT) & NVME_##name##_MASK)

#define NVME_SCT_SHIFT (0x8)
#define NVME_SCT_MASK  (0x7)

#define NVME_SC_SHIFT (0)
#define NVME_SC_MASK  (0xff)

#define NVME_STATUS_TYPE_SHIFT (27)
#define NVME_STATUS_TYPE_MASK  (0x7)

#define NVME_SC_CRD  (0x1800)
#define NVME_SC_MORE (0x2000)
#define NVME_SC_DNR  (0x4000)

int main(int argc, char **argv)
{
	static const char *perrstr;
	int err, fd;
	unsigned nsid;
	struct nvme_passthru_cmd cmd;
	char *data = malloc(SIZE * sizeof (char));

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <device> <nsid(optional)>\n",
								argv[0]);
		return 1;
	}

	perrstr = argv[1];
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
		goto perror;

	if (argc == 2) {
		perrstr = "Getting namespace ID";
		nsid = ioctl(fd, NVME_IOCTL_ID);
		if (nsid == (unsigned)-1)
			goto perror;
	} else {
		if (sscanf(argv[2], "%i", &nsid) != 1) {
			fprintf(stderr, "Invalid parameter:%s\n", argv[2]);
			return 1;
		}
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = 0x02; // 
	cmd.nsid = nsid;
	cmd.cdw10 = 1; /* Logical Block Address we want to read*/
	cmd.cdw11 = 0; /* higher significant double word of LBA*/
	cmd.cdw12 = BLKS - 1; /* number of LBAs we want to read - SECTORS = 4 here*/
	// cmd.cdw13 = ;
	// cmd.cdw14 = ;
	cmd.addr = (__u64) data;
	cmd.data_len = SIZE;

	perrstr = "READ";
	err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
	if (err < 0)
		goto perror;
	else if (err != 0) {
		fprintf(stderr, "NVME IO command error: %d %#X\n", err, err);
		fprintf(stderr, "SCT  : %#X\n", NVME_GET(err, SCT));
		fprintf(stderr, "SC   : %#X\n", NVME_GET(err, SC));
		fprintf(stderr, "CRD  : %#X\n", NVME_GET(err, STATUS_TYPE) & NVME_SC_CRD);
		fprintf(stderr, "MORE : %#X\n", NVME_GET(err, STATUS_TYPE) & NVME_SC_MORE);
		fprintf(stderr, "DNR  : %#X\n", NVME_GET(err, STATUS_TYPE) & NVME_SC_DNR);
	} else {
		printf("NVMe : %s : success\n", perrstr);
	}
	return err;

 perror:
	perror(perrstr);
	return 1;
}
