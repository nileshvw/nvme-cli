#include<stdio.h>
#include<stdlib.h>
#include<linux/nvme_ioctl.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>

int main(int argc, char **argv)
{
        static const char *perrstr;
        int err, fd;
        unsigned nsid;
        struct nvme_passthru_cmd cmd;

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
        cmd.opcode = 0; // nvme_cmd_flush;
        cmd.nsid = nsid;

        perrstr = "Flush";
        err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
        if (err < 0)
                goto perror;
        else if (err != 0)
                fprintf(stderr, "NVME IO command error:%d\n", err);
        else
                printf("NVMe Flush: success\n");
        return err;

 perror:
        perror(perrstr);
        return 1;
}
