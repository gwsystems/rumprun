/*-
 * Copyright (c) 2013, 2014 Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <bmk/types.h>
#include <bmk/kernel.h>

#include <bmk-core/pgalloc.h>
#include <bmk-core/printf.h>

#include "pci_user.h"
extern void *bmk_va2pa(void *addr);

#define PCI_CONF_ADDR 0xcf8
#define PCI_CONF_DATA 0xcfc

//int
//rumpcomp_pci_iospace_init(void)
//{
//	bmk_printf("%s:%d\n", __func__, __LINE__);
//
//	return 0;
//}

static uint32_t
makeaddr(unsigned bus, unsigned dev, unsigned fun, int reg)
{

	return (1<<31) | (bus<<16) | (dev <<11) | (fun<<8) | (reg & 0xfc);
}

int
rumpcomp_pci_confread(unsigned bus, unsigned dev, unsigned fun, int reg,
	unsigned int *value)
{
	uint32_t addr;
	unsigned int data;

	addr = makeaddr(bus, dev, fun, reg);
	outl(PCI_CONF_ADDR, addr);
	data = inl(PCI_CONF_DATA);

	*value = data;
	return 0;
}

int
rumpcomp_pci_confwrite(unsigned bus, unsigned dev, unsigned fun, int reg,
	unsigned int value)
{
	uint32_t addr;

	addr = makeaddr(bus, dev, fun, reg);
	outl(PCI_CONF_ADDR, addr);
	outl(PCI_CONF_DATA, value);

	return 0;
}

static int intrs[BMK_MAXINTR];

int
rumpcomp_pci_irq_map(unsigned bus, unsigned device, unsigned fun,
	int intrline, unsigned cookie)
{

	bmk_printf("%s:%d - cookie: %x fun: %x device: %x intrline: %x bus: %x\n", 
			__func__, __LINE__, 
			cookie, fun, device, intrline, bus);
	if (cookie > BMK_MAXINTR)
		return BMK_EGENERIC;

	bmk_printf("%s:%d\n", __func__, __LINE__);
	intrs[cookie] = intrline;
	return 0;
}

void *
rumpcomp_pci_irq_establish(unsigned cookie, int (*handler)(void *), void *data)
{

	bmk_printf("%s:%d - cookie: %x handler: %x data: %x\n", __func__, __LINE__, 
			cookie, handler, data);
	if (bmk_isr_init(handler, data, intrs[cookie]) == 0)
		return &intrs[cookie];
	else
		return NULL;
}

/*
 * Well at least there's some benefit to running on physical memory.
 * This stuff is really trivial.
 */

void *
rumpcomp_pci_map(unsigned long addr, unsigned long len)
{
	bmk_printf("%s:%d\n", __func__, __LINE__);
	bmk_printf("%s: 0x%x:%lu\n", __func__, addr, len);
	void *mem;
	int i;

        for (i = 0; len >> (i + PAGE_SHIFT); i++)
                continue;


	mem = bmk_pgalloc(i);
	if (!mem)
		return (void *)NULL;

	bmk_printf("%s: 0x%x:%lu\n", __func__, (int)mem, (int)i);

	return (void *)mem;
}

int
rumpcomp_pci_dmalloc(size_t size, size_t align,
	unsigned long *pap, unsigned long *vap)
{
	bmk_printf("%s:%d\n", __func__, __LINE__);
	void *mem;
	int i;

        for (i = 0; size >> (i + PAGE_SHIFT); i++)
                continue;


	mem = bmk_pgalloc(i);
	if (!mem)
		return BMK_ENOMEM;
	bmk_printf("%s: 0x%x:%lu\n", __func__, (int)mem, (int)i);

	//*pap = (unsigned long)mem;
	*pap = (unsigned long)bmk_va2pa(mem);
	*vap = (unsigned long)mem;
	bmk_printf("%s: 0x%x-0x%x\n", __func__, *pap, *vap);
	return 0;
}

int
rumpcomp_pci_dmamem_map(struct rumpcomp_pci_dmaseg *dss, size_t nseg,
	size_t totlen, void **vap)
{

	bmk_printf("%s:%d - dss[0].ds_pa:%x dss[0].ds_len:%d dss[0].ds_vacookie:%x\n", __func__, __LINE__, dss[0].ds_pa, dss[0].ds_len, dss[0].ds_vacookie);
	bmk_printf("%s:%d - nseg:%d totlen:%d\n", __func__, __LINE__, nseg, totlen);
	bmk_printf("%s:%d\n", __func__, __LINE__);
	if (nseg > 1)
		return 1;

	*vap = (void *)dss[0].ds_vacookie;
	bmk_printf("%s: 0x%x\n", __func__, (int)*vap);
	return 0;
}

//void
//rumpcomp_pci_dmafree(unsigned long mem, size_t size)
//{
//
//	bmk_printf("%s:%d\n", __func__, __LINE__);
//	int i;
//
//        for (i = 0; size >> (i + PAGE_SHIFT); i++)
//                continue;
//	bmk_pgfree((void *)mem, i);
//}

unsigned long
rumpcomp_pci_virt_to_mach(void *virt)
{

	bmk_printf("%s:%d\n", __func__, __LINE__);
	bmk_printf("%s: 0x%x:0x%x\n", __func__, (int)virt, (int)bmk_va2pa(virt));
	return (unsigned long)bmk_va2pa(virt);
	//return (unsigned long)(virt);
}
