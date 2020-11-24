/*******************************************************
 * $File:   ethstt.c
 * $Author: Hua Shao
 *******************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <linux/if.h>
#include <ra_ioctl.h>


int port_status(int port, int reg)
{
    int sk, ret;
    int result;
    struct ifreq ifr;
    int method;

    struct ra_mii_ioctl_data mii;
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0)
    {
        printf("Open socket failed\n");
        return -1;
    }
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &mii;

    mii.phy_id=port; //port
    mii.reg_num=reg; //reg
    method = RAETH_MII_READ;
    /* NOTE: Here we read it twice on purpose.
     * According to IEEE-802.3-2005, section 22.2.4.2.13:
     * " The Link Status bit shall be implemented with a
     * latch function, such that the occurrence of a link
     * failure condition will cause the Link Status bit
     * to become cleared and remain cleared until its read
     * via the management interface. "
     */
    ret = ioctl(sk, method, &ifr); // read it twice
    ret = ioctl(sk, method, &ifr); // read it twice
    if (ret < 0)
    {
        printf("mii_mgr: ioctl error\n");
    }
    result = mii.val_out;
    result = ((result&0x04) >>2);
    return result;

}


int main(int argc, char *argv[])
{
    int i;

    /* p0-p4 are lan/wan ports. */
    for (i=0; i<5; i++)
    {
        printf("port %d %s\n", i, port_status(i, 1)>0?"up":"down");
    }

    return 0;
}
