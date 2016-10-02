#include "cnic_init.h"
#include <bmk-core/printf.h>

#include <rump/rump_syscalls.h>
#include <rump/rump.h>

#include <bmk-core/sched.h>
#include <unistd.h>
#include <dirent.h>
#include <netconfig.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "../../src-netbsd/sys/net/if_cnic.h"
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netconfig.h>
#include <rump/netconfig.h>
#include <rump/rump.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>

int rumpns_rtinit(struct ifaddr *, int, int);
extern int rump_vmid;
//
///* HACK FIXME */
extern struct ifnet *rumpns_global_ifp;

static int
set_gw(char *addr)
{
	int rv;

	struct ifaddr *rt;
	struct sockaddr_in *rt_addr;
	struct sockaddr_in *rt_netmask;
	struct in_addr baddr4; /* network byte ip address */

	int netmask = 24;        /* highlevel netmask */
	uint32_t bit_mask;           /* actual bit mask */

	rt = (struct ifaddr *) malloc(sizeof(struct ifaddr));
	assert(rt);

	rt_addr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
	assert(rt_addr);

	rt_netmask = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
	assert(rt_netmask);

	/* Netmask */
	bit_mask = ~0;
	bit_mask = ~(bit_mask >> netmask);
	bit_mask = htonl(bit_mask); /* Go from host byte order to network byte order */

	memset(rt_netmask, '\0', sizeof(struct sockaddr_in));
	rt_netmask->sin_family = AF_INET;
	rt_netmask->sin_addr.s_addr = bit_mask;
	rt_netmask->sin_len = sizeof(struct sockaddr_in);

	/* Address */
	memset(&baddr4, '\0', sizeof(baddr4));
	rv = inet_pton(AF_INET, addr, &baddr4);
	assert(rv == 1);

	memset(rt_addr, '\0', sizeof(struct sockaddr_in));
	rt_addr->sin_family = AF_INET;
	rt_addr->sin_addr.s_addr = baddr4.s_addr;
	rt_addr->sin_len = sizeof(struct sockaddr_in);

	memset(rt, '\0', sizeof(struct ifaddr));
	rt->ifa_addr = (struct sockaddr *)rt_addr;
	rt->ifa_netmask = (struct sockaddr *)rt_netmask;
	assert(rumpns_global_ifp);
	rt->ifa_ifp = rumpns_global_ifp;

	rv = rumpns_rtinit(rt, RTM_ADD, RTF_UP | RTF_GATEWAY);

	return rv;
}

int
rump_cnic_create(int num, char *addr, char *netmask){
	bmk_printf("rump_cnic_create!\n");
	int rv, fd;
	int debug = 1;
	char cnic_dev[11];
	char cnic[6];

	if(!rump_vmid) bmk_printf("creating cnic%d for DOM0\n", num);
	else bmk_printf("creating cnic%d for VM%d\n", num, rump_vmid);

	sprintf(cnic, "cnic%d", num);
	bmk_printf("%s\n", cnic);
	sprintf(cnic_dev, "/dev/cnic%d", num);
	bmk_printf("%s\n", cnic_dev);
	bmk_printf("addr: %s\n", addr);
	bmk_printf("netmask: %s\n", netmask);

	fd = open(cnic_dev, O_RDWR);
	bmk_printf("open fd: %d\n", fd);

	rv = ioctl(fd, CNICSDEBUG, &debug);
	if(!rv) bmk_printf("cnic%d set debug: success\n", num);
	else bmk_printf("cnic%d set debug: fail %d\n", num, rv);

	rv = rump_pub_netconfig_ipv4_ifaddr(cnic, addr, netmask);
	if(!rv) bmk_printf("ipv4 set address: success\n");
	else bmk_printf("ipv4 set address: fail %d\n", rv);

	/* We assume that this gateway route is for range of [addr, 255.255.255.0] */
	if(rump_vmid == 0) rv = set_gw(addr);
	else rv = rump_pub_netconfig_ipv4_gw(addr);

	if(!rv) bmk_printf("ipv4 set gw: success\n");
	else bmk_printf("ipv4 set gw: fail %d\n", rv);

	rv = rump_pub_netconfig_ifup(cnic);
	if(!rv) bmk_printf("ifup: success\n");
	else bmk_printf("ifup: fail %d\n", rv);
	
	return fd;	
}

int
rump_cnic_init(int ignore, char *argv[])
{
	printf("rump_cnic_init\n");
	int vmid = rump_vmid;
	int fd;
	/*
	 * Uncomment for paws file system tests
	 * paws_tests();
	 */
	if(vmid == 0) {
		bmk_printf("----- dom0 -----\n");
	        fd = rump_cnic_create(0, "111.111.111.0", "255.255.255.0");	
		assert(fd);
	        fd = rump_cnic_create(1, "222.222.222.0", "255.255.255.0");	
		assert(fd);
	} else if(vmid == 1){
		bmk_printf("----- VM%d -----\n", vmid);
	        fd  = rump_cnic_create(2, "111.111.111.1", "255.255.255.0");	
		bmk_printf("assert\n", vmid);
		assert(fd);
	} else if(vmid == 2){
		bmk_printf("----- VM%d -----\n", vmid);
	        fd = rump_cnic_create(3, "222.222.222.1", "255.255.255.0");	
		assert(fd);
	}

	printf("done with cnic_init\n");
	bmk_sched_blockprepare();
	bmk_sched_block();
	assert(0);
	return 0;
}
