#ifndef _IPADDR_H_
#define _IPADDR_H_

char *ntoa_impl(unsigned long s_addr, char *buf, int buflen);
int aton_impl(const char *cp, uint32_t *addr);
#endif //_IPADDR_H_
